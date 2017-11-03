#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>

#include <random>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <thread>

#define TAG "[mainParent] "
#include "common.h"

using std::cin;

pid_t spawn(const string processExecutable);

vector<int> create_random_numbers(const int n);
void send_to_process(const vector<int>& rnd_numbers, int shm_fd, sem_t* mutex, struct SharedMemory& shared_memory);
void send_to_process(const vector<int>& numbers, const char* pipe_name); 

void handle_write_error(int write_ret);

int main() {
	log("Main Process started");

	// Open named pipe to child A
	const char* pipe_name = "/tmp/pipeToChildA";
	int error = mkfifo(pipe_name, 0666);
	if (error != 0) 
		exit_with_error("mkfifo has failed");

	// Creating mutex for access control and shared memory for child B
	sem_unlink(SHM_MUTEX_NAME);
	sem_t* mutex = sem_open(SHM_MUTEX_NAME, O_CREAT, 0660, 1);
	if (mutex == SEM_FAILED) 
		exit_with_error("Creating mutex for shared memory access has failed. Exiting."); 

	// Creating shared memory (file handle, ftruncate and mmap call)
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
	if (fd_shm == -1) 
		exit_with_error("Trying to create shared memory file handle has failed. Exiting.");

	if (ftruncate(fd_shm, SHARED_MEMORY_DEFAULT_SIZE*sizeof(int)) == -1) 
		exit_with_error("ftruncate for setting shared memory size has failed: Exiting "); 

	struct SharedMemory shared_memory;
	shared_memory.size = SHARED_MEMORY_DEFAULT_SIZE;
	
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;
	shared_memory.buffer = static_cast<int*>(mmap(NULL, shared_memory.size, prot, flags, fd_shm, 0)); 
	if (shared_memory.buffer == MAP_FAILED) 
		exit_with_error("mmap for shared memory has failed. Exiting ");

	// Initialize shared memory with a valid state (to prevent childB from immediately exiting the loop)
	shared_memory.buffer[0] = 2;
	shared_memory.buffer[1] = 0;
	shared_memory.buffer[2] = 0;
	shared_memory.buffer[3] = -1;

	// Starting child processes
	pid_t childA_pid = spawn("./childA");
	pid_t childB_pid = spawn("./childB");


	// Wait a little bit, so that user input prompt stays immediately before cursor
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	// User input loop
	int user_input = 1;
	while (user_input != 0) {
		log("Enter integer number n: ");
		cin >> user_input;
		log(ONLY_NEWLINE);
		
		vector<int> rnd_numbers = create_random_numbers(user_input);
		send_to_process(rnd_numbers, pipe_name);
		send_to_process(rnd_numbers, fd_shm, mutex, shared_memory); 
	}

	log("User entered 0, requested exit. Waiting for terminating child processes ... ");

	// Close pipe to process A and terminate it
	unlink(pipe_name);

	sem_close(mutex);

	shm_unlink(SHM_NAME);
	sem_unlink(SHM_MUTEX_NAME);

	// Waiting for child process to complete
	int childB_status = 0;
	int childA_status = 0;

	int waitB_ret = 0;
	int waitA_ret = 0;

	do {
		waitA_ret = waitpid(childA_pid, &childA_status, 0);
		waitB_ret = waitpid(childB_pid, &childB_status, 0);
	} while (waitB_ret == -1 || waitA_ret == -1);

	log("done! Exiting.");

	return 0;
}

pid_t spawn(const string processExecutable) {
	pid_t pid = fork();
	switch (pid)
	{
	case -1: // error
		exit_with_error("Error in spawn after calling fork(): ");
	case 0: // child process
		execl(processExecutable.c_str(), "", (char*)NULL);
		loge("Error in call to execl");
		exit(1);
	default: // parent process, pid now contains the process ID
		log(processExecutable + " Process Created");
		break;
	}
	return pid;
}

vector<int> create_random_numbers(const int n) {
	std::random_device rd;
	std::mt19937 mt(rd()); // Standard Mersenne twister pseudorandom number generator
	std::uniform_int_distribution<int> dist(50, 100);

	vector<int> numbers;
	numbers.reserve(n);

	for (int i=0; i < n; i++) {
		numbers.push_back(dist(mt));
	}
	return numbers;
}

void send_to_process(const vector<int>& numbers, const char* pipe_name) {
	int pipe_fd = open(pipe_name, O_WRONLY);
	if (pipe_fd == -1) 
		exit_with_error("Opening pipe for write-only failed");

	// First, write to the pipe, how many random numbers there are
	size_t  num_rnd_numbers = numbers.size();
	int bytes = write(pipe_fd, &num_rnd_numbers, sizeof(size_t));
	if (bytes == -1) {
		handle_write_error(bytes);
		return;
	}

	// Then, write the random numbers
	for (int num : numbers) {
		int bytes = write(pipe_fd, &num, sizeof(int));
		if (bytes == -1) {
			handle_write_error(bytes);
			return;
		} else {
			log(std::to_string(num) + ", ", NO_NEWLINE);
		}
	}
	log(ONLY_NEWLINE);
	close(pipe_fd);
}

void handle_write_error(int write_ret) {
	loge("write() failed (no bytes written)");
	loge("Error is: " + string(strerror(errno)));
}

void send_to_process(const vector<int>& numbers, int shm_fd, sem_t* mutex, struct SharedMemory& shared_memory) {
	// Same way as with the pipe, the first number written to the shared memory is the length
	// of the shm buffer
	
	if (sem_wait(mutex) == -1) 
		exit_with_error("Trying to lock mutex for accessing shared memory failed: ");

	const size_t max_num_idx = numbers.size() + 1;
	if (max_num_idx > shared_memory.size) 
		increase_shared_memory(numbers.size(), shm_fd, shared_memory);
	else if (shared_memory.size > 2*max_num_idx)
		decrease_shared_memory(numbers.size(), shm_fd, shared_memory);

	memset(shared_memory.buffer, 0, shared_memory.size);
	shared_memory.buffer[0] = static_cast<int>(numbers.size());
	
	for (int i = 1; i < max_num_idx; i++) {
		shared_memory.buffer[i] = numbers.at(i-1);
	}

	log("Written " + std::to_string(shared_memory.buffer[0]) + " numbers to shared memory");
	for (size_t i = 1; i < max_num_idx; i++) {
		log(std::to_string(shared_memory.buffer[i]) + ", ", NO_NEWLINE);
	}
	log(ONLY_NEWLINE);
	
	// Marking shared memory as modified
	shared_memory.buffer[max_num_idx + 1] = SHM_DIRTY;

	if (sem_post(mutex) == -1) 
		exit_with_error("Trying to unlock mutex for accessing shared memory failed: "); 
}
