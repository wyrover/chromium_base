#ifndef SANDBOX_POLICY_H_
#define SANDBOX_POLICY_H_

#include <string>
#include "../base/process.h"
#include "../base/win/scoped_handle.h"
#include "../base/sandbox/security_level.h"

class FilePath;
class CommandLine;

class SandboxPolicy {
public:

  enum ResultCode {
    SBOX_ALL_OK = 0,
    SBOX_ERROR_GENERIC = 1
  };

  SandboxPolicy();
  ~SandboxPolicy();
  static SandboxPolicy* GetInstance();
  ResultCode MakeJobObject(HANDLE* job);

  void SetJobHandle(HANDLE job);
  ResultCode SetJobLevel(sandbox::JobLevel job_level, uint32 ui_exceptions);

  base::ProcessHandle StartProcessWithAccess(CommandLine* cmd_line,
    const FilePath& exposed_dir);

private:
  base::win::ScopedHandle job_;
  sandbox::JobLevel job_level_;
  uint32 ui_exceptions_;

  DISALLOW_COPY_AND_ASSIGN(SandboxPolicy);
};
#endif