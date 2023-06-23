#include "cnpy.h"

void load_data(std::string path, std::vector<std::vector<int>> &data);

std::vector<int> getCurrentInput(std::vector<std::vector<int>> &data, int bs, int fs, int i);