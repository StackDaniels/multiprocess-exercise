#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>

#define TAG "[childA] "
#include "common.h"

#include "computations.h"

const char* pipe_name = "/tmp/pipeToChildA";

inline int str_to_int(const string& str);

double calculate_median(vector<int>& number_buffer); 

int main() {
	size_t num_rnd_numbers = 1;
	
	vector<int> number_buffer;
	do {
		int fd = open(pipe_name, O_RDONLY);

		// Read, how many random numbers there are:
		ssize_t bytes_read = read(fd, &num_rnd_numbers, sizeof(size_t));
		if (bytes_read == -1) {
			perror("Reading num_rnd_numbers has failed");
			break;
		} else if (num_rnd_numbers == 0) {
			log("No new data. Child A will exit now");
		} else {
			log("Will read " + std::to_string(num_rnd_numbers) + " numbers from pipe");
		}

		number_buffer.reserve(num_rnd_numbers);

		log(string(TAG) + " Random numbers received from pipe: ", NO_NEWLINE);
		for (int i=0; i<num_rnd_numbers; i++) {
			int tmp;
			bytes_read = read(fd, &tmp, sizeof(int));
			if (bytes_read == -1) {
				perror("Reading num_rnd_numbers has failed");
				break;
			} else {
				number_buffer.push_back(tmp);
			}
			log(std::to_string(number_buffer.back()) + ", ", NO_NEWLINE);
		}
		log(ONLY_NEWLINE);

		double median = calculate_median(number_buffer);
		log("Median: " + std::to_string(median));

		close(fd);
		number_buffer.clear();

	} while (num_rnd_numbers > 1);

	log("Child process exits");
	return 0;
}


inline int str_to_int(const string& str) {
	int result;
	std::stringstream(str) >> result;
	return result;
}

