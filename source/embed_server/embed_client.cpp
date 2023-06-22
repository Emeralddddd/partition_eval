#include "embed_client.hpp"

using grpc::ClientContext;
using grpc::Status;
using std::vector;

void EmbedClient::RemoteLookup(const std::vector<int>& input, std::vector<std::vector<float>>& output){
    EmbedRequest request;
    EmbedReply reply;
    int n = input.size();
    for(int i = 0; i < n; i++){
        request.add_data(input[i]);
    }
    RemoteLookup(request, &reply);
    int dim = reply.embed_values_size();
    output.resize(n,vector<float>(dim));
    for(int i = 0; i < n; i++){
        auto se = reply.embed_values(i);
        for(int j = 0; j < dim; j++){
            output[i][j] = se.data(j);
        }
    }
    return;
}

void EmbedClient::RemoteLookup(const EmbedRequest& request, EmbedReply *reply){
    ClientContext context;
    Status status = stub_->Lookup(&context,request,reply);
    if (!status.ok()) {
      std::cout << "gRPC call error: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
}