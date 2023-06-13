#include "merger.hpp"
#include <algorithm>
#include <queue>

using std::vector;

void load(std::string path, vector<int>& partition, vector<vector<double>>& priority){
    partition = cnpy::npz_load(path,"embed_partition").as_vec<int>();
    cnpy::NpyArray pr = cnpy::npz_load(path,"priority");
    size_t numRows = pr.shape[0];
    size_t numCols = pr.shape[1];
    priority.resize(numRows,vector<double>(numCols));
    for(int i = 0; i < numRows; i++){
        for(int j = 0; j < numCols; j++){
            priority[i][j] = (double)pr.data<float>()[i*numCols + j];
        }
    } 
    return;
}

void Merger::init(const std::string& path){
    vector<int> partition;
    vector<vector<double>> priority;
    load(path,partition,priority);
    n_parts_ = priority.size();
    n_embeds_ = priority[0].size();
    for(int i = 0; i < n_parts_; i++){
        assert(priority[i].size() == n_embeds_);
    }
    assert(partition.size() == n_embeds_);
    weights_.resize(n_embeds_, vector<int>(n_parts_, 0));
    priority_.resize(n_parts_, vector<double>(n_embeds_,0));
    addPartition(partition,priority);
}

void Merger::update(const std::string &path){
    vector<int> partition;
    vector<vector<double>> priority;
    load(path,partition,priority);
    for(int i = 0; i < n_parts_; i++){
        assert(priority[i].size() == n_embeds_);
    }
    assert(priority.size() == n_parts_);
    assert(partition.size() == n_embeds_);
    addPartition(partition,priority);
}

void Merger::addPartition(const vector<int>& new_partition, \
    const vector<vector<double>>& new_priority){
    if(new_partition.size() != n_embeds_){
        throw std::runtime_error("partition size not match");
        return;
    }
    #pragma omp parallel for num_threads(32)
    for(int i = 0; i < n_embeds_; i++){
        if(new_partition[i] >= 0){
            ++weights_[i][new_partition[i]];
        }
    }
    #pragma omp parallel for num_threads(32)
    for(int i = 0; i < n_parts_; i++){
        for(int j = 0; j < n_embeds_;j++){
            if(j > n_embeds_ - 5) std::cout << new_priority[i][j] << " ";
            priority_[i][j] += std::max(0.,new_priority[i][j]);
        }
        std::cout << std::endl;
    }
}

PartitionResult Merger::generatePartition(double hot_rate){
    int hot_length = n_embeds_ * hot_rate;
    PartitionResult ret;
    ret.partition.resize(n_embeds_,-1);
    ret.caches.resize(n_parts_,vector<int>(n_embeds_,-1));
    for(int i = 0; i < n_embeds_; i++){
        int part = std::max_element(weights_[i].begin(), weights_[i].end()) - weights_[i].begin();
        ret.partition[i] = weights_[i][part] > 0 ? part : -1;
    }
    for(int i = 0; i < n_parts_; i++){
        std::priority_queue<std::pair<double,int>, std::vector<std::pair<double,int>>, std::greater<std::pair<double,int>>> minHeap;
        for(int j = 0; j < n_embeds_; j++){
            if(j > n_embeds_ - 5) std::cout << priority_[i][j] << " ";
            if(priority_[i][j] <= 0 || ret.partition[j] == i) continue;
            if(minHeap.size() < hot_length) minHeap.emplace(priority_[i][j],j);
            else if(minHeap.top().first < priority_[i][j]){
                minHeap.pop();
                minHeap.emplace(priority_[i][j],j);
            }
        }
        std::cout << std::endl;
        for(int j = 0; j < hot_length && !minHeap.empty(); j++){
            ret.caches[i][j] = minHeap.top().second;
            minHeap.pop();
        }
    }
    return ret;
}