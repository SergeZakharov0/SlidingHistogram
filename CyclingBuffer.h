#pragma once

#include <vector>


class CyclingBuffer {
public:
	CyclingBuffer(const std::vector<double>& data) : data(data) {};

	const std::vector<double>& GetData() const { return data; }

	// Places the newVal onto the cycling buffer and returns the replaced value
	double Place(double newVal);

private:
	std::vector<double> data;
	size_t replaceIndex = 0;
};
