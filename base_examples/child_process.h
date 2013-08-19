#ifndef CHILD_THREAD_IMPL_H_
#define CHILD_THREAD_IMPL_H_

#include "../base/ipc/ipc_channel.h"
#include "../base/ipc/ipc_sync_channel.h"
#include "../base/memory/weak_ptr.h"
#include "../base/threading/thread.h"
#include "../base/synchronization/waitable_event.h"

struct FromHost_ChildLeon_New_Params;

class ChildProcess: public IPC::Sender, public IPC::Listener {
public:
  static ChildProcess* current();

  ChildProcess();
  virtual ~ChildProcess();

  virtual bool Send(IPC::Message* message) OVERRIDE;

  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;

  virtual void OnChannelError() OVERRIDE;

  IPC::SyncChannel* channel() { return channel_.get(); }

  scoped_refptr<base::MessageLoopProxy> GetFileThreadMessageLoopProxy();

  scoped_refptr<base::MessageLoopProxy> GetIoThreadMessageLoopProxy();

  base::WaitableEvent* GetShutDownEvent() { return &shutdown_event_; }

  void OnChildLeonNew(const FromHost_ChildLeon_New_Params& params);

private:
  void Init();

  base::WaitableEvent shutdown_event_;

  scoped_ptr<base::Thread> file_thread_;

  scoped_ptr<base::Thread> io_thread_;

  std::string channel_name_;

  scoped_ptr<IPC::SyncChannel> channel_;

  base::WeakPtrFactory<ChildProcess> channel_connected_factory_;

  DISALLOW_COPY_AND_ASSIGN(ChildProcess);
};
#endif