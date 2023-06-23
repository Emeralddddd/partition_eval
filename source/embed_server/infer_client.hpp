#pragma once
#include "inference_request.pb.h"
#include "inference_request.grpc.pb.h"
#include "../merge/merger.hpp"
#include <cassert>
#include <grpcpp/grpcpp.h>

class Dispatcher{
public:
    Dispatcher(int n_parts, std::vector<std::string> ServerAddressList) : n_parts_(n_parts){
        assert(ServerAddressList.size() == n_parts_);
        for(int i = 0; i < n_parts_; i++){
            auto channel = grpc::CreateChannel(ServerAddressList[i], grpc::InsecureChannelCredentials());
            stub_list_.emplace_back(InferenceServer::NewStub(channel));
        }
    }
    void resize(int n_embeds){
        n_embeds_ = n_embeds;
        partition_bits.resize(n_embeds_);
    }
    void LoadPartitionNPZ(std::string path, double hr);
    void LoadPartitionMerge(const PartitionResult& pr);
    void DispatchRequest(const std::vector<int> &input);

private:
    int n_embeds_, n_parts_;
    std::vector<int> partition_bits;
    std::vector<std::unique_ptr<InferenceServer::Stub>> stub_list_;
};