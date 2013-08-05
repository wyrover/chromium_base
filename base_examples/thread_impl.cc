#include "thread_impl.h"

#include "../base/atomicops.h"
#include "../base/lazy_instance.h"
#include "../base/threading/thread_restrictions.h"

namespace {
  static const char* g_thread_names[ThreadHelper::ID_COUNT] = {
    "",
    "FileThread",
    "IoThread"
  };

  struct ThreadGlobals {
    ThreadGlobals() {
      memset(threads, 0, ThreadHelper::ID_COUNT* sizeof threads[0]);
      memset(thread_delegates, 0, ThreadHelper::ID_COUNT* sizeof thread_delegates[0]);
    }

    base::Lock lock;

    ThreadImpl* threads[ThreadHelper::ID_COUNT];

    ThreadDelegate* thread_delegates[ThreadHelper::ID_COUNT];
  };

  base::LazyInstance<ThreadGlobals>::Leaky
    g_globals = LAZY_INSTANCE_INITIALIZER;
}

ThreadImpl::ThreadImpl(ID identifier)
  : Thread(g_thread_names[identifier])
  , identifier_(identifier) {
    Initialize();
}

ThreadImpl::ThreadImpl(ID identifier, MessageLoop* message_loop)
  : Thread(message_loop->thread_name().c_str())
  , identifier_(identifier) {
    set_message_loop(message_loop);
    Initialize();
}

void ThreadImpl::Init() {
  ThreadGlobals& globals = g_globals.Get();

  using base::subtle::AtomicWord;
  AtomicWord* storage =
    reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  ThreadDelegate* delegate =
    reinterpret_cast<ThreadDelegate*>(stored_pointer);
  if (delegate)
    delegate->Init();
}

void ThreadImpl::Run(MessageLoop* message_loop) {
  Thread::Run(message_loop);
}

void ThreadImpl::CleanUp() {
  ThreadGlobals& globals = g_globals.Get();

  using base::subtle::AtomicWord;
  AtomicWord* storage =
    reinterpret_cast<AtomicWord*>(&globals.thread_delegates[identifier_]);
  AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
  ThreadDelegate* delegate =
    reinterpret_cast<ThreadDelegate*>(stored_pointer);
  if (delegate)
    delegate->CleanUp();
}

void ThreadImpl::Initialize() {
  ThreadGlobals& globals = g_globals.Get();

  base::AutoLock lock(globals.lock);
  globals.threads[identifier_] = this;
}

ThreadImpl::~ThreadImpl() {
  Stop();

  ThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  globals.threads[identifier_] = NULL;
}

bool ThreadImpl::PostTaskHelper(ThreadHelper::ID identifier,
  const tracked_objects::Location& from_here,
  const base::Closure& task, base::TimeDelta delay, bool nestable) {
    ThreadHelper::ID current_thread;
    bool guaranteed_to_outlive_target_thread =
      GetCurrentThreadIdentifier(&current_thread) && current_thread <= identifier;

    ThreadGlobals& globals = g_globals.Get();
    if (!guaranteed_to_outlive_target_thread)
      globals.lock.Acquire();

    MessageLoop* message_loop = globals.threads[identifier]
    ? globals.threads[identifier]->message_loop() : NULL;
    if (message_loop) {
      if (nestable) {
        message_loop->PostDelayedTask(from_here, task, delay);
      } else {
        message_loop->PostNonNestableDelayedTask(from_here, task, delay);
      }
    }
    if (!guaranteed_to_outlive_target_thread)
      globals.lock.Release();

    return !!message_loop;
}

class ThreadHelperMessageLoopProxy: public base::MessageLoopProxy {
public:
  explicit ThreadHelperMessageLoopProxy(ThreadHelper::ID identifier)
    : id_(identifier) {
  }

  virtual bool PostDelayedTask(const tracked_objects::Location& from_here,
    const base::Closure& task, base::TimeDelta delay) OVERRIDE {
      return ThreadHelper::PostNonNestedDelayedTask(id_, from_here, task, delay);
  }

