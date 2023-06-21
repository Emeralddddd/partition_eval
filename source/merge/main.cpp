#include "cnpy.h"
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <fstream>
#include "merger.hpp"


using std::vector;
using std::string;

const static std::string CRITEO_PATH = "/home/xuzhizhen/datasets/criteo-tb/";

auto main(int argc, char* argv[]) -> int{
    Merger merger(4);
    for(int i = 1;i <= 80; i++){
        auto start = std::chrono::high_resolution_clock::now();
        merger.update(CRITEO_PATH + "partition/new_5/day0_" + std::to_string(i) + "m.bin");
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << i <<"th time: " << time << std::endl;
    }
    auto result = merger.generatePartition(0.001);
    merger.savePartitionToNpz(result, CRITEO_PATH + "partition/merged/day0_80m.npz");
    return 0; 
}