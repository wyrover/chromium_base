#include "child_leon_host.h"

#include "child_process_host.h"
#include "message_filters.h"
#include "from_host_messages.h"
#include "to_host_messages.h"

bool ChildLeonHost::Delegate::OnMessageReceived(ChildLeonHost* child_leon_host,
  const IPC::Message& message) {
    return false;
}

ChildLeonHost::Observer::Observer(ChildLeonHost* child_leon_host)
  : child_leon_host_(child_leon_host) {
    child_leon_host->AddObserver(this);
}

ChildLeonHost::Observer::~Observer() {
  if (child_leon_host_)
    child_leon_host_->RemoveObserver(this);
}

void ChildLeonHost::Observer::ChildLeonHostInitialized() {
}

void ChildLeonHost::Observer::ChildLeonHostDestroyed() {
  delete this;
}

bool ChildLeonHost::Observer::OnMessageReceived(const IPC::Message& message) {
  return false;
}

bool ChildLeonHost::Observer::Send(IPC::Message* msg) {
  if (!child_leon_host_) {
    delete msg;
    return false;
  }
  return child_leon_host_->Send(msg);
}

ChildLeonHost::ChildLeonHost(Delegate* delegate,
  ChildProcessHost* child_process_host, int routing_id)
  : delegate_(delegate)
  , child_process_host_(child_process_host)
  , routing_id_(routing_id) {
  if (routing_id_ == MSG_ROUTING_NONE) {
    routing_id_ = child_process_host_->GetNextRoutingID();
  }
  child_process_host_->Attach(this, routing_id_);
}

ChildLeonHost* ChildLeonHost::FromID(int child_process_id, int child_leon_id) {
  ChildProcessHost* process = ChildProcessHost::FromID(child_process_id);
  if (!process)
    return NULL;
  ChildLeonHost* clh = process->GetChildLeonHostByID(child_leon_id);
  return clh;
}

ChildLeonHost* ChildLeonHost::From(ChildLeonHost* clh) {
  return clh;
}

ChildLeonHost::~ChildLeonHost() {
  FOR_EACH_OBSERVER(ChildLeonHost::Observer, observers_, ChildLeonHostDestroyed());
  child_process_host_->Release(routing_id_);
}

ChildLeonHost* ChildLeonHost::Create(Delegate* delegate,
  ChildProcessHost* child_process_host, int routing_id) {
  return new ChildLeonHost(delegate, child_process_host, routing_id);
}

bool ChildLeonHost::NewChildLeon() {
  if (!child_process_host_->CreateChildProcess())
    return false;
  FromHost_ChildLeon_New_Params params;
  params.name = L"leon";
  params.routing_id = routing_id_;
  Send(new FromHost_ChildLeon_New(routing_id_, params));
  FOR_EACH_OBSERVER(ChildLeonHost::Observer, observers_, ChildLeonHostInitialized());
  return true;
}

void ChildLeonHost::DelChildLeon() {
  if (child_process_host_->HasConnection())
    Send(new FromHost_ChildLeon_Del(routing_id_));
  delete this;
}

bool ChildLeonHost::Send(IPC::Message* msg) {
  return child_process_host_->Send(msg);
}

bool ChildLeonHost::OnMessageReceived(const IPC::Message& message) {
  if (!BaseMessageFilter::CheckCanDispatchOnUI(message, this))
    return true;

  ObserverListBase<Observer>::Iterator it(observers_);
  Observer* observer;
  while((observer = it.GetNext()) != NULL) {
    if (observer->OnMessageReceived(message))
      return true;
  }
  if (delegate_ && delegate_->OnMessageReceived(this, message))
    return true;

  bool handled = true;
  bool msg_is_ok = true;
  IPC_BEGIN_MESSAGE_MAP_EX(ChildLeonHost, message, msg_is_ok)
  IPC_MESSAGE_HANDLER(ToHost_Task5, OnTask5)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  return handled;
}

void ChildLeonHost::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ChildLeonHost::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ChildLeonHost::OnTask5(const std::wstring& params) {
}