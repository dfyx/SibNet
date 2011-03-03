#ifndef MUTEXHELPER_H
#define MUTEXHELPER_H

#include <pthread.h>
#include <map>

struct MutexState
{
	pthread_t m_iThread;
	size_t m_iLocks;
};

class MutexHelper
{
private:
	pthread_mutex_t *m_piMutex;
	
	static pthread_mutex_t m_iListMutex;
	static std::map<pthread_mutex_t*, MutexState> m_mList;
public:
	static void Init();
	
	MutexHelper(pthread_mutex_t *p_piMutex);
	~MutexHelper();
};

#endif // MUTEXHELPER_H