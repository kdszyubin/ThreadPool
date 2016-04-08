#include "ThreadPool.h"
#include <iostream>

void ThreadPool::AddTask(const struct Task& task)
{
	pthread_mutex_lock(&mutex);
	if (!idleworkers.empty())
	{
		ThreadWorker &worker = *idleworkers.top();
		idleworkers.pop();
		std::cout << "out:"  << worker.id << "  size:" << idleworkers.size() << std::endl;
		worker.task = task;
		pthread_mutex_lock(&worker.mutex);
		pthread_cond_signal(&worker.cond);
		pthread_mutex_unlock(&worker.mutex);
	}
	else if (workercounter != WORKSIZE)
	{
		ThreadWorker *pworker = new ThreadWorker;
		pworker->PoolManager = this;
		pworker->task = task;
		pthread_mutex_init(&pworker->mutex, NULL);
		pthread_cond_init(&pworker->cond, NULL);
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&pworker->id, &attr, wrapper, (void*)pworker);
		workercounter++;
	}
	else
		tasks.push(task);
	pthread_mutex_unlock(&mutex);
}

void* ThreadPool::wrapper(void* arg)
{
	ThreadWorker &worker = *(ThreadWorker*)arg;
	ThreadPool &pool = *worker.PoolManager;

	pthread_mutex_lock(&pool.mutex);
	std::cout << "create:"  << worker.id << std::endl;
	pthread_mutex_unlock(&pool.mutex);
	while (!pool.terminate && !worker.terminate)
	{
		worker.task.func(worker.task.arg);
		std::cout << "run:" << worker.id << " int:" << (long )worker.task.arg<< std::endl;
		pthread_mutex_lock(&pool.mutex);
		if (!pool.tasks.empty())
		{
			struct Task task = pool.tasks.front();
			pool.tasks.pop();
			worker.task = task;
			std::cout << "task:" << pool.tasks.size() <<  std::endl;
			pthread_mutex_unlock(&pool.mutex);
		}
		else
		{
			pool.idleworkers.push(&worker);
			std::cout << " in" << worker.id << "  size:" << pool.idleworkers.size() << std::endl;
			if (pool.idleworkers.size() == pool.workercounter)
				pthread_cond_signal(&pool.allidle);
			pthread_mutex_unlock(&pool.mutex);
			pthread_mutex_lock(&worker.mutex);
			pthread_cond_wait(&worker.cond, &worker.mutex);
			pthread_mutex_unlock(&worker.mutex);
		}
	}
	pthread_mutex_destroy(&worker.mutex);
	pthread_cond_destroy(&worker.cond);
	delete &worker;
}

ThreadPool::ThreadPool()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&allidle, NULL);
	workercounter = 0;
	terminate = false;
}

ThreadPool::~ThreadPool()
{
	pthread_mutex_lock(&mutex);
	if (idleworkers.size() != workercounter)
		pthread_cond_wait(&allidle, &mutex);
	terminate = true;
	while (idleworkers.size() > 0)
	{
		ThreadWorker &worker = *idleworkers.top();
		idleworkers.pop();
		pthread_mutex_lock(&worker.mutex);
		pthread_cond_signal(&worker.cond);
		pthread_mutex_unlock(&worker.mutex);
		workercounter--;
	}
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&allidle);
}
