// base_examples.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include <tchar.h>
#include "base_examples.h"
#include "../base/command_line.h"
#include "../base/memory/scoped_ptr.h"
#include "../base/at_exit.h"
#include "main_runner.h"

HINSTANCE g_instance = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
  CommandLine::Init(0, NULL);
  g_instance = hInstance;
  base::AtExitManager exit_manager;

  scoped_ptr<MainRunner> main_runner_(MainRunner::Create());
  int exit_code = main_runner_->Initialize();
  if (exit_code >= 0)
    return exit_code;
  exit_code = main_runner_->Run();
  main_runner_->Shutdown();

  return exit_code;
}