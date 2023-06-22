#include "embed_client.hpp"
#include <iostream>

using std::vector;

auto main()->int{
    EmbedClient remoteEmbed(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
    vector<int> input(10);
    vector<vector<float>> output;
    remoteEmbed.RemoteLookup(input,output);
    for(auto v : output){
        for(auto f : v){
            std::cout << f;
        }
        std::cout << std::endl;
    }
    return 0;
}