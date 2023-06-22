#include "infer_server.hpp"

using std::vector;
using grpc::Status;

InferServerImpl::InferServerImpl(std::string ServerAddress){
    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    embed_client_ = std::make_unique<EmbedClient>(channel);
}

grpc::Status InferServerImpl::Inference(grpc::ServerContext* context, const EmbedRequest* request, InferenceReply* reply){
    int n = request -> data_size();
    EmbedReply embed_reply;
    embed_client_->RemoteLookup(*request, &embed_reply);
    for(int i = 0; i < n; i++){
        reply->add_data(request->data(i));
    }
    return Status::OK;
}