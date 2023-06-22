#include "inference_request.pb.h"
#include "inference_request.grpc.pb.h"
#include "../eval/partition.hpp"

class InferClient{
public:
    InferClient(std::vector<std::string> ServerAddressList){}

    void StartInference(const EmbedRequest& request, InferenceReply* reply);
private:
    std::vector<std::unique_ptr<InferenceServer::Stub>> stubs_;
};




