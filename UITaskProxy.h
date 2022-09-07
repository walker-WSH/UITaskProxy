#pragma once
#include <Windows.h>
#include <memory>
#include <assert.h>
#include <functional>
#include <vector>
#include <mutex>

class UITaskProxy {
public:
	UITaskProxy() = default;
	virtual ~UITaskProxy();

	// Called from UI thread
	bool Initialize();
	void Uninitialize();

	// Called from any thread
	void PushTask(uint64_t key, std::function<void()> func, bool waitDone = false);
	void PushAsyncTask(uint64_t key, std::function<void()> func);
	void PopTask(uint64_t key);

protected:
	static LRESULT WindowMessageProcedure(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
	void RunTask();
	void PushTaskInner(uint64_t key, std::function<void()> func, bool waitDone);

private:
	class UITaskInfo {
	public:
		HANDLE event = 0;
		uint64_t key = 0;
		std::function<void()> func;

		UITaskInfo(HANDLE hEvent);
		virtual ~UITaskInfo();
	};

	bool m_bStopped = false;
	DWORD m_uThreadID = 0;
	HWND m_hWnd = 0;
	std::recursive_mutex m_lockList;
	std::vector<std::shared_ptr<UITaskInfo>> m_TaskList;
};
