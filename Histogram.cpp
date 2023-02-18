#include "Histogram.h"


void Histogram::AddToBin(double value, int binNum) {
	if (binNum < 0) {
		binNum = 0;
	}
	if (binNum >= static_cast<int>(data.size())) {
		binNum = data.size() - 1;
	}
	data[binNum] += value;
}