#include "frame_window.h"
#include "Resource.h"
#include "../base/logging.h"
#include "../base/win/wrapped_window_proc.h"
#include "../base/bind.h"
#include "thread_helper.h"
#include "leon_generator.h"

extern HINSTANCE g_instance;

FrameWindow::FrameWindow() {

}

FrameWindow::~FrameWindow() {

}

FrameWindow* FrameWindow::GetInstance() {
  static FrameWindow* frame_window = NULL;
  if (NULL == frame_window)
    frame_window = new FrameWindow;
  return frame_window;
}

void FrameWindow::Initialize() {
  LoadString(g_instance, IDS_APP_TITLE, title_, MAX_LOADSTRING);
  LoadString(g_instance, IDC_BASE_EXAMPLES, window_class_name_, MAX_LOADSTRING);
  static ATOM atom = 0;
  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(window_class_name_,
    base::win::WrappedWindowProc<WndProc>, CS_HREDRAW|CS_VREDRAW, 0, 0, LoadCursor(0, IDC_ARROW),
    reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), MAKEINTRESOURCE(IDC_BASE_EXAMPLES),
    NULL, NULL, &window_class);
  atom = RegisterClassEx(&window_class);
  DCHECK(atom);

  window_handle_ = CreateWindow(MAKEINTATOM(atom), title_, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, g_instance, NULL);
}

void FrameWindow::Show() {
  ShowWindow(window_handle_, SW_SHOWNORMAL);
  UpdateWindow(window_handle_);
}

LRESULT FrameWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hDc;

  switch (message) {
  case WM_COMMAND:
    wmId = LOWORD(wParam);
    wmEvent = HIWORD(wParam);
    switch (wmId) {
    case IDM_ABOUT:
      DialogBox(g_instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    case ID_FILE_NEWLEON:
      {
        LEON::GenerateParams params;
        LEON::Generate(params);
      }
      break;
    case ID_FILE_NEWLEONWITHNEWPROCESS:
      {
        LEON::GenerateParams params;
        params.create_process_ = true;
        LEON::Generate(params);
        break;
      }
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    break;
  case WM_PAINT:
    hDc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

INT_PTR FrameWindow::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;
  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
    }
    break;
  }
  return (INT_PTR)FALSE;
}