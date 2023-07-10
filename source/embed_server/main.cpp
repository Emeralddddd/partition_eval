#include <iostream>
#include "embed_server.hpp"
#include "infer_server.hpp"
#include "CLI11.hpp"
#include <numeric>

using std::vector;

auto main(int argc, char** argv) -> int
{
    CLI::App app{"App description"};
    int rank = -1, embed_dim = 32;
    app.add_option("-r,--rank", rank, "rank number");
    app.add_option("-d,--dim", embed_dim, "embed dim");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    std::string server_address("0.0.0.0:50051");
    vector<std::string> server_address_list= {"49.52.27.23:50051","49.52.27.25:50051","49.52.27.26:50051","49.52.27.27:50051"};
    EmbedServerImpl embedService(embed_dim);
    std::cout << "embedService dim " << embed_dim << std::endl; 
    InferServerSyncImpl inferService(4, rank, server_address_list);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&embedService);
    builder.RegisterService(&inferService);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server " << rank << " listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}