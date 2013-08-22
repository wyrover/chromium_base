#include "child_process_launcher.h"
#include "../base/bind.h"
#include "../base/process.h"
#include "../base/process_util.h"
#include "result_codes.h"
#include "sandbox_policy.h"

ChildProcessLauncher::ChildProcessLauncher( CommandLine* cmdline, Observer* observer)
  : observer_(observer)
  , observer_thread_id_(ThreadHelper::UI) {
  DCHECK(observer);
  Launch(cmdline);
}

ChildProcessLauncher::~ChildProcessLauncher() {
  Terminate();
}

base::ProcessHandle ChildProcessLauncher::GetHandle() {
  return child_process_.handle();
}

void ChildProcessLauncher::Launch(CommandLine* cmdline) {
  ThreadHelper::PostTask(ThreadHelper::FILE, FROM_HERE, base::Bind(
    &ChildProcessLauncher::LaunchInternal, make_scoped_refptr(this),
    observer_thread_id_, cmdline));
}

void ChildProcessLauncher::LaunchInternal(
  scoped_refptr<ChildProcessLauncher> this_object,
  ThreadHelper::ID observer_thread_id,
  CommandLine* cmd_line) {
    scoped_ptr<CommandLine> cmd_line_deleter(cmd_line);
    base::ProcessHandle handle =
      SandboxPolicy::GetInstance()->StartProcessInJob(cmd_line);

    ThreadHelper::PostTask(observer_thread_id, FROM_HERE, base::Bind(
      &ChildProcessLauncher::Notify, this_object.get(), handle));
}

void ChildProcessLauncher::Notify(base::ProcessHandle handle) {
  child_process_.set_handle(handle);
  observer_->OnProcessLaunched();
}

void ChildProcessLauncher::Terminate() {
  if (!child_process_.handle())
    return;
  ThreadHelper::PostTask(ThreadHelper::FILE, FROM_HERE, base::Bind(
    &ChildProcessLauncher::TerminateInternal, child_process_.handle()));
  child_process_.set_handle(base::kNullProcessHandle);
}

void ChildProcessLauncher::TerminateInternal(base::ProcessHandle handle) {
  base::Process process(handle);
  process.Terminate(RESULT_CODE_NORMAL_EXIT);
  process.Close();
}