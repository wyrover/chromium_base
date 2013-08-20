#include "child_process_launcher.h"
#include "../base/bind.h"
#include "../base/process.h"
#include "../base/process_util.h"
#include "result_codes.h"
#include "sandbox_policy.h"

ChildProcessLauncher::ChildProcessLauncher(const FilePath& exposed_dir,
  CommandLine* cmdline, Observer* observer)
  : observer_(observer)
  , starting_(true)
  , observer_thread_id_(ThreadHelper::UI) {
  Launch(exposed_dir, cmdline);
}

ChildProcessLauncher::~ChildProcessLauncher() {
  Terminate();
}

bool ChildProcessLauncher::IsStarting() {
  return starting_;
}

base::ProcessHandle ChildProcessLauncher::GetHandle() {
  return process_.handle();
}

void ChildProcessLauncher::Launch(const FilePath& exposed_dir, CommandLine* cmdline) {
  ThreadHelper::PostTask(ThreadHelper::FILE, FROM_HERE, base::Bind(
    &ChildProcessLauncher::LaunchInternal, make_scoped_refptr(this),
    observer_thread_id_, exposed_dir, cmdline));
}

void ChildProcessLauncher::LaunchInternal(
  scoped_refptr<ChildProcessLauncher> this_object,
  ThreadHelper::ID observer_thread_id,
  const FilePath& exposed_dir,
  CommandLine* cmd_line) {
    scoped_ptr<CommandLine> cmd_line_deleter(cmd_line);
    base::ProcessHandle handle = base::kNullProcessHandle;
    handle = SandboxPolicy::GetInstance()->StartProcessWithAccess(cmd_line, exposed_dir);

    ThreadHelper::PostTask(observer_thread_id, FROM_HERE, base::Bind(
      &ChildProcessLauncher::Notify, this_object.get(), handle));
}

void ChildProcessLauncher::Notify(base::ProcessHandle handle) {
  starting_ = false;
  process_.set_handle(handle);
  if (observer_)
    observer_->OnProcessLaunched();
  else
    Terminate();
}

void ChildProcessLauncher::Terminate() {
  if (!process_.handle())
    return;
  ThreadHelper::PostTask(ThreadHelper::FILE, FROM_HERE, base::Bind(
    &ChildProcessLauncher::TerminateInternal, process_.handle()));
  process_.set_handle(base::kNullProcessHandle);
}

void ChildProcessLauncher::TerminateInternal(base::ProcessHandle handle) {
  base::Process process(handle);
  process.Terminate(RESULT_CODE_NORMAL_EXIT);
  process.Close();
}