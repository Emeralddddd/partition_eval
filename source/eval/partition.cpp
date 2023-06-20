#include <map>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "cnpy.h"

#include "partition.hpp"

using std::map;
using std::vector;
using std::unordered_set;
using std::string;

int BasePartition::getPartition(int u){
    return partition_[u];
}

vector<int> BasePartition::getPartitions(std::vector<int> &data){
    std::vector<int> ret(data.size(),0);
    for(int i = 0; i < data.size(); i++){
        ret[i] = data[i] >= max_id_ ? -1 :partition_[data[i]];
    }
    return std::move(ret);
}

int BasePartition::getPartitionCnt(int i){
    return i>=0 && i < n_parts_? embed_cnt_[i]:-1;
}

int BasePartition::getAccessCnt(int i){
    return i>=0 && i < n_parts_? access_cnt_[i]:-1;
}

void BasePartition::expandEmbedRange(int n){
    partition_.resize(n,-1);
    max_id_ = n;
}


StaticPartition::StaticPartition(int n_parts, std::string path, double hot_rate) : BasePartition(n_parts) {
    load_partition_from_npz(path, hot_rate);
}

void StaticPartition::load_partition_from_npz(std::string path, double hot_rate){
    cnpy::npz_t partition = cnpy::npz_load(path);
    partition_ = partition["embed_partition"].as_vec<int>();
    query_partition_ = partition["data_partition"].as_vec<int>();
    n_parts_ = partition.size() - 2;
    embed_cnt_.resize(n_parts_,0);
    access_cnt_.resize(n_parts_,0);
    max_id_ = partition_.size()-1;
    n_embeds_ = max_id_;
    for(int p : partition_){
        embed_cnt_[p]++;
    }
    std::cout << hot_rate << std::endl;
    if(hot_rate > 1e-6){
        vector<vector<int>> priorList(n_parts_);
        for(int i = 0; i < n_parts_; i++) priorList[i] = partition[std::to_string(i)].as_vec<int>();
        cache_ = new StaticCache(priorList, hot_rate);
    }else{
        cache_ = new EmptyCache();
        std::cout << "using empty cache" << std::endl;
    }

    // cache_ = new GlobalCache(n_parts_,30000);
}

void StaticPartition::load_partition_from_merger(const PartitionResult& pr){
    partition_ = std::move(pr.partition);
    n_parts_ = pr.caches.size();
    embed_cnt_.resize(n_parts_,0);
    access_cnt_.resize(n_parts_,0);
    max_id_ = pr.partition.size()-1;
    n_embeds_ = max_id_;
    for(int p : partition_){
        embed_cnt_[p]++;
    }
    vector<vector<int>> priorList(n_parts_);
    for(int i = 0; i < n_parts_; i++) priorList[i] = pr.caches[i];
    cache_ = new StaticCache(priorList, 1.);
}



void StaticPartition::load_query_partition(string path){
    cnpy::npz_t partition = cnpy::npz_load(path);
    query_partition_ = partition["data_partition"].as_vec<int>();
}

void StaticPartition::processRequest(vector<int> &data){
    int n = data.size();
    int cost = 0;
    vector<int> partCnt(n_parts_, 0);
    vector<int> parts = getPartitions(data);
    vector<bool> dataValid(n,false);
    vector<int> remoteAccess(n_parts_,0);
    vector<int> cacheHitCnt(n_parts_,0);
    vector<int> localAccessCnt(n_parts_,0);
    int targetPart;
    for(int i = 0; i < n; i++){
        queryCnt_++;
        if(data[i] <= max_id_ && parts[i] != -1){
            dataValid[i] = true;
        }
    }
    for(int j = 0; j < n_parts_; j++){
        vector<bool> isAccessed(4,false);
        vector<int> cacheResults = cache_->query(data, j);
        for(int i = 0; i < n ; i++){
            if(!dataValid[i]) continue;
            if(parts[i] != j){ 
                if(cacheResults[i] == 0){
                    partCnt[j]++;
                    if(!isAccessed[parts[i]]){
                        isAccessed[parts[i]] = true;
                        remoteAccess[j]++;
                    }
                }else cacheHitCnt[j]++;
            }else localAccessCnt[j]++; 
        }
    }
    // targetPart = min_element(remoteAccess.begin(),remoteAccess.end()) - remoteAccess.begin();
    targetPart = min_element(partCnt.begin(),partCnt.end()) - partCnt.begin();
    // todo 选择targetPart的方式
    // vector<int> partCnt(n_parts, 0);
    access_cnt_[targetPart]++;
    allCost_ += partCnt[targetPart];
    cacheHit_ += cacheHitCnt[targetPart];
    localCnt_ += localAccessCnt[targetPart];
    remoteAccessCnt_ += remoteAccess[targetPart];
    cache_->update(data,0);
}

