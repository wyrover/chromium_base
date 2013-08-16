#ifndef CHILD_THREAD_IMPL_H_
#define CHILD_THREAD_IMPL_H_

#include "../base/ipc/ipc_channel.h"
#include "../base/ipc/ipc_sync_channel.h"
#include "../base/memory/weak_ptr.h"
#include "../base/threading/thread.h"

struct FromHost_ChildLeon_New_Params;

class ChildThreadImpl: public IPC::Sender, public IPC::Listener {
public:
  static ChildThreadImpl* current();

  ChildThreadImpl();
  virtual ~ChildThreadImpl();

  virtual bool Send(IPC::Message* message) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual void OnChannelError() OVERRIDE;

  IPC::SyncChannel* channel() { return channel_.get(); }

  scoped_refptr<base::MessageLoopProxy> GetFileThreadMessageLoopProxy();

  void OnChildLeonNew(const FromHost_ChildLeon_New_Params& params);

private:
  void Init();
  scoped_ptr<base::Thread> file_thread_;
  std::string channel_name_;
  scoped_ptr<IPC::SyncChannel> channel_;
  base::WeakPtrFactory<ChildThreadImpl> channel_connected_factory_;
  DISALLOW_COPY_AND_ASSIGN(ChildThreadImpl);
};
#endif