#include "computations.h"

// Ghetto unit test program to verify correct implementation of median and geometric mean calculation functions

enum TestResult { OK, FAIL };

// Precision epsilon for comparing floating point numbers
constexpr double EPSILON = 0.000001;

TestResult test_median();
TestResult test_median_with(vector<int>& numbers, const double expected);

TestResult test_geometric_mean();
TestResult test_geometric_mean_with(vector<int>& numbers, const double expected); 

int main() {
	log("Ghetto unit test for median and geometric mean implementation started ... ");
	auto median_result = test_median();
	if (median_result == FAIL) {
		loge("test_median() has failed");
		return 1;
	}
	log("test_median(): OK");

	auto geom_mean_result = test_geometric_mean();
	if (geom_mean_result == FAIL) {
		loge("test_geometric_mean() has failed");
		return 2;
	}
	log("test_geometric_mean(): OK");

	log("Test successful. Exiting");
	return 0;
}

TestResult test_median() {
	auto result = FAIL;

	vector<int> first_numbers { 82, 52, 71, 84, 64, 72, 50, 75, 87, 77, 74, 83, 54, 89, 66, 83 };
	const double expected_first = 74.5;

	auto result1 = test_median_with(first_numbers, expected_first);

	vector<int> second_numbers { 90, 98, 94, 78, 74, 80, 50, 63,85 };
	const double expected_second = 80.0;
	auto result2 = test_median_with(second_numbers, expected_second);

	vector<int> third_numbers { 62, 52, 85, 71 };
	const double expected_third = 66.5;
	auto result3 = test_median_with(third_numbers, expected_third);

	if (result1 == OK && result2 == OK && result3 == OK)
		result = OK;
	return result;
}

TestResult test_median_with(vector<int>& numbers, const double expected) {
	const double actual = calculate_median(numbers);
	if (std::abs(actual - expected) > EPSILON) {
		loge("In test_median(): for first_numbers: actual median: " + std::to_string(actual) + " is not as expected: " + std::to_string(expected));
		return FAIL;
	} else {
		return OK;
	}
}

TestResult test_geometric_mean() {
	auto result = FAIL;

	// The geometric_mean function has an implementation-specific detail in which it skips the first
	// element of the numbers given to it, because in shared memory, we store the number of input elements
	// there
	vector<int> first_numbers { 16, 82, 52, 71, 84, 64, 72, 50, 75, 87, 77, 74, 83, 54, 89, 66, 83 };
	const double expected_first = 71.589859;
	auto result1 = test_geometric_mean_with(first_numbers, expected_first);

	vector<int> second_numbers { 9, 90, 98, 94, 78, 74, 80, 50, 63,85 };
	const double expected_second = 77.639453;
	auto result2 = test_geometric_mean_with(second_numbers, expected_second);

	vector<int> third_numbers { 4, 62, 52, 85, 71 };
	const double expected_third = 66.415291;
	auto result3 = test_geometric_mean_with(third_numbers, expected_third);

	if (result1 == OK && result2 == OK && result3 == OK)
		result = OK;
	return result;
}

TestResult test_geometric_mean_with(vector<int>& numbers, const double expected) {
	struct SharedMemory numbers_shm;
	numbers_shm.buffer = numbers.data();
	numbers_shm.size = static_cast<int>(numbers.size());
	const double actual = calculate_geometric_mean(numbers_shm, numbers_shm.size);
	if (std::abs(actual - expected) > EPSILON ) {
		loge("In test_geometric_mean(): for numbers: actual geometric mean: " + std::to_string(actual) + " is not as expected: " + std::to_string(expected) + " Difference is: " + std::to_string(std::abs(actual - expected)));
		return FAIL;
	} else {
		return OK;
	}
}

