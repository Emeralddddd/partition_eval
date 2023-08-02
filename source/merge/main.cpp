#include "cnpy.h"
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <fstream>
#include "merger.hpp"


using std::vector;
using std::string;

const static std::string CRITEO_PATH = "/home/xuzhizhen/datasets/criteo-tb/";

void diff(){
    Merger merger(4);
    PartitionResult p1, p2;
    merger.update(CRITEO_PATH + "partition/new_window_1/day0_20m.bin");
    p1 = merger.generatePartition(0.001);
    for(int k = 21; k <= 180; k++){
        merger.update(CRITEO_PATH + "partition/new_window_1/day0_" + std::to_string(k) + "m.bin");
        p2 = merger.generatePartition(0.001);
        int n = p1.partition.size();
        int m = p2.partition.size();
        int count = 0;
        for(int i = 0; i < n; i++){
            if(p1.partition[i] != p2.partition[i]) count++;
        }
        std::cout << k << " " << count << std::endl;
        p1 = std::move(p2);
    }
}

auto main(int argc, char* argv[]) -> int{
    diff();
    return 0; 
}