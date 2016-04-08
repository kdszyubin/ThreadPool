#include "ThreadPool.h"
#include <iostream>
using namespace std;

#define TASKNUM 10
ThreadPool pool;

void* func(void* x)
{
	//cout << (long long)x;
}

int main()
{
	struct Task task;
	for (int i = 0; i < TASKNUM; ++i)
	{
		task.func = (func);
		task.arg = (void*)(long long)i;
		pool.AddTask(task);
	}
	return 0;
}

