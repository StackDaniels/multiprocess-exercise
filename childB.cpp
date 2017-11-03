#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#define TAG "[childB] "
#include "common.h"

#include "computations.h"

inline void print_shm(struct SharedMemory& shared_memory, const int count); 
double calcualte_geometric_mean(struct SharedMemory& shared_memory, const int count); 

int main() {
	log("Child process started");

	// Get mutex for access control to shared memory
	sem_t* mutex = sem_open(SHM_MUTEX_NAME, 0, 0, 1);
	if (mutex == SEM_FAILED) 
		exit_with_error("sem_open has failed with error: ");

	// Get shared memory file handle
	int fd_shm = shm_open(SHM_NAME, O_RDWR, 0);
	if (fd_shm == -1) 
		exit_with_error("shm_open has failed with error: ");

	if (ftruncate(fd_shm, SHARED_MEMORY_DEFAULT_SIZE) == -1) 
		exit_with_error("ftruncate for setting shared memory size has failed: Exiting ");

	struct SharedMemory shared_memory;
	shared_memory.size = SHARED_MEMORY_DEFAULT_SIZE*sizeof(int);

	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;
	shared_memory.buffer = static_cast<int*>(mmap(NULL, shared_memory.size, prot, flags, fd_shm, 0));
	if (shared_memory.buffer == MAP_FAILED) 
		exit_with_error("mmap for shared memory failed.");

	int num_received = 2;

	while (num_received > 1) {

		if (sem_wait(mutex) == -1)
			exit_with_error("sem_wait for mutex has failed");

		num_received = shared_memory.buffer[0] + 1;
		
		//log(std::to_string(shared_memory.buffer[num_received]) + ", " + std::to_string(shared_memory.buffer[num_received + 1]));
		// If no new data has arrived, sleep for a short while and continue
		if (shared_memory.buffer[num_received + 1] != SHM_DIRTY) {
			if (sem_post(mutex) == -1)
				exit_with_error("sem_post for mutex has failed");
			std::this_thread::sleep_for(std::chrono::milliseconds(SHM_POLLING_INTERVAL_MS));
			continue;
		}

		if (num_received > shared_memory.size)
			increase_shared_memory(static_cast<size_t>(num_received), fd_shm, shared_memory);
		else if (shared_memory.size > static_cast<size_t>(2*num_received))
			decrease_shared_memory(static_cast<size_t>(num_received), fd_shm, shared_memory);
		
		log("Random numbers received from shared memory: ", NO_NEWLINE);
		print_shm(shared_memory, num_received);

		std::sort(shared_memory.buffer + 1, shared_memory.buffer + num_received);

		log("Sorted sequence: ", NO_NEWLINE);
		print_shm(shared_memory, num_received);

		double geometric_mean = calculate_geometric_mean(shared_memory, num_received);
		log("Geometric mean: " + std::to_string(geometric_mean));

		// Marking shared memory as read and "clean"
		shared_memory.buffer[num_received + 1] = SHM_CLEAN; 
		if (sem_post(mutex) == -1)
			exit_with_error("sem_post for mutex has failed");
		
	} 

	log("Child process exits");
	return 0;
}

void print_shm(struct SharedMemory& shared_memory, const int count) {
	const int cnt = std::min(count, shared_memory.size);
	for (int i = 1; i < cnt; i++) {
		log(std::to_string(shared_memory.buffer[i]) + ", ", NO_NEWLINE);
	}
	log(ONLY_NEWLINE);
}
