#include "CyclingBuffer.h"


double CyclingBuffer::Place(double newVal) {
	double replacedVal = data[replaceIndex];
	data[replaceIndex] = newVal;
	++replaceIndex;
	if (replaceIndex >= data.size()) {
		replaceIndex = 0;
	}
	return replacedVal;
}