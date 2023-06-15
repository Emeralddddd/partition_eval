#include <vector>
#include "partition.hpp"


class Eval{
public:
    Eval() = default;
    Eval(BasePartition* partition) : partition_(partition), accessCnt_(partition->getPartNums(),0) {}
    void setPartition(BasePartition* partition);
    void processRequest(std::vector<int> &data);
    double getAvgCost(){return (double)allCost_ / requestCnt_;}
    int getAccessCnt(int i){return i>=0 && i <partition_->getEmbedNums() ? accessCnt_[i]:-1;}
    void clear();
private:
    long long allCost_ = 0;
    int requestCnt_ = 0;
    std::vector<int> accessCnt_;
    BasePartition* partition_;
};