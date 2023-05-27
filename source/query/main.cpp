#include <iostream>
#include <random>
#include <sstream>
#include "CLI11.hpp"
#include "cnpy.h"
#include <hiredis/hiredis.h>

using std::vector;
using std::string;
using std::unique_ptr;

vector<int> load_partition(std::string path){
  cnpy::npz_t data = cnpy::npz_load(path);
  vector<int> partition = data["embed_partition"].as_vec<int>();
  return partition;
}

auto main(int argc, char* argv[]) -> int
{
    CLI::App app{"Redis Application"};
    std::string exec_mode;
    app.add_option("-m,--mode", exec_mode, "execution mode");
    CLI11_PARSE(app, argc, argv);
    vector<unique_ptr<redisContext>> rc_vec;
    vector<const char*> startup_nodes = {"49.52.27.23","49.52.27.25","49.52.27.26","49.52.27.27"};
    const char* password = "981025";
    for (size_t i = 0; i < 4; i++)
    {
        unique_ptr<redisContext> rc(redisConnect(startup_nodes[i], 6379));
        redisCommand(rc.get(),"AUTH %s", password);
        if (rc != NULL && rc->err) {
            printf("Error: %s\n", rc->errstr);
            return -1;
        }
        rc_vec.push_back(std::move(rc)); 
        std::cout << "node " << i << " connected" << std::endl;
    }
    if(exec_mode == "init"){
        for(auto &rc : rc_vec){
            redisCommand(rc.get(), "FLUSHALL");
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(INT32_MIN, INT32_MAX);
        // vector<int> partition = load_partition("/data/1/zhen/criteo-tb/partition/window/day0_20m.npz");
        // #pragma omp parallel for num_threads(4)
        for(int i = 0; i < 10; i++){
            if(i % 1000000 == 0) std::cout << i << std::endl;
            // if(partition[i] < 0) continue;
            std::stringstream ss;
            for (int i = 0; i < 32; ++i) {
                if (i != 0) ss << " ";
                ss << dis(gen);
            }
            std::cout << ss.str() << std::endl;
            // redisCommand(rc_vec[partition[i]].get(), "SET %d %s", i, ss.str().c_str());
        }
    }
    return 0;
}