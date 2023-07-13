#pragma once
#include "cnpy.h"
#include <algorithm>
#include "partial_result.pb.h"

class PartitionResult{
public:
    std::vector<int> partition;
    std::vector<std::vector<int>> caches;
};

class BaseMerger{
public:
    BaseMerger(int n_parts) : n_parts_(n_parts) {};
    virtual void update(const std::string &path) = 0;
    virtual PartitionResult generatePartition(double hot_rate) = 0;
protected:
    int n_parts_;
    int n_embeds_;
    PartitionResult pr_;
};

class Merger : public BaseMerger{
public:
    Merger() = delete;
    Merger(int n_parts) : BaseMerger(n_parts) {
        weights_.resize(1, std::vector<int>(n_parts_));
        priority_.resize(n_parts);
        caches_.resize(n_parts);
    }

    void init(const std::string& path);

    void update(const std::string &path) override;

    void addPartition(const std::vector<int>& new_partition, \
    const std::vector<std::vector<double>>& new_priority);
    void addPartition(const PartialResult& pr);
    void resize(int n_embeds);

    PartitionResult generatePartition(double hot_rate) override;
    void savePartitionToNpz(const std::string &path);
    int getLastEmbedChanged(){return last_embed_changed_;}
private:
    int last_embed_changed_ = 0;
    std::vector<std::vector<int>> weights_;
    std::vector<std::vector<double>> priority_;
    std::vector<std::vector<int>> caches_;
};

class NaiveMerger : public BaseMerger{
public:
    NaiveMerger(int n_parts) : BaseMerger(n_parts) {
        caches_.resize(n_parts);
        weights_.resize(1, std::vector<int>(n_parts_));
        pr_.caches.resize(n_parts);
        priority_.resize(n_parts);
        partition_.resize(1,-1);
    }
    void update(const std::string &path) override;
    void addPartition(const PartialResult& pr);
    PartitionResult generatePartition(double hot_rate) override;
    void resize(int n_embeds);
private:
    std::vector<int> partition_;
    std::vector<std::vector<int>> weights_;
    std::vector<std::vector<double>> priority_;
    std::vector<std::vector<int>> caches_;
};

void load(std::string path, std::vector<int>& partition, \
    std::vector<std::vector<double>>& priority);

void load_partial_result(std::string path, PartialResult& pr);
