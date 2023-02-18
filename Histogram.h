#pragma once

#include <vector>


struct Histogram {
public:
	Histogram(size_t binsNum) : data(binsNum, 0) {};

	void AddToBin(double value, int binNum);

public:
	std::vector<double> data;
	double binStep = 0;
	double minValue = 0;
};