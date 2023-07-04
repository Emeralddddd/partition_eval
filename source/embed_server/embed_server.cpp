#include "embed_server.hpp"
#include <limits>

using grpc::Status;
using std::vector;

constexpr static int default_embed_size = std::numeric_limits<int>::max();
constexpr static int default_embed_dim = 1024;

std::unique_ptr<BaseInnerEmbedding> EmbeddingServerFactory::createEmbedding(const std::string& type, int embed_size, int embed_dim){
    if (type == "random") {
        return std::make_unique<RandomInnerEmbedding>(embed_size, embed_dim);
    }  
    return nullptr;
}

EmbedServerImpl::EmbedServerImpl() : EmbedServerImpl("random",default_embed_size,default_embed_dim){}

EmbedServerImpl::EmbedServerImpl(const std::string& embed_type, int embed_size, int embed_dim)
 : inner_embed(EmbeddingServerFactory::createEmbedding(embed_type, embed_size, embed_dim)){}

Status EmbedServerImpl::Lookup(grpc::ServerContext* context, const EmbedRequest* request, EmbedReply* reply){
    int n = request->data_size();
    vector<int> input(n);
    vector<vector<float>> output;
    for(int i = 0; i < n; i++){
        input[i] = request->data(i);
    }
    inner_embed->query(input,output);
    reply->mutable_embed_values()->Reserve(n);
    int dim = output[0].size();
    for(int i = 0; i < n; i++){
        SingleEmbed* se = reply->add_embed_values();
        reply->add_pos(reply->pos(i));
        se->mutable_data() -> Reserve(dim);
        for(int j = 0; j < output.size(); j++){
            se -> add_data(output[i][j]);
        }
    }
    return Status::OK;
}

void RunServer(){
    std::string server_address("0.0.0.0:50051");
    EmbedServerImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}