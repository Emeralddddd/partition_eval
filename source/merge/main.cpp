#include "cnpy.h"
#include <algorithm>
#include <omp.h>
#include <chrono>

using std::vector;
using std::string;

vector<int> load_partition(std::string path){
  cnpy::npz_t data = cnpy::npz_load(path);
  vector<int> partition = data["embed_partition"].as_vec<int>();
  return partition;
}

class Merger{
public:
    Merger(int n_parts) : n_parts_(n_parts) {}

    void init(const vector<int>& partition){
        n_embeds_ = partition.size();
        weights_.resize(n_embeds_, vector<int>(n_parts_, 0));
        addPartition(partition);
    }

    void addPartition(const vector<int>& new_partition){
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

    vector<int> generatePartition(){
        vector<int> partition(n_embeds_, -1);
        for(int i = 0; i < n_embeds_; i++){
            partition[i] = std::max_element(weights_[i].begin(), weights_[i].end()) - weights_[i].begin();
        }
        return partition;
    }
private:
    int n_parts_;
    int n_embeds_;
    vector<vector<int>> weights_;
};

auto main(int argc, char* argv[]) -> int{
    Merger merger(4);
    merger.init(load_partition("/data/1/zhen/criteo-tb/partition/new/day0_1m.npz"));
    for(int i = 2;i < 80; i++){
        auto start = std::chrono::high_resolution_clock::now();
        merger.addPartition(load_partition("/data/1/zhen/criteo-tb/partition/new/day0_" + std::to_string(i) + "m.npz"));
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << i <<"th time: " << time << std::endl;
    }
    vector<int> partition = merger.generatePartition();
    cnpy::npz_save("/data/1/zhen/criteo-tb/partition/merged/day0_79m.npz","embed_partition",&partition[0],{partition.size()},"w");
    return 0;
}