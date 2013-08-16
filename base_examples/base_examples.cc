// base_examples.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include <tchar.h>
#include "base_examples.h"
#include "../base/command_line.h"
#include "../base/memory/scoped_ptr.h"
#include "../base/at_exit.h"
#include "../base/ipc/ipc_switches.h"
#include "main_runner.h"
#include "child_main_runner.h"

HINSTANCE g_instance = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  CommandLine::Init(0, NULL);
  g_instance = hInstance;
  base::AtExitManager exit_manager;
  int exit_code = 0;
  if (CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kProcessChannelID).empty()) {
    scoped_ptr<MainRunner> main_runner(MainRunner::Create());
    exit_code = main_runner->Initialize();
    if (exit_code >= 0)
      return exit_code;
    exit_code = main_runner->Run();
    main_runner->Shutdown();
  } else {
#ifdef _DEBUG
    MessageBox(NULL, L"Debug", L"For Debug", MB_OK);
#endif
    scoped_ptr<ChildMainRunner> child_main_runner(ChildMainRunner::Create());
    exit_code = child_main_runner->Initialize();
    if (exit_code >= 0)
      return exit_code;
    exit_code = child_main_runner->Run();
    child_main_runner->Shutdown();
  }
  

  return exit_code;
}