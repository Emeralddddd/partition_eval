#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

class MisraGries {
private:
    std::unordered_map<int, int> counter_;
    int k_;

public:
    MisraGries(int k) : k_(k) {}

    void add(int x);

    std::unordered_set<int> getFrequentItems();

    bool isFrequent(int i){return counter_.count(i) && counter_[i] > 0;}

    void clear(){counter_.clear();}

    int size(){return counter_.size();}
};

class BasePartition{
public:
    BasePartition(int n_parts) : n_parts_(n_parts),embed_cnt_(n_parts,0),access_cnt_(n_parts,0) {}
    std::vector<int> getPartitions(std::vector<int> &data);
    int getPartition(int u);
    int getPartitionCnt(int part);
    int getAccessCnt(int part);
    int getEmbedNums(){return n_embeds_;}
    int getPartNums(){return n_parts_;}
    virtual void expandEmbedRange(int n);
    virtual void processRequest(std::vector<int> &data) = 0;
    double getAvgCost(){return queryCnt_>0?(double)allCost_ / queryCnt_:0;}
    double getCacheHitRate(){return queryCnt_>0?(double)cacheHit_/queryCnt_:0;}
    double getLocalRate(){return queryCnt_>0?(double)localCnt_/queryCnt_:0;}
    int getChangedEmbed(){return changeEmbed_;}
    int getRemoteAccess(){return remoteAccessCnt_;}
    void clear(){
        allCost_ = 0;
        queryCnt_ = 0;
        cacheHit_ = 0;
        localCnt_ = 0;
        changeEmbed_ = 0;
        remoteAccessCnt_ = 0;
    }
protected:
    int n_parts_;
    int max_id_ = -1;
    int n_embeds_ = 0;
    std::vector<int> partition_;
    std::vector<int> embed_cnt_;
    std::vector<int> access_cnt_;
    long long allCost_ = 0;
    int queryCnt_ = 0;
    int cacheHit_ = 0;
    int localCnt_ = 0;
    int changeEmbed_ = 0;
    int remoteAccessCnt_ = 0;
};

class StaticPartition : public BasePartition{
public:
    StaticPartition(int n_parts, std::string path, double hot_rate);
    void load_partition(std::string path, double hot_rate);
    void load_query_partition(std::string path);
    virtual void processRequest(std::vector<int> &data) override;
private:
    int hot_size_;
    int query_counter_ = 0;
    std::vector<int> query_partition_;
    std::vector<std::unordered_set<int>> local_hot_;
};

class ScorePartition : public BasePartition{
public:
    ScorePartition(int n_parts) : BasePartition(n_parts),mg_(0),local_hot_(n_parts_,MisraGries(30000)){}
    void updatePartition(std::vector<int> &data);
    void expandEmbedRange(int n) override;
    virtual void processRequest(std::vector<int> &data) override;
    int getGlobalHotSize(){return mg_.size();}
    void load_partition_base(std::string partition_path);
    void load_partition(std::string partition_path);
    void load_partition(std::string partition_path, std::vector<std::vector<int>> &data, double hot_rate);
    void load_partition(std::string partition_path, std::string scores_path, double hot_rate);
    void updateGlobalHot(){global_hot_ = mg_.getFrequentItems();}
    void resetPartition(){
        mg_.clear();
        int n = scores_[0].size();
        for(auto &s : scores_){
            std::fill(s.begin(), s.end(), 0);
        }
        std::fill(partition_.begin(),partition_.end(),-1);
    }
private:
    double alpha_ = 1;
    double beta_ = 1;
    double aging_factor_ = 1.0;
    MisraGries mg_;
    std::vector<std::vector<int>> scores_;
    std::unordered_set<int> global_hot_;
    std::vector<MisraGries> local_hot_;
};
