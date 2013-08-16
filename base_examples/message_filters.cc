#include "message_filters.h"

#include "../base/process_util.h"
#include "../base/task_runner.h"
#include "../base/memory/scoped_ptr.h"
#include "result_codes.h"
#include "to_host_messages.h"

BaseMessageFilter::BaseMessageFilter() : channel_(NULL),
  peer_handle_(base::kNullProcessHandle) {
}

void BaseMessageFilter::OnFilterAdded(IPC::Channel* channel) {
  channel_ = channel;
}

void BaseMessageFilter::OnChannelClosing() {
  channel_ = NULL;
}

void BaseMessageFilter::OnChannelConnected(int32 peer_pid) {
  if (!base::OpenProcessHandle(peer_pid, &peer_handle_)) {
    NOTREACHED();
  }
}

bool BaseMessageFilter::OnMessageReceived(const IPC::Message& message) {
  ThreadHelper::ID thread = ThreadHelper::IO;
  OverrideThreadForMessage(message, &thread);
  if (thread == ThreadHelper::IO) {
    scoped_refptr<base::TaskRunner> runner = OverrideTaskRunnerForMessage(message);
    if (runner) {
      runner->PostTask(FROM_HERE, base::Bind(
      base::IgnoreResult(&BaseMessageFilter::DispatchMessage), this, message));
      return true;
    }
    return DispatchMessage(message);
  }

  if (thread == ThreadHelper::UI && !CheckCanDispatchOnUI(message, this))
    return true;

  ThreadHelper::PostTask(thread, FROM_HERE, base::Bind(
    base::IgnoreResult(&BaseMessageFilter::DispatchMessage), this, message));
  return true;
}

bool BaseMessageFilter::Send(IPC::Message* message) {
  if (message->is_sync()) {
    NOTREACHED();
    return false;
  }
  if (!ThreadHelper::CurrentlyOn(ThreadHelper::IO)) {
    ThreadHelper::PostTask(ThreadHelper::IO, FROM_HERE, base::Bind(
      base::IgnoreResult(&BaseMessageFilter::Send), this, message));
    return true;
  }
  if (channel_)
    return channel_->Send(message);
  delete message;
  return false;
}

void BaseMessageFilter::OverrideThreadForMessage(const IPC::Message& message,
  ThreadHelper::ID* thread) {
}

base::TaskRunner* BaseMessageFilter::OverrideTaskRunnerForMessage(
  const IPC::Message& message) {
  return NULL;
}

void BaseMessageFilter::BadMessageReceived() {
  base::KillProcess(peer_handle(), RESULT_CODE_KILLED_BAD_MESSAGE, false);
}

bool BaseMessageFilter::CheckCanDispatchOnUI(const IPC::Message& message,
  IPC::Sender* sender) {
  if (message.is_sync() && !message.is_caller_pumping_messages()) {
    NOTREACHED();
    IPC::Message* reply = IPC::SyncMessage::GenerateReply(&message);
    reply->set_reply_error();
    sender->Send(reply);
    return false;
  }
  return true;
}

BaseMessageFilter::~BaseMessageFilter() {
  base::CloseProcessHandle(peer_handle_);
}

bool BaseMessageFilter::DispatchMessage(const IPC::Message& message) {
  bool message_was_ok = true;
  bool result = OnMessageReceived(message, &message_was_ok);
  if (!message_was_ok) {
    BadMessageReceived();
  }
  return result;
}

MessageFilter1::MessageFilter1(int child_process_id)
  : child_process_id_(child_process_id) {

}

MessageFilter1::~MessageFilter1() {
  DCHECK(ThreadHelper::CurrentlyOn(ThreadHelper::IO));
}

bool MessageFilter1::OnMessageReceived(const IPC::Message& message, bool* message_was_ok) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP_EX(MessageFilter1, message, *message_was_ok)
    IPC_MESSAGE_HANDLER(ToHost_Task1, OnTask1)
    IPC_MESSAGE_HANDLER(ToHost_Task2, OnTask2)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  return handled;
}

void MessageFilter1::OnTask1(const ToHost_Task1_Params& params) {

}

void MessageFilter1::OnTask2(const int& params) {

}

MessageFilter2::MessageFilter2(int child_process_id)
  : child_process_id_(child_process_id) {
}

MessageFilter2::~MessageFilter2() {
  DCHECK(ThreadHelper::CurrentlyOn(ThreadHelper::IO));
}

bool MessageFilter2::OnMessageReceived(const IPC::Message& message, bool* message_was_ok) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP_EX(MessageFilter2, message, *message_was_ok)
    IPC_MESSAGE_HANDLER(ToHost_Task3, OnTask3)
    IPC_MESSAGE_HANDLER(ToHost_Task4, OnTask4)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  return handled;
}

void MessageFilter2::OnTask3(const std::wstring& params) {

}

void MessageFilter2::OnTask4(const std::wstring& params) {

}