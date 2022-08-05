#include "UITaskProxy.h"

#define TASK_CLASS_NAME "webrtc_windows_native_task_proxy"
#define TASK_RUN_MSG (WM_USER + 1)

LRESULT UITaskProxy::WindowMessageProcedure(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	UITaskProxy *self = (UITaskProxy *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (nMsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}

	case TASK_RUN_MSG: {
		if (self) {
			self->RunTask();
			return 0;
		}
		break;
	}
	}

	return DefWindowProcA(hWnd, nMsg, wParam, lParam);
}

UITaskProxy::~UITaskProxy()
{
	if (::IsWindow(m_hWnd)) {
		Uninitialize();
		assert(false);
	}
}

bool UITaskProxy::Initialize()
{
	WNDCLASSA wc = {0};
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = (WNDPROC)&UITaskProxy::WindowMessageProcedure;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TASK_CLASS_NAME;

	if (0 == RegisterClassA(&wc)) {
		assert(false);
		return false;
	}

	m_hWnd = CreateWindowA(TASK_CLASS_NAME, TASK_CLASS_NAME, WS_POPUPWINDOW, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (!::IsWindow(m_hWnd)) {
		UnregisterClassA(TASK_CLASS_NAME, GetModuleHandle(NULL));
		assert(false);
		return false;
	}

	::ShowWindow(m_hWnd, SW_HIDE);
	::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	return true;
}

void UITaskProxy::Uninitialize()
{
	SendMessage(m_hWnd, WM_CLOSE, 0, 0);
	UnregisterClassA(TASK_CLASS_NAME, GetModuleHandle(NULL));

	ClearTask();
}

void UITaskProxy::PushTask(uint64_t key, std::function<void()> func)
{
	assert(::IsWindow(m_hWnd));

	std::lock_guard<std::recursive_mutex> autoLock(m_lockList);

	ST_TaskInfo info;
	info.key = key;
	info.func = func;

	m_TaskList.push_back(info);

	PostMessage(m_hWnd, TASK_RUN_MSG, 0, 0);
}

void UITaskProxy::PopTask(uint64_t key)
{
	std::lock_guard<std::recursive_mutex> autoLock(m_lockList);

	std::vector<ST_TaskInfo>::iterator itr = m_TaskList.begin();
	while (itr != m_TaskList.end()) {
		if (itr->key == key) {
			itr = m_TaskList.erase(itr);
			continue;
		}
		++itr;
	}
}

void UITaskProxy::ClearTask()
{
	std::lock_guard<std::recursive_mutex> autoLock(m_lockList);
	m_TaskList.clear();
}

void UITaskProxy::RunTask()
{
	std::lock_guard<std::recursive_mutex> autoLock(m_lockList);

	for (auto &item : m_TaskList)
		item.func();

	m_TaskList.clear();
}
