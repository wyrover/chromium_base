#include "main_runner.h"

#include "../base/memory/scoped_ptr.h"
#include "main_loop.h"
#include "frame_window.h"

class MainLoop;

bool g_exited_main_message_loop = false;

namespace {
  class MainRunnerImpl: public MainRunner {
  public:
    MainRunnerImpl()
      : is_initialized_(false)
      , is_shutdown_(false)
      , created_threads_(false) {}

    ~MainRunnerImpl() {
      if (is_initialized_ && !is_shutdown_)
        Shutdown();
    }

    virtual int Initialize() OVERRIDE {
      is_initialized_ = true;

      main_loop_.reset( new MainLoop());
      main_loop_->Init();
      main_loop_->StartMainMessageLoop();
      main_loop_->CreateThreads();
      int result_code = main_loop_->GetResultCode();
      if (result_code > 0)
        return result_code;
      created_threads_ = true;
      FrameWindow::GetInstance()->Initialize();
      FrameWindow::GetInstance()->Show();
      return -1;
    }

    virtual int Run() OVERRIDE {
      main_loop_->RunMainMessageLoop();
      return main_loop_->GetResultCode();
    }

    virtual void Shutdown() OVERRIDE {
      g_exited_main_message_loop = true;
      if (created_threads_)
        main_loop_->ShutdownThreadAndCleanUp();
      main_loop_.reset(NULL);
      is_shutdown_ = true;
    }

  protected:
    bool is_initialized_;

    bool is_shutdown_;

    bool created_threads_;

    scoped_ptr<MainLoop> main_loop_;

    DISALLOW_COPY_AND_ASSIGN(MainRunnerImpl);
  };
}

MainRunner* MainRunner::Create() {
  return new MainRunnerImpl();
}