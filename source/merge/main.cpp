#include "cnpy.h"
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <fstream>
#include "merger.hpp"


using std::vector;
using std::string;

auto main(int argc, char* argv[]) -> int{
    Merger merger(4);
    for(int i = 1;i <= 1; i++){
        auto start = std::chrono::high_resolution_clock::now();
        merger.update("/data/1/zhen/criteo-tb/partition/new/day0_" + std::to_string(i) + "m.bin");
        auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << i <<"th time: " << time << std::endl;
    }
    auto result = merger.generatePartition(0.001);
    return 0; 
}