#pragma once
#include <vector>

std::vector<size_t> jumpIndices(const std::vector<double> &data, int windowSize, double jumpScale, double stdDivScale, int minSeparation);

bool similar(double a, double b, double offset = 0.2);
