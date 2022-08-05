// PostTask.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <memory>
#include "UITaskProxy.h"

class CTest {
public:
	CTest() { printf("gou zao \n"); }
	~CTest() { printf("xi gou \n"); }
};

#include <Windows.h>
#include <process.h>

UITaskProxy pxy;

static unsigned __stdcall ThreadFunc(void *pParam)
{
	printf("worker thread   %u \n", GetCurrentThreadId());
	while (1) {
		pxy.PushTask(1, []() { printf("run task in thread   %u \n", GetCurrentThreadId()); });

		int value = GetTickCount();
		pxy.PushTask(1, [value]() { printf("run task in thread   %u  value:%u \n", GetCurrentThreadId(), value); });

		Sleep(2000);
	}
	return 0;
}

int main()
{
	printf("main thread   %u \n", GetCurrentThreadId());

	pxy.Initialize();
	_beginthreadex(0,0,ThreadFunc,0,0,0);


	{
		std::shared_ptr<CTest> obj(new CTest());
		printf("p1   %u \n", obj.use_count());

		pxy.PushTask(1, [obj]() { printf("I am WSH (%u) \n", obj.use_count()); });
		printf("p2   %u \n", obj.use_count());
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	printf("p4 \n");
	pxy.Uninitialize();
	printf("p5 \n");

	getchar();
	return 0;
}
