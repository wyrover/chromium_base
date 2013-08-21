#include "child_leon.h"

#include <map>
#include "../base/lazy_instance.h"
#include "from_host_messages.h"

#include "child_process.h"

typedef std::map<int, ChildLeon*> RoutingIDLeonMap;
static base::LazyInstance<RoutingIDLeonMap> g_routing_id_leon_map =
  LAZY_INSTANCE_INITIALIZER;

ChildLeon* ChildLeon::Create(int routing_id, const std::wstring& name) {
  return new ChildLeon(routing_id, name);
}

ChildLeon* ChildLeon::FromRoutingID(int routing_id) {
  RoutingIDLeonMap::iterator it = g_routing_id_leon_map.Pointer()->find(routing_id);
  return it == g_routing_id_leon_map.Pointer()->end()? NULL: it->second;
}

ChildLeon::ChildLeon(int routing_id, const std::wstring& name)
  : routing_id_(routing_id)
  , closing_(false)
  , name_(name) {
  g_routing_id_leon_map.Get().insert(std::make_pair(routing_id_, this));
  ChildProcess::current()->AddRefProcess();
}

ChildLeon::~ChildLeon() {
  ChildProcess::current()->ReleaseProcess();
}

bool ChildLeon::Send(IPC::Message* message) {
  if (closing_) {
    delete message;
    return false;
  }
  if (message->routing_id() == MSG_ROUTING_NONE)
    message->set_routing_id(routing_id_);
  return ChildProcess::current()->Send(message);
}

bool ChildLeon::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  bool msg_is_ok = false;
  IPC_BEGIN_MESSAGE_MAP_EX(ChildLeon, message, msg_is_ok)
    IPC_MESSAGE_HANDLER(FromHost_ChildLeon_Del, OnChildLeonDel)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP_EX()

  return handled;
}

void ChildLeon::OnChildLeonDel() {
  g_routing_id_leon_map.Get().erase(routing_id_);
  delete this;
}