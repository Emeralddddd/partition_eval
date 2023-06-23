#include "utils.hpp"

void load_data(std::string path, std::vector<std::vector<int>> &data){
  cnpy::NpyArray ndata = cnpy::npy_load(path);
  data.resize(ndata.shape[0], std::vector<int>(ndata.shape[1],-1));
  #pragma omp parallel for num_threads(16)
  for(int i = 0; i < ndata.shape[0]; i++){
    for(int j = 0; j < ndata.shape[1]; j++){
      data[i][j] = ndata.data<long long>()[j*ndata.shape[0] + i];
    }
  }
}