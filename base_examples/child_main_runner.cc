#include "child_main_runner.h"

#include "../base/message_loop.h"

#include "child_process.h"
#include "child_thread_impl.h"

class ChildMainRunnerImpl: public ChildMainRunner {
public:
  ChildMainRunnerImpl() : is_initialized_(false), is_shutdown_(false) {}

  ~ChildMainRunnerImpl() {
    if (is_initialized_ && !is_shutdown_)
      Shutdown();
  }

  virtual int Initialize() OVERRIDE {
    return -1;
  }

  virtual int Run() OVERRIDE {
    MessageLoop main_message_loop(MessageLoop::TYPE_DEFAULT);

    base::PlatformThread::SetName("ChildMainThread");

    ChildProcess child_process;
    new ChildThreadImpl();

    MessageLoop::current()->Run();
    return 0;
  }

  virtual void Shutdown() {
    is_shutdown_ = true;
  }

  private:
  bool is_initialized_;

  bool is_shutdown_;

  DISALLOW_COPY_AND_ASSIGN(ChildMainRunnerImpl);
};

ChildMainRunner* ChildMainRunner::Create() {
  return new ChildMainRunnerImpl();
}