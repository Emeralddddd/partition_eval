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
        vector<int> priority_list = cnpy::npz_load(path,"embed_partition").as_vec<int>();
        caches_[i] = unordered_set<int>(priority_list.begin(),priority_list.end());
    }
}

void Dispatcher::LoadPartitionMerge(const PartitionResult& pr){
    int n = pr.partition.size();
    if(n > n_embeds_) resize(n);
    caches_ =  vector<unordered_set<int>>(4);
    for(int i = 0; i < n; i++){
       partition_ = std::move(pr.partition);
    }
    for(int i = 0; i < n_parts_; i++){
        caches_[i] = unordered_set<int>(pr.caches[i].begin(),pr.caches[i].end());
    }
}

void Dispatcher::DispatchRequest(const vector<int> &input){
    auto start = std::chrono::high_resolution_clock::now();
    int n = input.size();
    vector<int> partCnt(n_parts_);
    InferenceRequest request;
    InferenceReply reply;
    grpc::ClientContext context;
    request.mutable_data()->Reserve(n);
    request.mutable_pos()->Reserve(n); 
    for(int i = 0; i < n; i++){
        if(input[i] >= n_embeds_ || input[i] < 0) continue;
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
    }
    stub_list_[target]->Inference(&context, request, &reply);
    auto end = std::chrono::high_resolution_clock::now();
    time_vec_.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    query_cnt_++;
    return;
}