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

  HWND window_handle() { return window_handle_; }

  static FrameWindow* GetInstance();

  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

  static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

  static INT_PTR CALLBACK NewLeon(HWND, UINT, WPARAM, LPARAM);

  static INT_PTR CALLBACK DelLeon(HWND, UINT, WPARAM, LPARAM);

  static INT_PTR CALLBACK Shutdown(HWND, UINT, WPARAM, LPARAM);

private:
  static const unsigned int ContentSize = 100;

  wchar_t title_[MAX_LOADSTRING];

  wchar_t window_class_name_[MAX_LOADSTRING];

  HWND window_handle_;
};
#endif