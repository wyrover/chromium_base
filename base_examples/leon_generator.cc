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
  NewLeonParams::NewLeonParams(): create_process_(false) {}
  NewLeonParams::~NewLeonParams() {}

  DelLeonParams::DelLeonParams(): routing_id_(0) {}
  DelLeonParams::~DelLeonParams() {}

  void NewLeon(const NewLeonParams& params) {
    ChildLeonHost* clh = ChildLeonHost::Create(NULL, new ChildProcessHost, MSG_ROUTING_NONE);

    clh->NewChildLeon();
  }

  void DelLeon(const DelLeonParams& params) {
    ChildProcessHost* cph = ChildProcessHost::FromID(1);
    ChildLeonHost::FromID(cph->GetID(), params.routing_id_)->DelChildLeon();
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

      swprintf_s(text_buf, L"ChildProcessHost: %d, ChildProcess: %d, ChildLeonHost: %s",
        cph->GetID(), base::GetProcId(cph->GetHandle()), sub_strs.c_str());
      strs = text_buf;
      stepY++;
      TextOut(hDc, 10, 10 + 30*stepY, strs.c_str(), strs.length());
      iter.Advance();
    }
    EndPaint(hWnd, &ps);
  }
}