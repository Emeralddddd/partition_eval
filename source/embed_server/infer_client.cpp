#include "infer_client.hpp"
#include "cnpy.h"
#include <chrono>

using std::vector;
using std::unordered_set;

void Dispatcher::LoadPartitionNPZ(std::string path, double hot_rate){
    vector<int> embed_partition = cnpy::npz_load(path,"embed_partition").as_vec<int>();
    int n = embed_partition.size();
    if(n > n_embeds_) resize(n);
    partition_ = std::move(embed_partition);
    if(hot_rate < 1e-6) return;
    int hot_size = hot_rate * n_embeds_;
    for(int i = 0; i < n_parts_; i++){
        vector<long long> priority_list = cnpy::npz_load(path,std::to_string(i)).as_vec<long long>();
        caches_[i] = unordered_set<int>(priority_list.begin(),priority_list.begin() + hot_size);
    }
}

void Dispatcher::LoadPartitionMerge(PartitionResult&& pr){
    int n = pr.partition.size();
    if(n > n_embeds_) resize(n);
    partition_ = std::move(pr.partition);
    for(int i = 0; i < n_parts_; i++){
        caches_[i] = unordered_set<int>(pr.caches[i].begin(),pr.caches[i].end());
    }
}

void Dispatcher::DispatchRequest(const vector<int> &input){
    int n = input.size();
    vector<int> partCnt(n_parts_);
    InferenceRequest request;
    InferenceReply reply;
    grpc::ClientContext context;
    request.mutable_data()->Reserve(n);
    request.mutable_pos()->Reserve(n); 
    for(int i = 0; i < n; i++){
        if(input[i] < 0) continue;
        if(input[i] >= n_embeds_) resize(input[i]);
        int targetPart = partition_[input[i]] == -1 ? input[i] % n_parts_ : partition_[input[i]];
        request.add_data(input[i]);
        request.add_pos(targetPart);
        for(int j = 0; j < n_parts_; j++){
            if(targetPart != j && !caches_[j].count(input[i])) partCnt[j]++;
        }
    }
    int target = std::min_element(partCnt.begin(),partCnt.end()) - partCnt.begin();
    vector<bool> nodeIsAccess(n_parts_);
    for(int i = 0; i < n; i++){
        if(caches_[target].count(input[i])) request.set_pos(i,target);
    }
    #pragma omp critical
    {
        for(int i = 0; i < n; i++){
            if(request.pos(i) != target) remoteCnt_++;
            remote_data_cnt[target][request.pos(i)]++;
            if(!nodeIsAccess[request.pos(i)]) nodeIsAccess[request.pos(i)] = true;
        }
        for(int i = 0; i < n_parts_; i++){
            if(nodeIsAccess[i]){
                nodeCnt_++;
                remote_request_cnt[target][i]++;
            }
        }
        nodeCnt_--;
    }
    if(stats_only_) return;
    grpc::Status status = stub_list_[target]->Inference(&context, request, &reply);
    if(!status.ok()){
        std::cout << "Request failed. Error message: " << status.error_message() << std::endl;
        return;
    }
    #pragma omp critical
    {
        time_vec_.emplace_back(reply.time());
        auto & time_map = reply.info().remote_time();
        for(int i = 0; i < n_parts_; i++){
            if(auto it = time_map.find(i); it != time_map.end()){
                remote_time_vecs[target][i].emplace_back(it->second);
            }else{
                std::cout << "request from " << target << " miss key " << i << std::endl;
                std::cout << " --------- " << std::endl;
            }
        }
    }
    return;
}