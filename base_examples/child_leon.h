#ifndef CHILD_LEON_H_
#define CHILD_LEON_H_

#include "../base/ipc/ipc_channel.h"
#include "../base/ipc/ipc_listener.h"

class ChildLeon: public IPC::Listener, public IPC::Sender {
public:
  static ChildLeon* Create(int routing_id, const std::wstring& name);

  static ChildLeon* FromRoutingID(int routing_id);

  int routing_id() { return routing_id_; }

  virtual bool Send(IPC::Message* message) OVERRIDE;

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  virtual void Close();

protected:
  explicit ChildLeon(int routing_id, const std::wstring& name);
  virtual ~ChildLeon();

private:
  int routing_id_;

  std::wstring name_;

  bool closing_;

  DISALLOW_COPY_AND_ASSIGN(ChildLeon);
};
#endif