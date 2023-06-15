#include <iostream>
#include <string>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <fstream>

#include "lib.hpp"
#include "partition.hpp"
#include "eval.hpp"
#include "cnpy.h"

using std::vector;
using std::unordered_set;

int load_data(std::string path, std::vector<std::vector<int>> &data){
  cnpy::NpyArray ndata = cnpy::npy_load(path);
  data.resize(ndata.shape[0], std::vector<int>(ndata.shape[1],-1));
  for(int i = 0; i < ndata.shape[0]; i++){
    for(int j = 0; j < ndata.shape[1]; j++){
      data[i][j] = ndata.data<long long>()[j*ndata.shape[0] + i];
    }
  }
  return 0;
}

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
  const int records_per_day = 20000000;
  std::vector<std::vector<int>> data;
  load_data("/data/1/zhen/criteo-tb/sparse_day_0.npy", data);
  std::cout << "data loaded" << std::endl;
  StaticPartition partitionHET(4,"/data/1/zhen/criteo-tb/partition/window/day0_195m.npz",0.001);
  StaticPartition partitionMerge(4,"/data/1/zhen/criteo-tb/partition/window/day0_195m.npz",0.001);
  Merger merger(4);
  vector<double> cost;
  vector<double> mergeCost;
  std::cout << "partition test start" << std::endl;
  auto start_time = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::now();
  std::unordered_set<int> ban_points = {22,23,24,26,27,28,31,32,33,34,36,39,42,43,51,56};
  for(int i = 20000001; i < 80000000; i++){
    if(i % 1000000 == 0 && !ban_points.count(i/1000000)){
      merger.update("/data/1/zhen/criteo-tb/partition/new/day0_" + std::to_string(i/1000000) + "m.bin");
      partitionMerge.load_partition_from_merger(merger.generatePartition(0.001));
      partitionHET.load_partition_from_npz("/data/1/zhen/criteo-tb/partition/full/day0_"+ std::to_string(i/1000000) +"m.npz",0.001);
    } 
    int bs = 1;
    int fieldSize = 26;
    vector<int> currentInput;
    currentInput.reserve(bs * fieldSize);
    for (int j = 0; j < bs; j++)
    {
      for(auto & x : data[i*bs + j]) {
        currentInput.push_back(x);
      }
    }
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
  for(int i = 0; i < 4; i++){
    std::cout <<" partition " << i << " Het partition "<< partitionHET.getPartitionCnt(i)\
    << " Het acccess count " << partitionHET.getAccessCnt(i) << std::endl;
  }
  std::cout << "HET avg cost: " << std::accumulate(cost.begin(), cost.end(), 0.0) / cost.size() << std::endl;
  std::cout << "Merge avg cost: " << std::accumulate(mergeCost.begin(), mergeCost.end(), 0.0) / mergeCost.size() << std::endl;
  return 0;
}