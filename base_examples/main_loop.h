#ifndef MAIN_LOOP_H_
#define MAIN_LOOP_H_

#include "../base/basictypes.h"
#include "../base/memory/scoped_ptr.h"

class MessageLoop;
class ThreadImpl;
class SubThread;

class MainLoop {
public:
  explicit MainLoop();
  virtual ~MainLoop();

  void Init();

  void CreateThreads();

  void StartMainMessageLoop();

  void RunMainMessageLoop();

  void ShutdownThreadAndCleanUp();

  int GetResultCode() const {return result_code_;}

private:
  int result_code_;

  scoped_ptr<MessageLoop> main_message_loop_;

  scoped_ptr<ThreadImpl> main_thread_;

  scoped_ptr<SubThread> file_thread_;

  scoped_ptr<SubThread> io_thread_;

  DISALLOW_COPY_AND_ASSIGN(MainLoop);
};

#endif