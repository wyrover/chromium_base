#ifndef CHILD_PROCESS_H_
#define CHILD_PROCESS_H_

#include "../base/synchronization/waitable_event.h"
#include "../base/message_loop.h"
#include "../base/threading/thread.h"

class ChildThreadImpl;

class ChildProcess {
public:
  ChildProcess();
  virtual ~ChildProcess();

  ChildThreadImpl* main_thread() { return main_thread_.get(); }

  void set_main_thread(ChildThreadImpl* thread);

  static ChildProcess* current() { return child_process_; }

  MessageLoop* io_message_loop() { return io_thread_.message_loop(); }

  base::MessageLoopProxy* io_message_loop_proxy() {
    return io_thread_.message_loop_proxy();
  }
  base::WaitableEvent* GetShutDownEvent() { return &child_process_->shutdown_event_; }
private:
  base::WaitableEvent shutdown_event_;

  base::Thread io_thread_;

  static ChildProcess* child_process_;

  scoped_ptr<ChildThreadImpl> main_thread_;

  DISALLOW_COPY_AND_ASSIGN(ChildProcess);
};
#endif