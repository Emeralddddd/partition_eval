#pragma once
#include <grpcpp/grpcpp.h>
#include "inner_embed.hpp"
#include "embed_server.pb.h"
#include "embed_server.grpc.pb.h"

class EmbedServerImpl final : public EmbedServer::Service{
public:
    EmbedServerImpl(const std::string& type, int embed_size, int embed_dim);
    EmbedServerImpl();
    grpc::Status Lookup(grpc::ServerContext* context, const EmbedRequest* request, EmbedReply* reply) override;
private:
    std::unique_ptr<BaseInnerEmbedding> inner_embed;
};

class EmbeddingServerFactory {
public:
    static std::unique_ptr<BaseInnerEmbedding> createEmbedding(const std::string& embed_type, int embed_size, int embed_dim);
};

void RunServer();