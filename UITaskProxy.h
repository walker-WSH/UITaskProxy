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
	void PushTask(uint64_t key, std::function<void()> func);
	void PopTask(uint64_t key);
	void ClearTask();

protected:
	static LRESULT WindowMessageProcedure(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
	void RunTask();

private:
	struct ST_TaskInfo {
		uint64_t key = 0;
		std::function<void()> func;
	};

	DWORD m_uThreadID = 0;
	HWND m_hWnd = 0;
	std::recursive_mutex m_lockList;
	std::vector<ST_TaskInfo> m_TaskList;
};
