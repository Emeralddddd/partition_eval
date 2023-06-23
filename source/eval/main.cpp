#include <iostream>
#include <string>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <fstream>

#include "partition.hpp"
#include "eval.hpp"
#include "cnpy.h"
#include "../utils/utils.hpp"

using std::vector;
using std::unordered_set;

const static std::string CRITEO_PATH = "/home/xuzhizhen/datasets/criteo-tb/";

auto main() -> int
{
  const std::string filename = "results/cost.csv";
  std::ofstream outfile(filename.c_str());
  if(!outfile.is_open()){
    outfile.open(filename.c_str(), std::ios::out);
    if (!outfile) {
        std::cout << "Error: failed to create file " << filename << std::endl;
        return 0;
    }
  }
  constexpr double HOT_RATE = 0.001;
  constexpr int PART_NUMS = 4;
  std::vector<std::vector<int>> data;
  load_data(CRITEO_PATH + "sparse_day_0.npy", data);
  std::cout << "data loaded" << std::endl;
  StaticPartition partitionHET(PART_NUMS,CRITEO_PATH + "partition/full/day0_1m.npz",HOT_RATE);
  StaticPartition partitionMerge(PART_NUMS,CRITEO_PATH + "partition/full_5/day0_1m.npz",HOT_RATE);
  Merger merger(PART_NUMS);
  vector<double> cost;
  vector<double> mergeCost;
  std::cout << "partition test start" << std::endl;
  auto start_time = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::now();
  std::unordered_set<int> ban_points = {22,23,24,26,27,28,31,32,33,34,36,39,42,43,51,56};
  for(int i = 20000000; i < 80000000; i++){
    if(i % 1000000 == 0){
      auto t0 = std::chrono::system_clock::now();
      merger.update(CRITEO_PATH + "partition/new_5/day0_" + std::to_string(i/1000000) + "m.bin");
      partitionMerge.load_partition_from_merger(merger.generatePartition(HOT_RATE));
      auto t1 = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_time = t1 - t0;
      std::cout << "Merge time : " << elapsed_time.count();
      partitionHET.load_partition_from_npz(CRITEO_PATH + "partition/full_5/day0_"+ std::to_string(i/1000000) +"m.npz",HOT_RATE);
    } 
    int bs = 1;
    int fieldSize = 26;
    vector<int> currentInput = getCurrentInput(data, bs, fieldSize, i);
    partitionHET.processRequest(currentInput);
    partitionMerge.processRequest(currentInput);
    if(i > 0 && i%100000 == 0){
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_time = now - t;
      std::cout << i << ": " << elapsed_time.count() << " -------------------------------------------"<<std::endl;
      std::cout << "HET Partition :" << std::endl;
      std::cout << "node access: " << partitionHET.getRemoteAccess() << " local access rate: " << partitionHET.getLocalRate()  << " cache hit rate: " \
      << partitionHET.getCacheHitRate() << " avg cost: " << partitionHET.getAvgCost() << std::endl;
      std::cout << "Merge Partition :" << std::endl;
      std::cout << "node access: " << partitionMerge.getRemoteAccess() << " local access rate: " << partitionMerge.getLocalRate()  << " cache hit rate: " \
      << partitionMerge.getCacheHitRate() << " avg cost: " << partitionMerge.getAvgCost() << " embedding change: " << partitionMerge.getChangedEmbed() << std::endl;
      outfile << i;
      outfile << "," << partitionHET.getAvgCost();
      outfile << ", " << partitionMerge.getAvgCost() << "," << partitionMerge.getChangedEmbed();
      outfile << std::endl;
      cost.push_back(partitionHET.getAvgCost());
      mergeCost.push_back(partitionMerge.getAvgCost());
      partitionHET.clear();
      partitionMerge.clear();
      t = now;
    }
  }
  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = now - start_time;
  std::cout << "partition test time: " << elapsed_time.count() << std::endl;
  for(int i = 0; i < PART_NUMS; i++){
    std::cout <<" partition " << i << " Het partition "<< partitionHET.getPartitionCnt(i)\
    << " Het acccess count " << partitionHET.getAccessCnt(i) << std::endl;
      std::cout <<" partition " << i << " Merge partition "<< partitionMerge.getPartitionCnt(i)\
    << " Merge acccess count " << partitionMerge.getAccessCnt(i) << std::endl;
  }
  std::cout << "HET avg cost: " << std::accumulate(cost.begin(), cost.end(), 0.0) / cost.size() << std::endl;
  std::cout << "Merge avg cost: " << std::accumulate(mergeCost.begin(), mergeCost.end(), 0.0) / mergeCost.size() << std::endl;
  return 0;
}