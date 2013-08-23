#include "child_process.h"

#include "../base/threading/thread_local.h"
#include "../base/lazy_instance.h"
#include "../base/ipc/ipc_switches.h"
#include "../base/command_line.h"

#include "from_host_messages.h"
#include "to_host_messages.h"
#include "child_leon.h"

base::LazyInstance<base::ThreadLocalPointer<ChildProcess> >
  lazy_tls = LAZY_INSTANCE_INITIALIZER;

ChildProcess* ChildProcess::current() {
  return lazy_tls.Pointer()->Get();
}

ChildProcess::ChildProcess()
  : ALLOW_THIS_IN_INITIALIZER_LIST(channel_connected_factory_(this))
  , ref_count_(0)
  , shutdown_event_(true, false) {
  channel_name_ = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(
    switches::kProcessChannelID);
  Init();
}

void ChildProcess::Init() {
  channel_.reset(new IPC::SyncChannel(channel_name_, IPC::Channel::MODE_CLIENT,
    this, GetIoThreadMessageLoopProxy(), true, GetShutDownEvent()));
  lazy_tls.Pointer()->Set(this);
}

ChildProcess::~ChildProcess() {
  if (file_thread_.get())
    file_thread_->Stop();
  if (io_thread_.get())
    io_thread_->Stop();
  shutdown_event_.Signal();
  lazy_tls.Pointer()->Set(NULL);
}

scoped_refptr<base::MessageLoopProxy> ChildProcess::GetFileThreadMessageLoopProxy() {
  if (!file_thread_.get()) {
    file_thread_.reset(new base::Thread("ChildFileThread"));
    file_thread_->Start();
  }
  return file_thread_->message_loop_proxy();
}

scoped_refptr<base::MessageLoopProxy> ChildProcess::GetIoThreadMessageLoopProxy() {
  if (!io_thread_.get()) {
    io_thread_.reset(new base::Thread("ChildIoThread"));
    io_thread_->StartWithOptions(base::Thread::Options(MessageLoop::TYPE_IO, 0));
  }
  return io_thread_->message_loop_proxy();
}

bool ChildProcess::Send(IPC::Message* message) {
  if (!channel_.get()) {
    delete message;
    return false;
  }
  return channel_->Send(message);
}

bool ChildProcess::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ChildProcess, message)
  IPC_MESSAGE_HANDLER(FromHost_ChildLeon_New, OnChildLeonNew)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  if (handled)
    return true;
  ChildLeon* child_leon = ChildLeon::FromRoutingID(message.routing_id());
  return child_leon == NULL? false: child_leon->OnMessageReceived(message);
}

void ChildProcess::OnChannelConnected(int32 peer_pid) {
  channel_connected_factory_.InvalidateWeakPtrs();
}

void ChildProcess::OnChannelError() {
  MessageLoop::current()->Quit();
}

void ChildProcess::AddRefProcess() {
  ++ref_count_;
}

void ChildProcess::ReleaseProcess() {
  if (--ref_count_)
    return;
  // delete this;
}

void ChildProcess::OnChildLeonNew(const FromHost_ChildLeon_New_Params& params) {
  ChildLeon::Create(params.routing_id, params.name);
}