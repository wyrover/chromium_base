#ifndef FRAME_WINDOW_H_
#define FRAME_WINDOW_H_

#include <Windows.h>

#define MAX_LOADSTRING 100

class FrameWindow {
public:
  FrameWindow();
  virtual ~FrameWindow();

  void Initialize();

  void Show();

  static FrameWindow* GetInstance();

  static void DoSomethingOnIOThread(HWND hWnd);

  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

  static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

private:
  wchar_t title_[MAX_LOADSTRING];

  wchar_t window_class_name_[MAX_LOADSTRING];

  HWND window_handle_;
};
#endif