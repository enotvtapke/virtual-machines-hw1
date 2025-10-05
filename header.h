#pragma once
#include <vector>

std::vector<size_t> jumpIndices(const std::vector<double> &data, int windowSize, double jumpScale, double stdDivScale, int minSeparation);

