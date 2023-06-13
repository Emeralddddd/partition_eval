#pragma once
#include "cnpy.h"
#include <algorithm>

class PartitionResult{
public:
    std::vector<int> partition;
    std::vector<std::vector<int>> caches;
};

class Merger{
public:
    Merger() {}
    Merger(int n_parts) : n_parts_(n_parts) {}

    void init(const std::string& path);

    void update(const std::string &path);

    void addPartition(const std::vector<int>& new_partition, \
    const std::vector<std::vector<double>>& new_priority);
    

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