void ScorePartition::load_partition_base(string partition_path){
    cnpy::npz_t partition = cnpy::npz_load(partition_path);
    partition_ = partition["embed_partition"].as_vec<int>();
    expandEmbedRange(partition_.size());
    n_embeds_ = max_id_;
    for(int p : partition_){
        embed_cnt_[p]++;
    }
}

void ScorePartition::load_partition(string partition_path){
    load_partition_base(partition_path);
    for(int i = 0; i < partition_.size(); i++){
        scores_[i][partition_[i]] = partition_.size()/2;
    }
}

void ScorePartition::load_partition(string partition_path, vector<vector<int>> &data, double hot_rate){
    load_partition_base(partition_path);
    int rowNums = 20000000;
    for(int k = 0; k < rowNums; k++){
        auto row = data[k];
        int n = row.size();
        map<int, int> freq;
        for(int i = 0; i < n; i++){
            if(partition_[row[i]] != -1){
                freq[row[i]] += 1;
            }
        }
        for (auto it = freq.begin(); it != freq.end(); ++it) {
            for(auto it2 = freq.begin(); it2 != freq.end(); ++it2){
                if(it != it2) scores_[it->first][partition_[it2->first]] += it->second * it2->second;
            }
        }
    }
    string filename = "scores.csv";
    std::ofstream outfile(filename);
    if(!outfile.is_open()){
        outfile.open(filename.c_str(), std::ios::out);
        if (!outfile) {
            std::cout << "Error: failed to create file " << filename << std::endl;
            return;
        }
    }
    for (const auto& innerVec : scores_) {
        for (const auto& element : innerVec) {
            outfile << element << " ";
        }
        outfile << std::endl;
    }
    outfile.close();
}

void ScorePartition::load_partition(string partition_path,string scores_path, double hot_rate){
    load_partition_base(partition_path);
    string filename = scores_path;
    std::ifstream file(filename);
    int i = 0 ,j = 0;
    if (file.is_open()) {
        string line;
        while(getline(file,line)){
            try{
                std::stringstream ss(line);
                string value;
                while(getline(ss,value,' ')){
                    scores_[i][j++] = std::stoi(value) * 100;
                }
                i++;
                j = 0;
            }catch(...){
                std::cout << i << " " << j << std::endl;
            }
        }
        file.close();
        std::cout << i << " " << j << std::endl;
    }
    std::cout <<scores_.size() << " " << scores_[0].size() << "scores load finished" << std::endl;
}

void ScorePartition::updatePartition(vector<int> &data){
    int n = data.size();
    map<int, int> freq;
    vector<int> partCnt(n_parts_,0);
    for(int i = 0; i < n; i++){
        if(data[i] > max_id_) expandEmbedRange(data[i] + 1);
        if(partition_[data[i]] == -1){
            int part = data[i] % n_parts_;
            partition_[data[i]] = part;
            embed_cnt_[part]++;
            n_embeds_++;
        }
        // if(mg_.isFrequent(data[i])) continue;
        partCnt[partition_[data[i]]]++;
        freq[data[i]] += 1;
    }
    int targetPart = std::max_element(partCnt.begin(),partCnt.end()) - partCnt.begin();
    for (auto it = freq.begin(); it != freq.end(); ++it) {
        for(auto it2 = freq.begin(); it2 != freq.end(); ++it2){
            if(it != it2){
                scores_[it->first][partition_[it2->first]] *= aging_factor_;
                scores_[it->first][partition_[it2->first]] += it->second * it2->second;
            } 
        }
        int maxScore = std::numeric_limits<int>::min();
        int maxPart = 0;
        for(int i = 0; i < n_parts_; i++){
            int score = scores_[it->first][i] - beta_ * embed_cnt_[i] - alpha_ * access_cnt_[i];
            if(score > maxScore){
                maxScore = score;
                maxPart = i;
            }
        }
        if(partition_[it->first] != maxPart) changeEmbed_++;
        embed_cnt_[partition_[it->first]]--;
        embed_cnt_[maxPart]++;
        partition_[it->first] = maxPart;
    }
}

