#pragma once
#include "cnpy.h"
#include <algorithm>

class Merger{
public:
    Merger(int n_parts) : n_parts_(n_parts) {}

    void init(const std::vector<int>& partition);

    void addPartition(const std::vector<int>& new_partition);

    std::vector<int> generatePartition();
private:
    int n_parts_;
    int n_embeds_;
    std::vector<std::vector<int>> weights_;
    std::vector<std::vector<double>> priority_;
};
