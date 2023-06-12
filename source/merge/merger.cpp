#include "merger.hpp"

using std::vector;

void Merger::init(const vector<int>& partition){
    n_embeds_ = partition.size();
    weights_.resize(n_embeds_, vector<int>(n_parts_, 0));
    addPartition(partition);
}

void Merger::addPartition(const vector<int>& new_partition){
    if(new_partition.size() != n_embeds_){
        throw std::runtime_error("partition size not match");
        return;
    }
    for(int i = 0; i < n_embeds_; i++){
        if(new_partition[i] >= 0){
            ++weights_[i][new_partition[i]];
        }
    }
}

vector<int> Merger::generatePartition(){
    vector<int> partition(n_embeds_, -1);
    for(int i = 0; i < n_embeds_; i++){
        partition[i] = std::max_element(weights_[i].begin(), weights_[i].end()) - weights_[i].begin();
    }
    return partition;
}