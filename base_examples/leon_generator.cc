#include "leon_generator.h"
#include "../base/ipc/ipc_message.h"
#include "../base/lazy_instance.h"
#include "../base/id_map.h"
#include "../base/process.h"
#include "../base/process_util.h"
#include "child_leon_host.h"
#include "child_process_host.h"

extern base::LazyInstance<IDMap<ChildProcessHost> >::Leaky g_all_hosts;

namespace LEON {
  NewLeonParams::NewLeonParams(): child_process_id_(0) {}
  NewLeonParams::~NewLeonParams() {}

  DelLeonParams::DelLeonParams(): routing_id_(0), child_process_id_(0) {}
  DelLeonParams::~DelLeonParams() {}

  ShutdownParams::ShutdownParams(): child_process_id_(0) {}
  ShutdownParams::~ShutdownParams() {}

  void NewLeon(const NewLeonParams& params) {
    ChildProcessHost* cph = ChildProcessHost::FromID(params.child_process_id_);
    ChildLeonHost::Create(NULL, cph? cph : new ChildProcessHost, MSG_ROUTING_NONE)->NewChildLeon();
  }

  void DelLeon(const DelLeonParams& params) {
    ChildLeonHost::FromID(params.child_process_id_, params.routing_id_)->DelChildLeon();
  }

  void Shutdown(const ShutdownParams& params) {
    ChildProcessHost::FromID(params.child_process_id_)->ShutdownChildProcess();
  }

  void OnPaint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hDc;
    int stepY = 0;
    hDc = BeginPaint(hWnd, &ps);
    wchar_t text_buf[100] = {0};
    swprintf_s(text_buf, L"MainProcess:%d", base::Process::Current().pid());
    std::wstring strs(text_buf);
    TextOut(hDc, 10, 10, strs.c_str(), strs.length());
    IDMap<ChildProcessHost>::iterator iter(g_all_hosts.Pointer());
    while(!iter.IsAtEnd()) {
      memset(text_buf, 0, sizeof text_buf);
      ChildProcessHost* cph = iter.GetCurrentValue();
      IDMap<ChildLeonHost>::iterator sub_iter(&cph->ChildLeonHosts());
      wchar_t sub_text_buf[100] = {0};
      std::wstring sub_strs;
      while(!sub_iter.IsAtEnd()) {
        ChildLeonHost* clh = sub_iter.GetCurrentValue();
        memset(sub_text_buf, 0, sizeof sub_text_buf);
        swprintf_s(sub_text_buf, L"%d,", clh->GetRoutingID());
        sub_strs.append(sub_text_buf);
        sub_iter.Advance();
      }
      if (cph->GetHandle() != base::kNullProcessHandle) {
        swprintf_s(text_buf, L"ChildProcessHost: %d, ChildProcess: %d, ChildLeonHost: %s",
          cph->GetID(), base::GetProcId(cph->GetHandle()), sub_strs.c_str());
        strs = text_buf;
        stepY++;
        TextOut(hDc, 10, 10 + 30*stepY, strs.c_str(), strs.length());
      }
      iter.Advance();
    }
    EndPaint(hWnd, &ps);
  }
}