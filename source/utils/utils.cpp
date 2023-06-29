#include "utils.hpp"
#include <chrono>

using std::vector;

void load_data(std::string path, vector<vector<int>> &data){
  auto start = std::chrono::high_resolution_clock::now();
  cnpy::NpyArray ndata = cnpy::npy_load(path);
  auto mid = std::chrono::high_resolution_clock::now();
  data.resize(ndata.shape[0],vector<int>(ndata.shape[1],-1));
  #pragma omp parallel for num_threads(16)
  for(int i = 0; i < ndata.shape[0]; i++){
    for(int j = 0; j < ndata.shape[1]; j++){
      data[i][j] = ndata.data<long long>()[j*ndata.shape[0] + i];
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
  auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();
  std::cout<<"load time: " << time1 << " copy time: "<< time2 << std::endl;
}

vector<int> getCurrentInput(const vector<vector<int>> &data, int bs, int fs, int i){
    vector<int> currentInput;
    currentInput.reserve(bs * fs);
    for (int j = 0; j < bs; j++)
    { 
      for(auto & x : data[i*bs + j]) {
        currentInput.push_back(x);
      }
    }
    return currentInput;
}