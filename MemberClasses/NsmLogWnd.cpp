#include "NsmLogWnd.h"

NsmLogWnd::NsmLogWnd()
{
	m_hWnd = nullptr;
	m_hInst = nullptr;
	m_isActiveWindow = false;
	m_windowSize = YsVec2(0.0f, 0.0f);
}

NsmLogWnd::~NsmLogWnd()
{
}

void NsmLogWnd::Create(HINSTANCE _hInst, HWND _hWnd)
{
}

void NsmLogWnd::Close()
{
}

void NsmLogWnd::SetClientSize()
{
}


NsmLogWnd::LogWndThread::LogWndThread()
{
}

NsmLogWnd::LogWndThread::~LogWndThread()
{
}

// LogWndThreadクラス
void NsmLogWnd::LogWndThread::Begin(HINSTANCE _hInst, HWND _parentHwnd, YsVec2& _size)
{
	m_thread = std::thread([&,_hInst, _parentHwnd, _size]()
	{
		WNDCLASS wc;
		MSG					msg;

		// ウインドウクラスの設定
		wc.lpszClassName = "DebugWindow";
		wc.hInstance = _hInst;
		wc.lpfnWndProc = (WNDPROC)m_wndProc.target<LRESULT CALLBACK>();
		wc.hCursor = LoadCursor((HINSTANCE)nullptr, IDC_ARROW);
		wc.hIcon = LoadIcon((HINSTANCE)nullptr, IDI_APPLICATION);
		wc.lpszMenuName = nullptr;
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		int x;
		int y;
		int w;
		int h;

		//m_hWnd = CreateWindow
		//(
		//	"ChatLog",
		//	"ForChatWindow",
		//	WS_CHILD | WS_VSCROLL | WS_HSCROLL
		//	)

	});
}

void NsmLogWnd::LogWndThread::End()
{
}

void NsmLogWnd::LogWndThread::SetLogWndProc()
{
	m_wndProc = [](HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
	{
		switch (_msg)
		{
			return (DefWindowProc(_hWnd, _msg, _wParam, _lParam));
		}
		return (LRESULT)0;
	};
}
