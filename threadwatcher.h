//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// threadwatcher.h
#pragma once


#include <memory>

class Engine;

class ThreadWatcher
{
public:
	ThreadWatcher(Engine* engine);
	~ThreadWatcher();

	void attach(DWORD thread_id);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


