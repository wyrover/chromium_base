#ifndef THREAD_HELPER_H_
#define THREAD_HELPER_H_

#include "../base/basictypes.h"
#include "../base/callback.h"
#include "../base/location.h"

class ThreadDelegate;

class ThreadHelper {
public:
  enum ID {
    UI,

    FILE,

    IO,

    ID_COUNT
  };

  static bool PostTask(ID identifier,
    const tracked_objects::Location& from_here, const base::Closure& task);

  static bool PostDelayedTask(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task, int64 delay_ms);

  static bool PostNonNestableTask(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task);

  static bool PostNonNestedDelayedTask(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task, int64 delay_ms);

  static bool PostTaskAndReply(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task, const base::Closure& reply);

  static bool CurrentlyOn(ID identifier);

  static bool GetCurrentThreadIdentifier(ID* identifier);

  static void SetDelegate(ID identifier, ThreadDelegate* delegate);

private:
  friend class ThreadImpl;

  ThreadHelper() {}

  DISALLOW_COPY_AND_ASSIGN(ThreadHelper);
};

#endif