#include "leon_generator.h"
#include "../base/ipc/ipc_message.h"
#include "child_leon_host.h"
#include "child_process_host.h"

namespace LEON {
  GenerateParams::GenerateParams() {}
  GenerateParams::~GenerateParams() {}

  void Generate(GenerateParams* params) {
    ChildLeonHost* clh = ChildLeonHost::Create(NULL,
      ChildProcessHost::GetInstance(), MSG_ROUTING_NONE);
    clh->CreateChildLeon();
  }
}