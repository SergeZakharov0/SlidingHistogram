#pragma once

#include <vector>
#include "CyclingBuffer.h"
#include "Histogram.h"


class SlidingHistogramCalc {
public:
	SlidingHistogramCalc(const std::vector<double>& initialData, size_t binsNum);

	// Updates the data window with new value and calculates a sliding histogram
	void CalculateHistogram(double newValue);

	const Histogram& GetHistogram() const { return histogram; };

protected:
	virtual void RecalculateHistogramInternal(double curMinValue, double curBinSize, double replacedValue, double curValue);

protected:
	CyclingBuffer rawData;
	double curSum = 0;
	double curSquaresSum = 0;
	Histogram histogram;
};
