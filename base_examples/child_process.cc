#include "child_process.h"
#include "child_thread_impl.h"

ChildProcess* ChildProcess::child_process_;

ChildProcess::ChildProcess(): shutdown_event_(true, false),
  io_thread_("ChildIOThread") {
  child_process_ = this;
  io_thread_.StartWithOptions(base::Thread::Options(MessageLoop::TYPE_IO, 0));
}

ChildProcess::~ChildProcess() {
  shutdown_event_.Signal();
  main_thread_.reset();
  child_process_ = NULL;
}

void ChildProcess::set_main_thread(ChildThreadImpl* thread) {
  main_thread_.reset(thread);
}