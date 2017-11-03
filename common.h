#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <cstring>

#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>


#ifndef TAG
#define TAG " "
#endif

// Names for shared memory (both the actual memory and the mutex to control access)
#define SHM_NAME "/hlxexam-shm"
#define SHM_MUTEX_NAME "/hlxexam-shm-mutex"
constexpr int SHARED_MEMORY_DEFAULT_SIZE = 256;

 // Will be appended after the last number written to shared memory, whenever mainParent updates the data in the shared memory
constexpr int SHM_DIRTY = -1;
constexpr int SHM_CLEAN = 0;
constexpr int SHM_POLLING_INTERVAL_MS = 10; // polling interval for shared memory access (in milliseconds)

using std::cout;
using std::endl;
using std::string;
using std::vector;

struct SharedMemory {
	int* buffer;
	int size;
};

// Convenience helpers for console output
enum LogOutputNewline { NO_NEWLINE, NEWLINE, ONLY_NEWLINE };
void log(const string& message);
void log(const string& message, LogOutputNewline nl);
void log(LogOutputNewline);
void loge(const string& message);

void change_shm_size(const int new_shm_size, int shm_fd, struct SharedMemory& shared_memory); 

inline void log(const string& message) {
	cout << TAG << message << endl;
}

inline void log(const string& message, LogOutputNewline no_newline) {
	if (no_newline == NO_NEWLINE) {
		cout << message;
	} else {
		cout << TAG << message << endl;
	}
}

inline void log(LogOutputNewline nl) {
	cout << endl;
}

inline void loge(const string& message) {
	std::cerr << TAG << message << endl;
}

inline void exit_with_error(const string message) {
	loge(message + " " + string(strerror(errno)));
	exit(1);
}

void increase_shared_memory(const size_t numbers_size, int shm_fd, struct SharedMemory& shared_memory) {
	log("WARNING: Received more numbers than current size of shared memory would support. Increasing size of shared memory region");
	const int n_size = static_cast<int>(numbers_size);
	const int def_size_factor = n_size / SHARED_MEMORY_DEFAULT_SIZE;
	const int new_shm_size = (1 + def_size_factor) * SHARED_MEMORY_DEFAULT_SIZE; 

	change_shm_size(new_shm_size, shm_fd, shared_memory);
}

void decrease_shared_memory(const size_t numbers_size, int shm_fd, struct SharedMemory& shared_memory) {
	// If the user has entered 0, this would trigger a call to decrease_shared_memory()
	// However, in that case, nothing needs to be done, because that means, no new data is coming
	// and we can exit
	if (numbers_size == 0)
		return;

	log("WARNING: Received way less numbers than shared memory size is available. Shrinking it"); 
	const int n_size = static_cast<int>(numbers_size);
	const int new_shm_size = 2 * n_size; // Shrink region to 2x the numbers, just in case  

	change_shm_size(new_shm_size, shm_fd, shared_memory);
}

inline void change_shm_size(const int new_shm_size, int shm_fd, struct SharedMemory& shared_memory) {
	log("New shared shared_memory region will have size " + std::to_string(new_shm_size));
 
	if (ftruncate(shm_fd, new_shm_size*sizeof(int)) == -1)
		exit_with_error("ftruncate has failed with error, when trying to increase_shared_memory()");

	if (munmap(shared_memory.buffer, shared_memory.size*sizeof(int)) == -1)
		exit_with_error("in change_shm_size(): munmap failed with error ");

	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;
	shared_memory.buffer = static_cast<int*>(mmap(NULL, new_shm_size*sizeof(int), prot, flags, shm_fd, 0));
	if (shared_memory.buffer == MAP_FAILED)
		exit_with_error("In change_shm_size(): mmap failed ");

	shared_memory.size = new_shm_size;

}

#endif