void ScorePartition::expandEmbedRange(int n){
    BasePartition::expandEmbedRange(n);
    scores_.resize(n,vector<int>(n_parts_,0));
}

void ScorePartition::processRequest(vector<int> &data){
    int n = data.size();
    int n_parts = getPartNums();
    vector<int> parts = getPartitions(data);
    vector<int> partCnt(n_parts, 0);
    vector<int> cacheHitCnt(n_parts_,0);
    vector<int> localAccessCnt(n_parts_,0);
    vector<int> remoteAccess(n_parts,0);
    vector<bool> dataValid(n,false);
    for(int i = 0; i < n; i++){
        if(data[i] <= max_id_ && parts[i] != -1){
            dataValid[i] = true;
            queryCnt_++;
        }
    } 
    for(int j = 0; j < n_parts; j++){
        vector<bool> isAccessed(4,false);
        vector<int> cacheResults = cache_->query(data,j);
        for(int i = 0; i < n ; i++){
            if(!dataValid[i]) continue;
            if(parts[i] != j){ 
                if(cacheResults[i] == 0){
                    partCnt[j]++;
                    if(!isAccessed[parts[i]]){
                        isAccessed[parts[i]] = true;
                        remoteAccess[j]++;
                    }
                }else cacheHitCnt[j]++;
            }else localAccessCnt[j]++; 
        }
    }
    // int targetPart = min_element(partCnt.begin(), partCnt.end()) - partCnt.begin();
    int targetPart = min_element(remoteAccess.begin(), remoteAccess.end()) - remoteAccess.begin();
    // for(const int x : data) local_hot_[targetPart].add(x);
    // int targetPart = queryCnt_ % n_parts_;
    access_cnt_[targetPart]++;
    allCost_ += partCnt[targetPart];
    cacheHit_ += cacheHitCnt[targetPart];
    localCnt_ += localAccessCnt[targetPart];
    remoteAccessCnt_ += remoteAccess[targetPart];
    // allCost_ += n - partCnt[targetPart] - global_hot_cnt;
    cache_ ->update(data,0);
    updatePartition(data);
}


void MisraGries::add(int x) {
    if (counter_.find(x) != counter_.end()) {
        counter_[x]++;
    } else if (counter_.size() < k_) {
        counter_[x] = 1;
    } else {
        for (auto it = counter_.begin(); it != counter_.end(); it) {
            it->second--;
            if (it->second == 0) {
                it = counter_.erase(it);
            }else{
                it++;
            }
        }
    }
}

unordered_set<int> MisraGries::getFrequentItems() {
    unordered_set<int> ret;
    ret.reserve(counter_.size());
    for (auto &p : counter_) {
        if (p.second >= 0) {
            ret.insert(p.first);
        }
    }
    return std::move(ret);
}

StaticCache::StaticCache(const vector<vector<int>> &priorList, double hotRate){
    n_parts_ = priorList.size();
    size_ = priorList[0].size() * hotRate;
    data_.resize(n_parts_);
    for(int i = 0; i < n_parts_; i++){
        data_[i] = unordered_set<int>(priorList[i].begin(),priorList[i].begin() + size_);
    }
}

vector<int> StaticCache::query(const vector<int> &input, int part){
    int n = input.size();
    vector<int> res(n,0);
    for(int i = 0; i < n; i++){
        if(data_[part].count(input[i])) res[i] = 1;
    }
    return res;
}