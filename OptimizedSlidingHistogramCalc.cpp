#include "OptimizedSlidingHistogramCalc.h"


void OptimizedSlidingHistogramCalc::RecalculateHistogramInternal(double curMinValue, double curBinSize, double replacedValue, double curValue) {
	if (isFirstCalc) {
		SlidingHistogramCalc::RecalculateHistogramInternal(curMinValue, curBinSize, replacedValue, curValue);
		isFirstCalc = false;
		return;
	}

	// Simple case - same starting value and bin size. O(1) worst
	if (curMinValue == histogram.minValue && curBinSize == histogram.binStep) {

		histogram.AddToBin(1, static_cast<int>((curValue - curMinValue) / curBinSize));
		histogram.AddToBin(-1, static_cast<int>((replacedValue - curMinValue) / curBinSize));
		return;
	}

	// Case of >= bin size + probably moved minValue
	if (curBinSize >= histogram.binStep) {
		histogram.AddToBin(-1, static_cast<int>((replacedValue - histogram.minValue) / histogram.binStep));  // Remove old value

		/* First of all, equalize the bin size. O(BINS_NUM) */
		if (curBinSize > histogram.binStep) {
			int binsToMerge = static_cast<int>(curBinSize / histogram.binStep);  // As binSize is always power of two, it is an integer
			double curSumValue;
			for (int i = 0; i < histogram.data.size() / binsToMerge; i += 1) {
				curSumValue = 0;
				for (int j = 0; j < binsToMerge && i * binsToMerge + j < histogram.data.size(); ++j) {
					curSumValue += histogram.data[i * binsToMerge + j];
				}
				histogram.data[i] = curSumValue;
			}
			fill(histogram.data.begin() + histogram.data.size() / binsToMerge, histogram.data.end(), 0.f);
			histogram.binStep = curBinSize;
		}

		/* Then move minValue if necessary*/
		if (curMinValue == histogram.minValue) {
			return;
		}
		const int posDelta = static_cast<int>((curMinValue - histogram.minValue) / histogram.binStep);
		if (posDelta > static_cast<int>(histogram.data.size()) || posDelta < -static_cast<int>(histogram.data.size())) {
			// Moving away from all the previous bins = zero all values
			fill(histogram.data.begin(), histogram.data.end(), 0.f);
		}
		else if (posDelta < 0) {
			// Moving all values to the right. O(BINS_NUM) worst but in most cases should be optimized by the compiler
			copy(histogram.data.begin(), histogram.data.end() + posDelta, histogram.data.begin() - posDelta);  // memcpy
			fill(histogram.data.begin(), histogram.data.begin() - posDelta, 0.f);  // memset
		}
		else if (posDelta > 0) {
			// Moving all values to the left. O(BINS_NUM) worst but in most cases should be optimized by the compiler
			copy(histogram.data.begin() + posDelta, histogram.data.end(), histogram.data.begin());  // memcpy
			fill(histogram.data.end() - posDelta, histogram.data.end(), 0.f);  // memset
		}
		histogram.minValue = curMinValue;
		histogram.AddToBin(1, static_cast<int>((curValue - histogram.minValue) / histogram.binStep));
		return;
	}

	// Other cases. Unoptimized
	SlidingHistogramCalc::RecalculateHistogramInternal(curMinValue, curBinSize, replacedValue, curValue);
}