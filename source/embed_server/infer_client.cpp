#include "infer_client.hpp"
#include "cnpy.h"

using std::vector;

void Dispatcher::LoadPartitionNPZ(std::string path, double hot_rate){
    vector<int> embed_partition = cnpy::npz_load(path,"embed_partition").as_vec<int>();
    int n = embed_partition.size();
    if(n > n_embeds_) resize(n);
    for(int i = 0; i < n; i++){
        if(embed_partition[i] < 0) continue;
        partition_bits[i] = 1 << embed_partition[i];
    }
    if(hot_rate < 1e-6) return;
    int hot_size = hot_rate * n_embeds_;
    for(int i = 0; i < n_parts_; i++){
        vector<int> priority_list = cnpy::npz_load(path,"embed_partition").as_vec<int>();
        for(int j = 0; j < hot_size; j++){
            int embed_id = priority_list[j];
            partition_bits[embed_id] |= (1 << i);
        }
    }
}

void Dispatcher::LoadPartitionMerge(const PartitionResult& pr){
    int n = pr.partition.size();
    if(n > n_embeds_) resize(n);
    for(int i = 0; i < n; i++){
        if(pr.partition[i] < 0) continue;
        partition_bits[i] = 1 << pr.partition[i];
    }
    for(int i = 0; i < n_parts_; i++){
        for(int j = 0; j < pr.caches[i].size(); j++){
            int embed_id = pr.caches[i][j];
            partition_bits[embed_id] |= (1 << i);
        }
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
        if(input[i] >= n_embeds_ || input[i] < 0) continue;
        int bit = partition_bits[input[i]];
        request.add_data(input[i]);
        for(int j = 0; j < n_parts_; j++){
            if((bit >> j) & 1) request.add_pos(j);
            else partCnt[j]++;
        }
    }
    int target = std::min_element(partCnt.begin(),partCnt.end()) - partCnt.begin();
    for(int i = 0; i < n; i++){
        if((partition_bits[input[i]] >> target) & 1) request.set_pos(i,target);
    }
    std::cout << "start inference on " << target << std::endl;
    stub_list_[target]->Inference(&context, request, &reply);
    std::cout << "inference finished " << target << std::endl;
    return;
}