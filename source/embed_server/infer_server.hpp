#include "embed_client.hpp"
#include "inference_request.pb.h"
#include "inference_request.grpc.pb.h"

class InferServerImpl final : public InferenceServer::Service{
public:
    InferServerImpl(std::string ServerAddress);
    grpc::Status Inference(grpc::ServerContext* context, const EmbedRequest* request, InferenceReply* reply) override;
private:
    std::unique_ptr<EmbedClient> embed_client_;
};