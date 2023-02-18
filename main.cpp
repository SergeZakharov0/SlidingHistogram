#include <iostream>
#include <vector>
#include <set>

using namespace std;


constexpr size_t WINDOW_SIZE = 1024;
constexpr size_t BINS_NUM = 64;  // Should be a power of two. Actually can be dynamic but I did not research how that can be used


class CyclingBuffer {
public:
	CyclingBuffer(const vector<float>& data) : data(data) {};

	float Place(float newVal) {
		float replacedVal = data[replaceIndex];
		data[replaceIndex] = newVal;
		++replaceIndex;
		if (replaceIndex >= data.size()) {
			replaceIndex = 0;
		}
		return replacedVal;
	}

	const vector<float>& GetData() const { return data; }

private:
	vector<float> data;
	size_t replaceIndex = 0;
};



struct Histogram {
	Histogram(size_t binsNum) 
		: data(binsNum, 0) {};

	void AddToBin(float value, int binNum) {
		if (binNum < 0) {
			binNum = 0;
		}
		if (binNum >= static_cast<int>(data.size())) {
			binNum = data.size() - 1;
		}
		data[binNum] += value;
	}

	vector<float> data;
	float binStep = 0;  
	float minValue = 0;
};


class SlidingHistogramCalc {
public:
	SlidingHistogramCalc(const vector<float>& initialData, size_t binsNum)
		: rawData(initialData)
	    , histogram(binsNum) {
		for (const float& curVal : rawData.GetData()) {
			curSum += curVal;
			curSquaresSum += curVal * curVal;
		}
	}

	// Updates the data window with new value and calculates a sliding histogram
	void CalculateHistogram(float newValue);

	const Histogram& GetHistogram() const { return histogram; };

protected:
	virtual void RecalculateHistogramInternal(float curMinValue, float curBinSize, float replacedValue, float curValue);

protected:
	CyclingBuffer rawData;
	float curSum = 0;
	float curSquaresSum = 0;
	Histogram histogram;
};


class OptimizedSlidingHistogramCalc : public SlidingHistogramCalc {
public:
	OptimizedSlidingHistogramCalc(const vector<float>& initialData, size_t binsNum)
		: SlidingHistogramCalc(initialData, binsNum) {}

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
	virtual void RecalculateHistogramInternal(float curMinValue, float curBinSize, float replacedValue, float curValue) override;

protected:
	bool isFirstCalc = true;
};


void SlidingHistogramCalc::CalculateHistogram(float newValue) {
	float replacedValue = rawData.Place(newValue);

	// Update sums
	curSum -= replacedValue;
	curSum += newValue;
	curSquaresSum -= replacedValue * replacedValue;
	curSquaresSum += newValue * newValue;

	// Get actual mean and deviation
	const float actualMean = curSum / rawData.GetData().size();
	const float actualDeviation = sqrt(curSquaresSum / rawData.GetData().size() - actualMean * actualMean);

	// Get step and min
	// Just intuitively try to follow three sigma rule (two sigma with ceil rounding on top)
	const float actualBinSize = actualDeviation * 4 / histogram.data.size();
	const float roundedBinSize = pow(2, ceil(log(actualBinSize) / log(2)));
	const float roundedMean = ceil(actualMean / roundedBinSize) * roundedBinSize;
	const float roundedMinValue = roundedMean - roundedBinSize * histogram.data.size() / 2;

	RecalculateHistogramInternal(roundedMinValue, roundedBinSize, replacedValue, newValue);
}


void SlidingHistogramCalc::RecalculateHistogramInternal(float curMinValue, float curBinSize, float replacedValue, float curValue) {
	// Just recalculate full histogram. O(WINDOW_SIZE) which can be huge
	fill(histogram.data.begin(), histogram.data.end(), 0.f);
	for (const float& curValue : rawData.GetData()) {
		histogram.AddToBin(1, static_cast<int>((curValue - curMinValue) / curBinSize));
	}
	histogram.minValue = curMinValue;
	histogram.binStep = curBinSize;
}


void OptimizedSlidingHistogramCalc::RecalculateHistogramInternal(float curMinValue, float curBinSize, float replacedValue, float curValue) {
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
			float curSumValue;
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


void PrintHistogram(const Histogram& histogram) {
	cout << "Bin Step: " << histogram.binStep << ", Min Value : " << histogram.minValue << "\n";
	for (const float& val : histogram.data) {
		cout << val;
		cout << " ";
	}
	cout << "\n=================================\n";
}


int main() {
	// Get initial values from the input
	vector<float> initialData;
	float val;
	for (int i = 0; i < WINDOW_SIZE; ++i) {
		cin >> val;
		initialData.push_back(val);
	}

	// Calculate and print histograms
	SlidingHistogramCalc histoCalc(initialData, BINS_NUM);
	OptimizedSlidingHistogramCalc OptihistoCalc(initialData, BINS_NUM);
	while (cin >> val) {
		cout << "Unoptimized histo calculator:\n";
		histoCalc.CalculateHistogram(val);
		PrintHistogram(histoCalc.GetHistogram());
		cout << "Optimized histo calculator:\n";
		OptihistoCalc.CalculateHistogram(val);
		PrintHistogram(OptihistoCalc.GetHistogram());
		cout << '\n';
	}

	system("pause");
	return 0;
}