  virtual bool PostNonNestableDelayedTask(const tracked_objects::Location& from_here,
    const base::Closure& task, base::TimeDelta delay) OVERRIDE {
      return ThreadHelper::PostNonNestedDelayedTask(id_, from_here, task, delay);
  }

  virtual bool RunsTasksOnCurrentThread() const OVERRIDE {
    return ThreadHelper::CurrentlyOn(id_);
  }

protected:
  virtual ~ThreadHelperMessageLoopProxy() {}

private:
  ThreadHelper::ID id_;
  DISALLOW_COPY_AND_ASSIGN(ThreadHelperMessageLoopProxy);
};

bool ThreadHelper::PostTask(ID identifier,
  const tracked_objects::Location& from_here, const base::Closure& task) {
    return ThreadImpl::PostTaskHelper(identifier, from_here, task, base::TimeDelta(), true);
}

bool ThreadHelper::PostDelayedTask(ID identifier,
  const tracked_objects::Location& from_here,
  const base::Closure& task, base::TimeDelta delay) {
    return ThreadImpl::PostTaskHelper(identifier, from_here, task, delay, true);
}

bool ThreadHelper::PostNonNestableTask(ID identifier,
  const tracked_objects::Location& from_here,
  const base::Closure& task) {
    return ThreadImpl::PostTaskHelper(identifier, from_here, task,
      base::TimeDelta(), false);
}

bool ThreadHelper::PostNonNestedDelayedTask(ID identifier,
  const tracked_objects::Location& from_here,
  const base::Closure& task, base::TimeDelta delay) {
    return ThreadImpl::PostTaskHelper(identifier, from_here, task, delay, false);
}

bool ThreadHelper::PostTaskAndReply(ID identifier,
  const tracked_objects::Location& from_here,
  const base::Closure& task, const base::Closure& reply) {
  return GetMessageLoopProxyForThread(identifier)->PostTaskAndReply(
    from_here, task, reply);
}

scoped_refptr<base::MessageLoopProxy> ThreadHelper::GetMessageLoopProxyForThread(
  ID identifier) {
  scoped_refptr<base::MessageLoopProxy> proxy(
    new ThreadHelperMessageLoopProxy(identifier));
  return proxy;
}

bool ThreadHelper::CurrentlyOn(ID identifier) {
  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  ThreadGlobals& globals = g_globals.Get();
  base::AutoLock lock(globals.lock);
  return globals.threads[identifier]
  && globals.threads[identifier]->message_loop() == MessageLoop::current();
}

bool ThreadHelper::GetCurrentThreadIdentifier(ID* identifier) {
  if (g_globals == NULL)
    return false;

  base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
  MessageLoop* current_message_loop = MessageLoop::current();
  ThreadGlobals& globals = g_globals.Get();
  for (int i = 0; i < ID_COUNT; ++i) {
    if (globals.threads[i]
    && globals.threads[i]->message_loop() == current_message_loop) {
      *identifier = globals.threads[i]->identifier_;
      return true;
    }
  }
  return false;
}

void ThreadHelper::SetDelegate(ID identifier, ThreadDelegate* delegate) {
  using base::subtle::AtomicWord;
  ThreadGlobals& globals = g_globals.Get();
  AtomicWord* storage = reinterpret_cast<AtomicWord*>(
    &globals.thread_delegates[identifier]);
  AtomicWord old_pointer = base::subtle::NoBarrier_AtomicExchange(
    storage, reinterpret_cast<AtomicWord>(delegate));
}

SubThread::SubThread(ThreadHelper::ID identifier)
  : ThreadImpl(identifier) {}

SubThread::~SubThread() {
  Stop();
}

void SubThread::Init() {
  ThreadImpl::Init();

  if (ThreadHelper::CurrentlyOn(ThreadHelper::IO)) {
    base::ThreadRestrictions::SetIOAllowed(false);
    base::ThreadRestrictions::DisallowWaiting();
  }
}

void SubThread::CleanUp() {
  ThreadImpl::CleanUp();
}