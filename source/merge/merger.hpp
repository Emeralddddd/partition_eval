#pragma once
#include "cnpy.h"
#include <algorithm>
#include "partial_result.pb.h"

class PartitionResult{
public:
    std::vector<int> partition;
    std::vector<std::vector<int>> caches;
};

class Merger{
public:
    Merger() = delete;
    Merger(int n_parts) : n_parts_(n_parts) {
        weights_.resize(1, std::vector<int>(n_parts_));
        priority_.resize(n_parts);
        caches_.resize(n_parts);
    }

    void init(const std::string& path);

    void update(const std::string &path);

    void addPartition(const std::vector<int>& new_partition, \
    const std::vector<std::vector<double>>& new_priority);
    void addPartition(const PartialResult& pr);
    void resize(int n_embeds);
    

    PartitionResult generatePartition(double hot_rate);
private:
    int n_parts_;
    int n_embeds_;
    std::vector<std::vector<int>> weights_;
    std::vector<std::vector<double>> priority_;
    std::vector<std::vector<int>> caches_;
};

void load(std::string path, std::vector<int>& partition, \
    std::vector<std::vector<double>>& priority);

void load_partial_result(std::string path, PartialResult& pr);
