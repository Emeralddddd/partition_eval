#include "embed_server.pb.h"
#include "embed_server.grpc.pb.h"
#include <grpcpp/grpcpp.h>



class EmbedClient{
public:
    EmbedClient(std::shared_ptr<grpc::Channel> channel) : stub_(EmbedServer::NewStub(channel)){}

    void RemoteLookup(const std::vector<int>& input, std::vector<std::vector<float>>& output);
private:
    std::unique_ptr<EmbedServer::Stub> stub_;
};

