#include "leon_generator.h"
#include "../base/ipc/ipc_message.h"
#include "child_leon_host.h"
#include "child_process_host.h"

namespace LEON {
  GenerateParams::GenerateParams(): create_process_(false) {}
  GenerateParams::~GenerateParams() {}

  void Generate(const GenerateParams& params) {
    if (!ThreadHelper::CurrentlyOn(ThreadHelper::IO)) {
      ThreadHelper::PostTask(ThreadHelper::IO, FROM_HERE, base::Bind(Generate, params));
      return;
    }
    ChildLeonHost* clh = ChildLeonHost::Create(
      NULL, params.create_process_? new ChildProcessHost: ChildProcessHost::GetInstance(),
      MSG_ROUTING_NONE);

    clh->NewChildLeon();
  }
}