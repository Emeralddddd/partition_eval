#pragma once
#include <grpcpp/grpcpp.h>
#include "inference_request.pb.h"
#include "inference_request.grpc.pb.h"
#include "embed_server.pb.h"
#include "embed_server.grpc.pb.h"
#include <mutex>

class InferServerImpl final : public InferenceServer::Service{
public:
    InferServerImpl(int n_part, int rank, std::vector<std::string> ServerAddressList);
    grpc::Status Inference(grpc::ServerContext* context, const InferenceRequest* request, InferenceReply* reply) override;
private:
    std::vector<std::unique_ptr<EmbedServer::Stub>> stub_list_;
    int n_part_, rank_;
    struct AsyncClientCall {
        EmbedReply reply;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<EmbedReply>> response_reader;
    };
    grpc::CompletionQueue cq_;
};

class InferServerSyncImpl : public InferenceServer::Service{
public:
    InferServerSyncImpl(int n_part, int rank, std::vector<std::string> ServerAddressList);
    grpc::Status Inference(grpc::ServerContext* context, const InferenceRequest* request, InferenceReply* reply) override;
private:
    std::vector<std::unique_ptr<EmbedServer::Stub>> stub_list_;
    int n_part_, rank_;
};

