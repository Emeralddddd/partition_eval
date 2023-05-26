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
  load_data("/data/1/zhen/criteo-tb/sparse_day_1.npy", data);
  // load_data("/data/1/zhen/dac/sparse_feats.npy", data);
  // vector<vector<int>> day0(data.begin(), data.begin() + records_per_day);
  // vector<vector<int>> day012(std::make_move_iterator(data.begin()), std::make_move_iterator(data.begin() + records_per_day * 3));
  // vector<vector<int>> day3(std::make_move_iterator(data.begin() + records_per_day * 3), std::make_move_iterator(data.end()));
  std::cout << "data loaded" << std::endl;
  ScorePartition partition(4);
  // partition.load_partition("/data/1/zhen/dac/partition/10m.npz");
  // partition.load_partition("/data/1/zhen/criteo-tb/partition/day_0_20m.npz", "scores.csv",0.00);
  StaticPartition partitionHET(4,"/data/1/zhen/criteo-tb/partition/window/day0_195m.npz",0.001);
  // partition.load_partition("/data/1/zhen/criteo-tb/partition/day_0_50m.npz",data,0.001);
  // partitionHET.load_query_partition("/data/1/zhen/dac/partition.npz");
  std::cout << "partition test start" << std::endl;
  auto start_time = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::now();
  // std::unordered_set<int> ban_points = {22,23,24,26,27,28,31,32,33,34,36,39,42,43,51,56};
  std::unordered_set<int> ban_points = {};
  for(int i = 20000001; i < 80000000; i++){
    // if(i % 1000000 == 0 && !ban_points.count(i/1000000)){
    //   partitionHET.load_partition("/data/1/zhen/criteo-tb/partition/window/day0_"+ std::to_string(i/1000000) +"m.npz",0.001);
    // } 
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
    // partition.processRequest(currentInput);
    if(i > 0 && i%100000 == 0){
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_time = now - t;
      std::cout << i << ": " << elapsed_time.count() << " -------------------------------------------"<<std::endl;
      std::cout << "Static Partition :" << std::endl;
      std::cout << "node access: " << partitionHET.getRemoteAccess() << " local access rate: " << partitionHET.getLocalRate()  << " cache hit rate: " \
      << partitionHET.getCacheHitRate() << " avg cost: " << partitionHET.getAvgCost() << std::endl;
      std::cout << "Dynamic Partition :" << std::endl;
      std::cout << "node access: " << partition.getRemoteAccess() << " local access rate: " << partition.getLocalRate()  << " cache hit rate: " \
      << partition.getCacheHitRate() << " avg cost: " << partition.getAvgCost() << " embedding change: " << partition.getChangedEmbed() << std::endl;
      outfile << i;
      outfile << "," << partitionHET.getAvgCost();
      outfile << ", " << partition.getAvgCost() << "," << partition.getChangedEmbed();
      outfile << std::endl;
      partition.clear();
      partitionHET.clear();
      t = now;
    }
    // if(i > 0 && i % 10000000 == 0){
    //   std::cout << "reset partition" << std::endl;
    //   partition.resetPartition();
    // } 
  }
  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = now - start_time;
  std::cout << "partition test time: " << elapsed_time.count() << std::endl;
  for(int i = 0; i < 4; i++){
    std::cout <<" partition " << i << " Het partition "<< partitionHET.getPartitionCnt(i)\
    << " Het acccess count " << partitionHET.getAccessCnt(i) << std::endl;
  }
  // outfile.close();
  return 0;
}