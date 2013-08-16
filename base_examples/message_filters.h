#ifndef MESSAGE_FILTERS_H_
#define MESSAGE_FILTERS_H_

#include "../base/ipc/ipc_channel_proxy.h"
#include "thread_helper.h"

struct ToHost_Task1_Params;

class BaseMessageFilter: public IPC::ChannelProxy::MessageFilter, public IPC::Sender {
public:
  BaseMessageFilter();

  virtual void OnFilterAdded(IPC::Channel* channel) OVERRIDE;
  virtual void OnChannelClosing() OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  virtual bool Send(IPC::Message* message) OVERRIDE;
  virtual void OverrideThreadForMessage(const IPC::Message& message, ThreadHelper::ID* thread);
  virtual base::TaskRunner* OverrideTaskRunnerForMessage(const IPC::Message& message);
  virtual bool OnMessageReceived(const IPC::Message& message, bool* message_was_ok) = 0;
  virtual void BadMessageReceived();
  base::ProcessHandle peer_handle() {return peer_handle_;}
  static bool CheckCanDispatchOnUI(const IPC::Message& message, IPC::Sender* sender);

protected:
  virtual ~BaseMessageFilter();

private:
  bool DispatchMessage(const IPC::Message& message);
  IPC::Channel* channel_;
  base::ProcessHandle peer_handle_;
};

class MessageFilter1: public BaseMessageFilter {
public:
  explicit MessageFilter1(int child_process_id);

  virtual bool OnMessageReceived(const IPC::Message& message, bool* message_was_ok) OVERRIDE;

private:
  void OnTask1(const ToHost_Task1_Params& params);
  void OnTask2(const int& params);
  ~MessageFilter1();

  int child_process_id_;

  DISALLOW_COPY_AND_ASSIGN(MessageFilter1);
};

class MessageFilter2: public BaseMessageFilter {
public:
  explicit MessageFilter2(int child_process_id);

  virtual bool OnMessageReceived(const IPC::Message& message, bool* message_was_ok) OVERRIDE;

private:
  void OnTask3(const std::wstring& params);
  void OnTask4(const std::wstring& params);
  ~MessageFilter2();

  int child_process_id_;

  DISALLOW_COPY_AND_ASSIGN(MessageFilter2);
};
#endif