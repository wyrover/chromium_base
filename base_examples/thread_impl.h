#ifndef THREAD_IMPL_H_
#define THREAD_IMPL_H_

#include "../base/threading/thread.h"
#include "thread_helper.h"

class ThreadDelegate {
public:
  virtual ~ThreadDelegate() {}

  virtual void Init() = 0;

  virtual void CleanUp() = 0;
};

class ThreadImpl: public ThreadHelper, public base::Thread {
public:
  explicit ThreadImpl(ThreadHelper::ID identifier);

  ThreadImpl(ThreadHelper::ID identifier, MessageLoop* message_loop);
  virtual ~ThreadImpl();

protected:
  virtual void Init() OVERRIDE;
  virtual void Run(MessageLoop* message_loop) OVERRIDE;
  virtual void CleanUp() OVERRIDE;

private:
  friend class ThreadHelper;

  static bool PostTaskHelper(ThreadHelper::ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task, base::TimeDelta delay, bool nestable);

  void Initialize();

  ID identifier_;

};

class SubThread: public ThreadImpl {
public:
  explicit SubThread(ThreadHelper::ID identifier);
  virtual ~SubThread();

protected:
  virtual void Init() OVERRIDE;

  virtual void CleanUp() OVERRIDE;

private:

  DISALLOW_COPY_AND_ASSIGN(SubThread);
};
#endif