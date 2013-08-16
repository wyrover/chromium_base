#include "child_thread_impl.h"

#include "../base/threading/thread_local.h"
#include "../base/lazy_instance.h"
#include "../base/ipc/ipc_switches.h"
#include "../base/command_line.h"

#include "child_process.h"
#include "from_host_messages.h"
#include "child_leon.h"

base::LazyInstance<base::ThreadLocalPointer<ChildThreadImpl> >
  lazy_tls = LAZY_INSTANCE_INITIALIZER;

ChildThreadImpl* ChildThreadImpl::current() {
  return lazy_tls.Pointer()->Get();
}

ChildThreadImpl::ChildThreadImpl()
  : ALLOW_THIS_IN_INITIALIZER_LIST(channel_connected_factory_(this)) {
  channel_name_ = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
    switches::kProcessChannelID);
  Init();
}

void ChildThreadImpl::Init() {
  channel_.reset(new IPC::SyncChannel(channel_name_, IPC::Channel::MODE_CLIENT,
    this, ChildProcess::current()->io_message_loop_proxy(), true,
    ChildProcess::current()->GetShutDownEvent()));
  lazy_tls.Pointer()->Set(this);

  ChildProcess::current()->set_main_thread(this);
}

ChildThreadImpl::~ChildThreadImpl() {
  if (file_thread_.get())
    file_thread_->Stop();
  lazy_tls.Pointer()->Set(NULL);
}

scoped_refptr<base::MessageLoopProxy> ChildThreadImpl::GetFileThreadMessageLoopProxy() {
  if (!file_thread_.get()) {
    file_thread_.reset(new base::Thread("ChildFileThread"));
    file_thread_->Start();
  }
  return file_thread_->message_loop_proxy();
}

bool ChildThreadImpl::Send(IPC::Message* message) {
  if (!channel_.get()) {
    delete message;
    return false;
  }
  return channel_->Send(message);
}

bool ChildThreadImpl::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChildThreadImpl, message)
  IPC_MESSAGE_HANDLER(FromHost_ChildLeon_New, OnChildLeonNew)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void ChildThreadImpl::OnChannelConnected(int32 peer_pid) {
  channel_connected_factory_.InvalidateWeakPtrs();
}

void ChildThreadImpl::OnChannelError() {
  MessageLoop::current()->Quit();
}

void ChildThreadImpl::OnChildLeonNew(const FromHost_ChildLeon_New_Params& params) {
  ChildLeon::Create(params.routing_id, params.name);
}