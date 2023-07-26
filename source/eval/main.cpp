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
constexpr static int start = 20000000, end = 180000000;

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
  constexpr double HOT_RATE = 0.0005;
  constexpr int PART_NUMS = 4;
  std::vector<std::vector<int>> data;
  load_data(CRITEO_PATH + "sparse_day_0.npy", data);
  std::cout << "data loaded" << std::endl;
  StaticPartition partitionHET(PART_NUMS);
  StaticPartition partitionMerge(PART_NUMS);
  partitionMerge.load_partition_from_npz(CRITEO_PATH + "partition/window_10/day0_"+ "20" +"m.npz",HOT_RATE);
  partitionHET.load_partition_from_npz(CRITEO_PATH + "partition/window_10/day0_"+ "20" +"m.npz",HOT_RATE);
  Merger merger(PART_NUMS);
  // for(int i = 10; i<20; i++){
  //   merger.update(CRITEO_PATH + "partition/new_10/day0_" + std::to_string(i) + "m.bin");
  // }
  vector<double> cost;
  vector<double> mergeCost;
  std::cout << "partition test start" << std::endl;
  std::unordered_set<int> ban_points = {};
  for(int i = start; i <= end; i++){
    if(i % 1000000 == 0){
      merger.update(CRITEO_PATH + "partition/new_window_10/day0_" + std::to_string(i/1000000) + "m.bin");
      partitionMerge.load_partition_from_merger(merger.generatePartition(HOT_RATE));
      if(!ban_points.count(i / 1000000)){
        partitionHET.load_partition_from_npz(CRITEO_PATH + "partition/window_10/day0_"+ std::to_string(i/1000000) +"m.npz",HOT_RATE);
      }
    }
    if(i > start && i % 100000 == 0){
      std::cout << i << " :----------------------------------"<< std::endl;
      std::cout << "HET Partition :" << std::endl;
      std::cout << "node access: " << partitionHET.getRemoteAccess() << " local access rate: " << partitionHET.getLocalRate()  << " cache hit rate: " \
      << partitionHET.getCacheHitRate() << " avg cost: " << partitionHET.getAvgCost() << std::endl;
      std::cout << "Merge Partition :" << std::endl;
      std::cout << "node access: " << partitionMerge.getRemoteAccess() << " local access rate: " << partitionMerge.getLocalRate()  << " cache hit rate: " \
      << partitionMerge.getCacheHitRate() << " avg cost: " << partitionMerge.getAvgCost() << " embedding change: " << partitionMerge.getChangedEmbed() << std::endl;
      outfile << i;
      outfile << "," << partitionHET.getAvgCost() << "," << partitionHET.getRemoteAccess();
      outfile << ", " << partitionMerge.getAvgCost() << "," << partitionMerge.getRemoteAccess();
      outfile << std::endl;
      cost.push_back(partitionHET.getAvgCost());
      mergeCost.push_back(partitionMerge.getAvgCost());
      partitionHET.clear();
      partitionMerge.clear();
    }
    // if(i % 10000000 == 0 && i > 20000000) {
    //   partitionMerge.load_partition_from_npz(CRITEO_PATH + "partition/window_10/day0_"+ std::to_string(i/1000000 - 10) +"m.npz",HOT_RATE);
    // }
    int bs = 1;
    int fieldSize = 26;
    vector<int> currentInput = getCurrentInput(data, bs, fieldSize, i);
    partitionHET.processRequest(currentInput);

    partitionMerge.processRequest(currentInput);
  }
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