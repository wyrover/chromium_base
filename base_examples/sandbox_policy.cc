#include "sandbox_policy.h"

#include "../base/sandbox/security_level.h"
#include "../base/sandbox/restricted_token_utils.h"
#include "../base/sandbox/job.h"
#include "../base/win/windows_version.h"
#include "../base/win/scoped_handle.h"
#include "../base/win/startup_information.h"
#include "../base/win/scoped_process_information.h"
#include "../base/process_util.h"
#include "../base/command_line.h"
#include "../base/memory/scoped_ptr.h"

SandboxPolicy::SandboxPolicy()
  : job_level_(sandbox::JOB_LOCKDOWN) {

}

SandboxPolicy::~SandboxPolicy() {

}

SandboxPolicy* SandboxPolicy::GetInstance() {
  static SandboxPolicy* sandbox_policy = NULL;
  if (!sandbox_policy) {
    sandbox_policy = new SandboxPolicy;
    sandbox_policy->SetJobLevel(sandbox::JOB_UNPROTECTED, 0);

    HANDLE job_temp;
    ResultCode result = sandbox_policy->MakeJobObject(&job_temp);
    if (SBOX_ALL_OK != result) {
      delete sandbox_policy;
      sandbox_policy = NULL;
      return NULL;
    }
    sandbox_policy->SetJobHandle(job_temp);
  }
  return sandbox_policy;
}

void SandboxPolicy::SetJobHandle(HANDLE job) {
  job_.Set(job);
}

SandboxPolicy::ResultCode SandboxPolicy::MakeJobObject(HANDLE* job) {
  if (job_level_ != sandbox::JOB_NONE) {
    sandbox::Job job_obj;
    DWORD result = job_obj.Init(job_level_, NULL, ui_exceptions_);
    if (ERROR_SUCCESS != result)
      return SBOX_ERROR_GENERIC;
    *job = job_obj.Detach();
  } else {
    *job = NULL;
  }
  return SBOX_ALL_OK;
}

SandboxPolicy::ResultCode SandboxPolicy::SetJobLevel(sandbox::JobLevel job_level, uint32 ui_exceptions) {
  job_level_ = job_level;
  ui_exceptions_ = ui_exceptions;
  return SBOX_ALL_OK;
}

base::ProcessHandle SandboxPolicy::StartProcessWithAccess(CommandLine* cmd_line,
  const FilePath& exposed_dir) {

  DWORD flags = CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT|DETACHED_PROCESS;
  if (base::win::GetVersion() < base::win::VERSION_WIN8) {
    flags |= CREATE_BREAKAWAY_FROM_JOB;
  }
  base::win::StartupInformation startup_info;
  base::win::ScopedProcessInformation process_info;
  scoped_ptr_malloc<wchar_t> cmd_line_const_remover(_wcsdup(cmd_line->GetCommandLineString().c_str()));
  if (!::CreateProcessW(NULL, cmd_line_const_remover.get(),
    NULL, NULL, FALSE, flags, NULL, NULL, startup_info.startup_info(),
    process_info.Receive())) {
    return base::kNullProcessHandle;
  }

  if (!::AssignProcessToJobObject(job_, process_info.process_handle())) {
    ::TerminateProcess(process_info.process_handle(), 0);
    return base::kNullProcessHandle;
  }
  ResumeThread(process_info.thread_handle());
  return process_info.TakeProcessHandle();
}