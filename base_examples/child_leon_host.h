#ifndef CHILD_LEON_HOST_H_
#define CHILD_LEON_HOST_H_

#include "../base/observer_list.h"
#include "../base/ipc/ipc_channel.h"
#include "../base/ipc/ipc_channel_proxy.h"
#include "../base/ipc/ipc_message.h"

class ChildProcessHost;

class ChildLeonHost: public IPC::Sender, public IPC::Listener {
public:
  static ChildLeonHost* FromID(int child_process_id, int child_leon_id);
  static ChildLeonHost* From(ChildLeonHost* clh);

  class Delegate {
  public:
    // give the delegate a chance to filter ipc message
    virtual bool OnMessageReceived(ChildLeonHost* child_leon_host,
      const IPC::Message& message);
  protected:
    virtual ~Delegate() {}
  };

  class Observer: public IPC::Sender, public IPC::Listener {
  public:
    explicit Observer(ChildLeonHost* child_leon_host);
    virtual ~Observer();
    virtual void ChildLeonHostInitialized();
    virtual void ChildLeonHostDestroyed();
    virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
    virtual bool Send(IPC::Message* msg) OVERRIDE;
    ChildLeonHost* child_leon_host() const;
  private:
    ChildLeonHost* child_leon_host_;
  };

  ChildLeonHost(Delegate* delegate, ChildProcessHost* child_process_host, int routing_id);
  virtual ~ChildLeonHost();
  static ChildLeonHost* Create(Delegate* delegate, ChildProcessHost* child_process_host, int routing_id);
  bool CreateChildLeon();
  virtual bool Send(IPC::Message* msg) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual int GetRoutingID() const { return routing_id_; };
  virtual Delegate* GetDelegate() { return delegate_; }

protected:
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

private:
  void OnTask5(const std::wstring& params);
  ChildProcessHost* child_process_host_;
  int routing_id_;
  Delegate* delegate_;
  ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(ChildLeonHost);
};
#endif