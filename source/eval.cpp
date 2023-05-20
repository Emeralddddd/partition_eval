#include "eval.hpp"
#include <algorithm>

using std::vector;
using std::max_element;

void Eval::processRequest(vector<int> &data){
    int n = data.size();
    int n_parts = partition_->getPartNums();
    vector<int> parts = partition_->getPartitions(data);
    vector<int> partCnt(n_parts, 0);
    for(int i = 0; i < n; i++){
        if(parts[i] != -1){
            partCnt[parts[i]] ++;
            requestCnt_ ++;
        } 
    }
    auto it = max_element(partCnt.begin(), partCnt.end());
    int localCost = *it;
    accessCnt_[it - partCnt.begin()]++;
    allCost_ += n - localCost;
}

void Eval::setPartition(BasePartition* partition){
        partition_ = partition;
        accessCnt_.resize(partition_->getEmbedNums(),0);
    }

void Eval::clear(){
    allCost_ = 0;
    requestCnt_ = 0;
}