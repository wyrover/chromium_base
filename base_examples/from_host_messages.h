
#include "../base/ipc/ipc_message_macros.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#define IPC_MESSAGE_START ViewMsgStart

IPC_STRUCT_BEGIN(FromHost_ChildLeon_New_Params)
IPC_STRUCT_MEMBER(std::wstring, name)
IPC_STRUCT_MEMBER(int, routing_id)
IPC_STRUCT_END()

IPC_MESSAGE_ROUTED1(FromHost_ChildLeon_New, FromHost_ChildLeon_New_Params)
IPC_MESSAGE_ROUTED0(FromHost_ChildLeon_Del)