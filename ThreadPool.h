#include <semaphore.h>
#include <pthread.h>
#include <queue>
#include <stack>

typedef void*(*start_routine)(void*);
class ThreadPool;

/*
 * Task结构
 *  func成员为函数指针
 *  arg为func的参数
 * 每次线程执行完Task后，直接将Task抛弃，若arg是动态分配内存的指针，会出现内存泄露，需要完善
 */
struct Task
{
	start_routine func;
	void * arg;
};

/*
 * ThreadWorker结构
 *  PoolManager成员直线线程对应的线程池的管理器
 *  id为线程id
 *  task为线程当前将要执行的任务或已执行的任务
 *  mutex和cond用于线程同步
 *  terminate用于在线程每次被唤醒时判断是否终止，可用于清除线程池中过量的空闲进程
 */
struct ThreadWorker
{
	ThreadPool *PoolManager;
	pthread_t id;
	struct Task task;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool terminate;
};

/*
 * ThreadPool类
 *  WORKSIZE指定线程池中最大线程数量
 *  对外接口主要有AddTask，用于向线程池添加任务
 *  terminate用于在线程池中所有线程每次被唤醒时判断是否全部终止，可用于销毁线程池
 *  每次添加任务时，先判断线程池中是否有空闲线程（即idleworkers栈是否非空），
 *      若有空闲进程（非空），则将其中空闲进程弹出，执行任务，
 *      若无空闲进程，则判断线程池正在运行的线程数是否已满，
 *          若未满，则添加新线程执行任务，
 *          若已满，将任务压入任务队列，正在运行的线程执行完后，会判断任务队列是否为空
 *              若非空，则取任务继续执行，
 *              若为空，则等待唤醒。
 *  idleworkers栈用于存放空闲线程
 *  tasks队列用于存放任务
 *  workercounter为当前已创建线程数量，包括空闲线程（阻塞态）和正在运行线程（就绪态和运行态）
 *  mutex用于线程池同步
 *  allidle条件变量表示当前所有线程都空闲，此时可以进行线程销毁
 *  线程实际运行方式是以ThreadWorker指针作为wrapper的参数进行运行
 */
class ThreadPool
{
#define WORKSIZE 2
	public:
		ThreadPool();
		void AddTask(const struct Task &);
		~ThreadPool();
		bool terminate;
	private:
		std::stack<ThreadWorker*> idleworkers;
		std::queue<Task> tasks;
		int workercounter;
		pthread_mutex_t mutex;
		pthread_cond_t allidle;
		static void* wrapper(void *);
};
