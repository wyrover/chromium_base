#include "child_process_host.h"

#include "../base/path_service.h"
#include "../base/ipc/ipc_switches.h"
#include "../base/ipc/ipc_sync_message.h"
#include "../base/ipc/ipc_message.h"
#include "../base/ipc/ipc_channel.h"
#include "../base/ipc/ipc_channel_proxy.h"
#include "../base/lazy_instance.h"
#include "../base/message_loop.h"

#include "message_filters.h"
#include "child_leon_host.h"
#include "to_host_messages.h"
#include "from_host_messages.h"
#include "frame_window.h"

base::LazyInstance<IDMap<ChildProcessHost> >::Leaky
  g_all_hosts = LAZY_INSTANCE_INITIALIZER;

ChildProcessHost::ChildProcessHost()
: id_(GenerateChildProcessUniqueId())
, delete_soon_(false) {
  RegisterHost(GetID(), this);
  g_all_hosts.Get().set_check_on_null_data(true);
}

ChildProcessHost::~ChildProcessHost() {
  channel_.reset();
  while(!queued_messages_.empty()) {
    delete queued_messages_.front();
    queued_messages_.pop();
  }
  UnregisterHost(GetID());
}

bool ChildProcessHost::CreateChildProcess() {
  if (channel_.get())
    return true;

  channel_name_ = IPC::Channel::GenerateVerifiedChannelID(std::string());
  channel_.reset(new IPC::ChannelProxy(channel_name_, IPC::Channel::MODE_SERVER, this,
    ThreadHelper::GetMessageLoopProxyForThread(ThreadHelper::IO)));
  channel_->AddFilter(new MessageFilter1(GetID()));
  channel_->AddFilter(new MessageFilter2(GetID()));

  CommandLine* cmd_line = new CommandLine(GetChildPath());
  cmd_line->AppendSwitchASCII(switches::kProcessChannelID, channel_name_);
  child_process_launcher_ = new ChildProcessLauncher(cmd_line, this);
  return true;
}

int ChildProcessHost::GetNextRoutingID() {
  return next_routing_id_.GetNext() + 1;
}

int ChildProcessHost::GenerateChildProcessUniqueId() {
  static base::subtle::Atomic32 last_unique_child_id = 0;
  int id = base::subtle::NoBarrier_AtomicIncrement(&last_unique_child_id, 1);
  return id;
}

FilePath ChildProcessHost::GetChildPath() {
  FilePath child_path;
  PathService::Get(base::FILE_EXE, &child_path);
  return child_path;
}

void ChildProcessHost::OnProcessLaunched() {
  if (delete_soon_)
    return;
  if (child_process_launcher_.get()) {
    if (!child_process_launcher_->GetHandle()) {
      OnChannelError();
      return;
    }
  }
  while(!queued_messages_.empty()) {
    Send(queued_messages_.front());
    queued_messages_.pop();
  }
  InvalidateRect(FrameWindow::GetInstance()->window_handle(), NULL, TRUE);
}

base::ProcessHandle ChildProcessHost::GetHandle() {
  return child_process_launcher_->GetHandle();
}

bool ChildProcessHost::Send(IPC::Message* msg) {
  if (!channel_.get()) {
    queued_messages_.push(msg);
    return true;
  }

  return channel_->Send(msg);
}

bool ChildProcessHost::OnMessageReceived(const IPC::Message& message) {
  if (delete_soon_)
    return false;
  if (message.routing_id() == MSG_ROUTING_CONTROL) {
    return true;
  }

  ChildLeonHost* clh = child_leon_hosts_.Lookup(message.routing_id());
  if (!clh) {
    if (message.is_sync()) {
      IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
      reply->set_reply_error();
      Send(reply);
    }
    bool msg_is_ok = true;
    /*IPC_BEGIN_MESSAGE_MAP_EX(ChildProcessHost, message, msg_is_ok)
      IPC_MESSAGE_HANDLER()
    IPC_END_MESSAGE_MAP_EX()*/
    return true;
  }
  return clh->OnMessageReceived(message);
}

void ChildProcessHost::OnChannelConnected(int32 peer_pid) {
}

void ChildProcessHost::OnChannelError() {
  ShutdownChildProcess();
}

void ChildProcessHost::Attach(ChildLeonHost* child_leon_host, int routing_id) {
  child_leon_hosts_.AddWithID(child_leon_host, routing_id);
}

void ChildProcessHost::Release(int routing_id) {
  child_leon_hosts_.Remove(routing_id);
  Cleanup();
}

ChildLeonHost* ChildProcessHost::GetChildLeonHostByID(int routing_id) {
  return child_leon_hosts_.Lookup(routing_id);
}

void ChildProcessHost::RegisterHost(int host_id, ChildProcessHost* host) {
  g_all_hosts.Get().AddWithID(host, host_id);
}

void ChildProcessHost::UnregisterHost(int host_id) {
  ChildProcessHost* host = g_all_hosts.Get().Lookup(host_id);
  if (!host)
    return;
  g_all_hosts.Get().Remove(host_id);
}

ChildProcessHost* ChildProcessHost::FromID(int child_process_id) {
  return g_all_hosts.Get().Lookup(child_process_id);
}

void ChildProcessHost::Cleanup() {
  if (!child_leon_hosts_.IsEmpty())
    return;
  MessageLoop::current()->DeleteSoon(FROM_HERE, this);
  delete_soon_ = true;
  channel_.reset();
  UnregisterHost(GetID());
}

void ChildProcessHost::ShutdownChildProcess() {
  IDMap<ChildLeonHost>::iterator it(&child_leon_hosts_);
  while(!it.IsAtEnd()) {
    it.GetCurrentValue()->DelChildLeon();
    it.Advance();
  }
}