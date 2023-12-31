#include "merger.hpp"
#include <algorithm>
#include <queue>
#include <fstream>

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

void load_partial_result(std::string path, PartialResult& pr){
    std::ifstream in_file(path, std::ios::binary);
    if (!in_file) {
        std::cout << "open file error" << std::endl;
        return;
    }
    std::string input((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
    if (!in_file) {
        std::cout << "read file error" << std::endl;
        return;
    }
    if (!pr.ParseFromString(input)) {
        std::cout << "parse file error" << std::endl;
        return;
    }
}

[[deprecated("no need init")]]
void Merger::init(const std::string& path) { 
    vector<int> partition;
    vector<vector<double>> priority;
    load(path,partition,priority);
    n_parts_ = priority.size();
    n_embeds_ = priority[0].size();
    for(int i = 0; i < n_parts_; i++){
        assert(priority[i].size() == n_embeds_);
    }
    assert(partition.size() == n_embeds_);
    addPartition(partition,priority);
}

void Merger::resize(int n_embeds){
    n_embeds_ = n_embeds;
    weights_.resize(n_embeds_, vector<int>(n_parts_));
    for(int i = 0; i < n_parts_; i++){
        priority_[i].resize(n_embeds_,0);
    }
}

void Merger::update(const std::string &path){
    PartialResult pr;
    load_partial_result(path,pr);
    assert(pr.priority_maps().size() == n_parts_);
    addPartition(pr);
}

void Merger::addPartition(const PartialResult& pr){
    for(const auto &kv : pr.partition_map()){
        int key = kv.first;
        int value = kv.second;
        if(key >= n_embeds_) resize(key + 1);
        if(value >= 0){
            ++weights_[key][value];
        }
    }
    for (int i = 0; i < n_parts_; ++i) {
        const auto& pm = pr.priority_maps(i);
        for (const auto& kv : pm.map_field()) {
            int key = kv.first;
            float value = kv.second;
            if(key >= n_embeds_) resize(key + 1);
            if(value > 0){
                priority_[i][key] += value;
            }
        }
    }
}

void Merger::addPartition(const vector<int>& new_partition, \
    const vector<vector<double>>& new_priority){
    if(new_partition.size() != n_embeds_){
        throw std::runtime_error("partition size not match");
        return;
    }
    for(int i = 0; i < n_embeds_; i++){
        if(new_partition[i] >= 0){
            ++weights_[i][new_partition[i]];
        }
    }
    for(int i = 0; i < n_parts_; i++){
        for(int j = 0; j < n_embeds_;j++){
            priority_[i][j] += std::max(0.,new_priority[i][j]);
        }
    }
}

PartitionResult Merger::generatePartition(double hot_rate){
    PartitionResult ret;
    ret.partition.resize(n_embeds_,-1);

    #pragma omp parallel for num_threads(16)
    for(int i = 0; i < n_embeds_; i++){
        int part = std::max_element(weights_[i].begin(), weights_[i].end()) - weights_[i].begin();
        ret.partition[i] = weights_[i][part] > 0 ? part : -1;
    }
    if(hot_rate < 1e-6){
        ret.caches.resize(n_parts_);
        return ret;
    } 
    int hot_length = n_embeds_ * hot_rate;
    ret.caches.resize(n_parts_,vector<int>(hot_length,-1));

    #pragma omp parallel for num_threads(4)
    for(int i = 0; i < n_parts_; i++){
        std::priority_queue<std::pair<double,int>, std::vector<std::pair<double,int>>, std::greater<std::pair<double,int>>> minHeap;
        for(int j = 0; j < n_embeds_; j++){
            if(priority_[i][j] <= 0 || ret.partition[j] == i) continue;
            if(minHeap.size() < hot_length) minHeap.emplace(priority_[i][j],j);
            else if(minHeap.top().first < priority_[i][j]){
                minHeap.pop();
                minHeap.emplace(priority_[i][j],j);
            }
        }
        for(int j = 0; j < hot_length && !minHeap.empty(); j++){
            ret.caches[i][j] = minHeap.top().second;
            minHeap.pop();
        }
    }
    return ret;
}

void Merger::savePartitionToNpz(const PartitionResult &pr,const std::string &path){
    int n = pr.partition.size();
    int n_part = pr.caches.size();
    cnpy::npz_save(path,"embed_partition",&pr.partition[0],{n},"w");
    for(int i = 0; i < n_part; i++){
        int cache_size = pr.caches[i].size();
        cnpy::npz_save(path,std::to_string(i),&pr.caches[i][0],{cache_size},"a");
    }
    return;
}