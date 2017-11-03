#ifndef COMPUTATIONS_H
#define COMPUTATIONS_H

#include "common.h"

#include <cmath>
#include <algorithm>

// The functions to compute the geometric mean and the median are moved into this separate file to make it easier to test them for correctness in a separate program

double calculate_median(vector<int>& number_buffer) {
	if (number_buffer.size() == 0)
		return 0.0;
	if (number_buffer.size() == 1)
		return static_cast<double>(number_buffer.at(0));
	
	std::sort(number_buffer.begin(), number_buffer.end());

	double median = 0.0;
	if (number_buffer.size() % 2 != 0) {
		// For an odd number of numbers, the median is right in the middle of the sorted numbers
		median = static_cast<double>(number_buffer.at(number_buffer.size() / 2));
	} else {

		// For an even number, the median is the arithmetic mean of the two numbers in the middle of the
		// number array
		double half_left = static_cast<double>(number_buffer.at((number_buffer.size() / 2) - 1));
		double half_right = static_cast<double>(number_buffer.at(number_buffer.size() / 2 ));
		
		median = 0.5 * ( half_left + half_right);
	}
	return median;
}

double calculate_geometric_mean(struct SharedMemory& shared_memory, const int count) {
	// We sum up logaritms of numbers to avoid overflow of the product calculated in the geometric mean
	double sum_log = 0.0;
	for (int i = 1; i < count; i++) {
		sum_log += std::log(static_cast<double>(shared_memory.buffer[i]));
	}
	return std::exp(sum_log / (count - 1));
}

#endif
