#include "cnpy.h"
#include <algorithm>
#include <omp.h>
#include <chrono>
#include "merger.hpp"

using std::vector;
using std::string;

auto main(int argc, char* argv[]) -> int{
    Merger merger;
    merger.init("/data/1/zhen/criteo-tb/partition/new/day0_1m.npz");
    for(int i = 2;i < 80; i++){
        auto start = std::chrono::high_resolution_clock::now();
        merger.update("/data/1/zhen/criteo-tb/partition/new/day0_" + std::to_string(i) + "m.npz");
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << i <<"th time: " << time << std::endl;
    }
    auto result = merger.generatePartition(0.001);
    for(int i = 0; i < 10; i++){
        std::cout << result.partition[i] << " ";
        for(int j = 0; j < 4; j++){
            std::cout << result.caches[j][i] << " ";
        }
        std::cout << std::endl;
    }
    int count = 0;
    for(int i = 0; i < result.partition.size(); i++){
        if(result.partition[i] == 0){
            count++;
        }
    }
    std::cout << count << std::endl;
    cnpy::npz_save("/data/1/zhen/criteo-tb/partition/merged/day0_79m.npz","embed_partition",&partition[0],{partition.size()},"w");
    return 0; 
}