#include <iostream>
#include "inner_embed.hpp"
#include <numeric>

using std::vector;

auto main() -> int
{
    RandomInnerEmbedding re(100000, 4);
    vector<int> input(10);
    vector<vector<float>> output;
    for(int i = 0; i < 5; i++){
        std::iota(input.begin(), input.end(), i*10);
        re.query(input, output);
        std::cout << output.size() << " " << output[0].size() << std::endl;
        for(auto x : output){
            for(auto f : x){
                std::cout << f;
            }
            std::cout << std::endl;
        }
        std::cout << "----------------------------------" << std::endl;
    }
    return 0;
}