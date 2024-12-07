#include <windows.h>
#include "threadwatcher.h"
#include "engine.h"
#include <mutex>
#include <thread>
#include <vector>

struct ThreadWatcher::PImpl
{
	void run_watcher();
	void add_thread(DWORD thread_id);
	void abort();
	bool is_abort();
	void get_handles(std::vector<HANDLE>& handles);
	void notify_thread_detach(size_t index);

	Engine* m_engine = nullptr;
	
	std::mutex m_mtx;
	bool m_is_initialized = false;
	bool m_is_abort = false;
	HANDLE m_wakeup_event = nullptr;
	std::vector<std::pair<DWORD, HANDLE> > m_thread_ids;
};


void ThreadWatcher::PImpl::run_watcher()
{
	std::lock_guard<std::mutex> lock(m_mtx);

	std::thread th([&]() {
//////////////////////////
		m_wakeup_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		std::vector<HANDLE> handles;
		while(is_abort() == false) {

			get_handles(handles);

			DWORD n = WaitForMultipleObjects((int)handles.size(), handles.data(), FALSE, INFINITE);
			if (n < WAIT_OBJECT_0 || WAIT_OBJECT_0 + handles.size() <= n) {
				continue;
			}

			bool is_wakeup = n == WAIT_OBJECT_0;
			if (is_wakeup) {
				continue;
			}

			size_t index = n - WAIT_OBJECT_0 - 1;
			notify_thread_detach(index);
		}

		CloseHandle(m_wakeup_event);
		m_wakeup_event = nullptr;
//////////////////////////
	});
	th.detach();
	m_is_initialized = true;
}

void ThreadWatcher::PImpl::add_thread(DWORD thread_id)
{
	if (m_is_initialized == false) {
		run_watcher();
	}

	std::lock_guard<std::mutex> lock(m_mtx);
	HANDLE handle = OpenThread(SYNCHRONIZE, FALSE, thread_id);
	m_thread_ids.push_back(std::pair<DWORD,HANDLE>(thread_id, handle));
	// 監視スレッドを起こす
	SetEvent(m_wakeup_event);
}

void ThreadWatcher::PImpl::abort()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_is_initialized == false) {
		return;
	}

	m_is_abort = true;
	// 監視スレッドを起こす
	SetEvent(m_wakeup_event);
}

bool ThreadWatcher::PImpl::is_abort()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_is_abort;
}

void ThreadWatcher::PImpl::get_handles(std::vector<HANDLE>& handles)
{
	std::lock_guard<std::mutex> lock(m_mtx);

	handles.resize(m_thread_ids.size() + 1);
	handles[0] = m_wakeup_event;

	for (size_t i = 0; i < m_thread_ids.size(); ++i) {
		handles[i + 1] = m_thread_ids[i].second;
	}
}

void ThreadWatcher::PImpl::notify_thread_detach(size_t index)
{
	DWORD detached_thread_id = 0;
	HANDLE detached_thread_handle = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		if (m_thread_ids.size() <= index) {
			return;
		}
		const auto& item = m_thread_ids[index];
		detached_thread_id = item.first;
		detached_thread_handle = item.second;

		m_thread_ids.erase(m_thread_ids.begin() + index);
	}

	CloseHandle(detached_thread_handle);
	m_engine->threadDetachNotify(detached_thread_id);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



ThreadWatcher::ThreadWatcher(Engine* engine) : in(new PImpl)
{
	in->m_engine = engine;
}

ThreadWatcher::~ThreadWatcher()
{
	in->abort();
}

void ThreadWatcher::attach(DWORD thread_id)
{
	in->add_thread(thread_id);

	in->m_engine->threadAttachNotify(thread_id);
}
