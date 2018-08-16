#pragma once

class NsmLogWnd
{
public:
	NsmLogWnd();
	~NsmLogWnd();

	void Create(HINSTANCE _hInst, HWND _hWnd);
	void Close();

	void SetClientSize();


private:

	HWND m_hWnd;
	HINSTANCE m_hInst;
	YsVec2 m_windowSize;
	bool m_isActiveWindow;

	class LogWndThread
	{
	public:
		LogWndThread();
		~LogWndThread();
		void Begin(HINSTANCE _hInst, HWND _parentHwnd, YsVec2& _size);
		void End();

	private:
		void SetLogWndProc();
	private:
		std::function<LRESULT CALLBACK(HWND, UINT, WPARAM, LPARAM)> m_wndProc;
		std::thread m_thread;
		bool m_enableThread;
		HWND m_hWnd;
	};

};