#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#define QUEUE_SIZE 1000000


typedef struct Task {
	pthread_mutex_t waitUntilFinished;
	void (*runTask)(void*, int);
	void* data;
}Task;

void forceShutDownWorkers();
void putTask(Task task);
void startWorkers();
void joinWorkerThreads();