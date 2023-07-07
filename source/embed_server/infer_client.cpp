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
    caches_ =  vector<unordered_set<int>>(4);
    if(hot_rate < 1e-6) return;
    int hot_size = hot_rate * n_embeds_;
    for(int i = 0; i < n_parts_; i++){
        vector<long long> priority_list = cnpy::npz_load(path,std::to_string(i)).as_vec<long long>();
        caches_[i] = unordered_set<int>(priority_list.begin(),priority_list.begin() + hot_size);
    }
}

void Dispatcher::LoadPartitionMerge(const PartitionResult& pr){
    int n = pr.partition.size();
    if(n > n_embeds_) resize(n);
    caches_ =  vector<unordered_set<int>>(4);
    for(int i = 0; i < n; i++){
       partition_ = pr.partition;
    }
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
    for(int i = 0; i < n; i++){
        if(caches_[target].count(input[i])) request.set_pos(i,target);
        if(request.pos(i) != target) remoteCnt_++;
    }
    auto start = std::chrono::high_resolution_clock::now();
    stub_list_[target]->Inference(&context, request, &reply);
    auto end = std::chrono::high_resolution_clock::now();
    #pragma omp critical
    {
        time_vec_.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }
    return;
}