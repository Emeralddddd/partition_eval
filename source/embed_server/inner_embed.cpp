#include "inner_embed.hpp"
#include <cassert>
#include <iostream>

using std::vector;

RandomInnerEmbedding::RandomInnerEmbedding(int _embed_size, int _embed_dim)
 : BaseInnerEmbedding(_embed_size,_embed_dim){
    std::random_device rd;
    random_engine_ = std::default_random_engine(rd());
    distr_ = std::uniform_real_distribution<float>(0.0,1.0);
}

void RandomInnerEmbedding::query(const std::vector<int>& input, std::vector<std::vector<float>>& output){
    int n = input.size();
    std::cout <<  embed_size_ << " " <<embed_dim_ << std::endl;
    output.resize(n, vector<float>(embed_dim_));
    #pragma omp parallel for num_threads(4)
    for(int i = 0; i < n; i++){
        assert(input[i] < embed_size_);
        for(int j = 0; j < embed_dim_; j++){
            output[i][j] = generateRandom();
        }
    }
    return;
}