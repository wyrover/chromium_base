#ifndef CHILD_PROCESS_LAUNCHER_H_
#define CHILD_PROCESS_LAUNCHER_H_

#include "../base/memory/ref_counted.h"
#include "../base/file_path.h"
#include "../base/command_line.h"
#include "../base/process.h"
#include "thread_helper.h"

class ChildProcessLauncher: public base::RefCountedThreadSafe<ChildProcessLauncher> {
public:
  class Observer {
  public:
    virtual void OnProcessLaunched() = 0;
  protected:
    virtual ~Observer() {}
  };

  explicit ChildProcessLauncher(const FilePath& exposed_dir,
    CommandLine* cmdline, Observer* observer);
  ~ChildProcessLauncher();

  bool IsStarting();

  base::ProcessHandle GetHandle();

  void Launch(const FilePath& exposed_dir, CommandLine* cmdline);

  void Terminate();

private:
  void Notify(base::ProcessHandle handle);

  static void LaunchInternal(scoped_refptr<ChildProcessLauncher> this_object,
    ThreadHelper::ID observer_thread_id, const FilePath& exposed_dir, CommandLine* cmd_line);

  static void TerminateInternal(base::ProcessHandle handle);

  Observer* observer_;

  ThreadHelper::ID observer_thread_id_;

  base::Process process_;

  bool starting_;

  DISALLOW_COPY_AND_ASSIGN(ChildProcessLauncher);
};
#endif