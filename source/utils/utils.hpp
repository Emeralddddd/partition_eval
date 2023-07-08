#include "cnpy.h"

void load_data(std::string path, std::vector<std::vector<int>> &data);

std::vector<int> getCurrentInput(const std::vector<std::vector<int>> &data, int bs, int fs, int i);

double calculateAverage(const std::vector<int>& numbers);

double calculatePercentile(const std::vector<int>& numbers, double percentile);