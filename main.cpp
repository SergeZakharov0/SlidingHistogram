#include <iostream>
#include <vector>
#include "Histogram.h"
#include "SlidingHistogramCalc.h"
#include "OptimizedSlidingHistogramCalc.h"


constexpr size_t WINDOW_SIZE = 1024;
constexpr size_t BINS_NUM = 64;  // For OptimizedSlidingHistogramCalc, should be a power of two. Actually can be dynamic but I did not research how that can be used


void PrintHistogram(const Histogram& histogram) {
	std::cout << "Bin Step: " << histogram.binStep << ", Min Value : " << histogram.minValue << "\n";
	for (const double& val : histogram.data) {
		std::cout << val;
		std::cout << " ";
	}
	std::cout << "\n=================================\n";
}


int main() {
	// Get initial values from the input
	std::vector<double> initialData;
	double val;
	for (int i = 0; i < WINDOW_SIZE; ++i) {
		std::cin >> val;
		initialData.push_back(val);
	}

	// Calculate and print histograms
	SlidingHistogramCalc histoCalc(initialData, BINS_NUM);
	OptimizedSlidingHistogramCalc OptihistoCalc(initialData, BINS_NUM);
	while (std::cin >> val) {
		std::cout << "Unoptimized histo calculator:\n";
		histoCalc.CalculateHistogram(val);
		PrintHistogram(histoCalc.GetHistogram());
		std::cout << "Optimized histo calculator:\n";
		OptihistoCalc.CalculateHistogram(val);
		PrintHistogram(OptihistoCalc.GetHistogram());
		std::cout << '\n';
	}

	system("pause");
	return 0;
}