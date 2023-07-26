#pragma once
#include "inference_request.pb.h"
#include "inference_request.grpc.pb.h"
#include "../merge/merger.hpp"
#include "../utils/utils.hpp"
#include <cassert>
#include <grpcpp/grpcpp.h>
#include <numeric>
#include <algorithm>

class Dispatcher{
public:
    Dispatcher(int n_parts, std::vector<std::string> ServerAddressList, bool stats_only) : n_parts_(n_parts), stats_only_(stats_only){
        assert(ServerAddressList.size() == n_parts_);
        caches_.resize(n_parts_);
        remote_time_vecs.resize(n_parts_,std::vector<std::vector<int>>(n_parts_));
        remote_data_cnt.resize(n_parts_,std::vector<int>(n_parts_));
        remote_request_cnt.resize(n_parts_,std::vector<int>(n_parts_));
        for(int i = 0; i < n_parts_; i++){
            auto channel = grpc::CreateChannel(ServerAddressList[i], grpc::InsecureChannelCredentials());
            stub_list_.emplace_back(InferenceServer::NewStub(channel));
        }
    }
    Dispatcher(int n_parts, std::vector<std::string> ServerAddressList):Dispatcher(n_parts,ServerAddressList,false){}
    void resize(int n_embeds){
        n_embeds_ = n_embeds;
        partition_.resize(n_embeds_, -1);
        std::cout << "resize to " << n_embeds_ << std::endl;
    }
    void LoadPartitionNPZ(std::string path, double hr);
    void LoadPartitionMerge(PartitionResult&& pr);
    void DispatchRequest(const std::vector<int> &input);
    void clearTime(){
        time_vec_.clear();
        remoteCnt_ = 0;
        nodeCnt_ = 0;
        for(int i = 0; i < n_parts_;i++){
            for(auto v : remote_time_vecs[i]) v.clear();
            remote_data_cnt[i].clear();
            remote_request_cnt[i].clear();
        }
    }
    double getAvgTime(){
        return calculateAverage(time_vec_);
    }
    int getTailTime(){
        return calculatePercentile(time_vec_, 0.99);
    }
    int getRemoteCount(){
        return remoteCnt_;
    }
    int getNodeCount(){
        return nodeCnt_;
    }
    void getDebugInfo(){
        std::cout << "Debug Info :" << std::endl;
        for(int i = 0; i < n_parts_; i++){
            for(int j = 0; j < n_parts_; j++){
                double avg_time = calculateAverage(remote_time_vecs[i][j]);
                double p95_time = calculatePercentile(remote_time_vecs[i][j], .99);
                std::cout << i << "->" <<j << " " <<remote_data_cnt[i][j] << " " \
                    << remote_request_cnt[i][j] << " " << avg_time << " " << p95_time << std::endl;
            }
        }
    }

private:
    int n_embeds_, n_parts_ = 0, remoteCnt_ = 0, nodeCnt_ = 0;
    bool stats_only_;
    std::vector<int> partition_;
    std::vector<std::unordered_set<int>> caches_;
    std::vector<std::unique_ptr<InferenceServer::Stub>> stub_list_;
    std::vector<int> time_vec_;
    std::vector<std::vector<std::vector<int>>> remote_time_vecs;
    std::vector<std::vector<int>> remote_data_cnt;
    std::vector<std::vector<int>> remote_request_cnt;
};