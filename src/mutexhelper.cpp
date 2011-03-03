#include <mutexhelper.h>

using namespace std;

pthread_mutex_t MutexHelper::m_iListMutex;
bool MutexHelper::inited = false;
map<pthread_mutex_t*, MutexState> MutexHelper::m_mList;

void MutexHelper::Init()
{
	pthread_mutex_init(&m_iListMutex, NULL);
	inited = true;
}

MutexHelper::MutexHelper(pthread_mutex_t *p_piMutex)
{
	if(!inited)
	{
		Init();
	}

	pthread_mutex_lock(&m_iListMutex);
	m_piMutex = p_piMutex;
	
	if(m_mList.find(m_piMutex) != m_mList.end() &&
	   pthread_equal(pthread_self(), m_mList[m_piMutex].m_iThread))
	{
		// Our lock, just increase counter
		m_mList[m_piMutex].m_iLocks++;
	}
	else
	{
		// Not our lock or not locked yet, lock and set
		pthread_mutex_lock(m_piMutex);
		m_mList[m_piMutex].m_iThread = pthread_self();
		m_mList[m_piMutex].m_iLocks = 1;
	}	
	
	pthread_mutex_unlock(&m_iListMutex);
}

MutexHelper::~MutexHelper()
{
	pthread_mutex_lock(&m_iListMutex);
	
	m_mList[m_piMutex].m_iLocks--;
	if(m_mList[m_piMutex].m_iLocks == 0)
	{
		pthread_mutex_unlock(m_piMutex);
		m_mList.erase(m_piMutex);
	}
	
	pthread_mutex_unlock(&m_iListMutex);
}