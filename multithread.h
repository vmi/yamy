//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// multithread.h


#ifndef _MULTITHREAD_H
#  define _MULTITHREAD_H

///
class SyncObject
{
public:
	///
	virtual void acquire() = 0;
	///
	virtual void acquire(int) {
		acquire();
	}
	///
	virtual void release() = 0;
};

///
class Acquire
{
	SyncObject* m_so;	///

public:
	///
	Acquire(SyncObject* i_so) : m_so(i_so) {
		m_so->acquire();
	}
	///
	Acquire(SyncObject* i_so, int i_n) : m_so(i_so) {
		m_so->acquire(i_n);
	}
	///
	~Acquire() {
		m_so->release();
	}
};

#endif // !_MULTITHREAD_H
