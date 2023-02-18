#include "SlidingHistogramCalc.h"


SlidingHistogramCalc::SlidingHistogramCalc(const std::vector<double>& initialData, size_t binsNum)
	: rawData(initialData)
	, histogram(binsNum) {
	for (const double& curVal : rawData.GetData()) {
		curSum += curVal;
		curSquaresSum += curVal * curVal;
	}
}


void SlidingHistogramCalc::CalculateHistogram(double newValue) {
	double replacedValue = rawData.Place(newValue);

	// Update sums
	curSum -= replacedValue;
	curSum += newValue;
	curSquaresSum -= replacedValue * replacedValue;
	curSquaresSum += newValue * newValue;

	// Get actual mean and deviation
	const double actualMean = curSum / rawData.GetData().size();
	const double actualDeviation = sqrt(curSquaresSum / static_cast<double>(rawData.GetData().size()) - actualMean * actualMean);

	// Get step and min
	// Just intuitively try to follow three sigma rule (two sigma with ceil rounding on top)
	const double actualBinSize = actualDeviation * 4 / histogram.data.size();
	const double roundedBinSize = pow(2, ceil(log(actualBinSize) / log(2)));
	const double roundedMean = ceil(actualMean / roundedBinSize) * roundedBinSize;
	const double roundedMinValue = roundedMean - roundedBinSize * histogram.data.size() / 2;

	RecalculateHistogramInternal(roundedMinValue, roundedBinSize, replacedValue, newValue);
}


void SlidingHistogramCalc::RecalculateHistogramInternal(double curMinValue, double curBinSize, double replacedValue, double curValue) {
	// Just recalculate full histogram. O(WINDOW_SIZE) which can be huge
	fill(histogram.data.begin(), histogram.data.end(), 0.f);
	for (const double& curValue : rawData.GetData()) {
		histogram.AddToBin(1, static_cast<int>((curValue - curMinValue) / curBinSize));
	}
	histogram.minValue = curMinValue;
	histogram.binStep = curBinSize;
}
