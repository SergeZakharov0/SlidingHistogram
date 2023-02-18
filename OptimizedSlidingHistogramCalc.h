#pragma once

#include <vector>
#include "SlidingHistogramCalc.h"


class OptimizedSlidingHistogramCalc : public SlidingHistogramCalc {
public:
	OptimizedSlidingHistogramCalc(const std::vector<double>& initialData, size_t binsNum) : SlidingHistogramCalc(initialData, binsNum) {}

protected:
	// Optimized version of the histogram recalculation
	// ------------------------------------------------------------------------------------------------
	// 1) In case of unchanged minValue and binSize: O(1) average / O(1) worst
	// 2) In case of changed minValue and equal/increased binSize: ?O(BINS_NUM)? average / O(BINS_NUM) worst
	// 3) In case of decreased binSize - unoptimized code: O(WINDOW_SIZE) average / O(WINDOW_SIZE) worst
	// ------------------------------------------------------------------------------------------------
	// When WINDOW_SIZE is big enough, the first case is the most common case of the recalculation 
	// as two numbers change does not affect dramatically the std. mean and std. deviation
	// which is used to get minValue and binSize in CalculateHistogram() 
	virtual void RecalculateHistogramInternal(double curMinValue, double curBinSize, double replacedValue, double curValue) override;

protected:
	bool isFirstCalc = true;
};