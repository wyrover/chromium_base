#include "frame_window.h"
#include "Resource.h"

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

  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = g_instance;
  wcex.hIcon = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_BASE_EXAMPLES));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDC_BASE_EXAMPLES);
  wcex.lpszClassName = window_class_name_;
  wcex.hIconSm = LoadIcon(g_instance, MAKEINTRESOURCE(IDI_SMALL));

  RegisterClassEx(&wcex);

  window_handle_ = CreateWindow(window_class_name_, title_, WS_OVERLAPPEDWINDOW,
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