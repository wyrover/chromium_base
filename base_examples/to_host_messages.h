
#include "../base/ipc/ipc_message_macros.h"

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

#define IPC_MESSAGE_START AutomationMsgStart

IPC_STRUCT_BEGIN(ToHost_Task1_Params)
IPC_STRUCT_MEMBER(std::wstring, name)
IPC_STRUCT_MEMBER(bool, actived)
IPC_STRUCT_END()

IPC_MESSAGE_ROUTED1(ToHost_Task1, ToHost_Task1_Params)
IPC_MESSAGE_ROUTED1(ToHost_Task2, int)
IPC_MESSAGE_ROUTED1(ToHost_Task3, std::wstring)
IPC_MESSAGE_ROUTED1(ToHost_Task4, std::wstring)
IPC_MESSAGE_ROUTED1(ToHost_Task5, std::wstring)
IPC_MESSAGE_ROUTED0(ToHost_ChildLeonGone)
IPC_MESSAGE_CONTROL0(ToHost_ChildProcess_ShutdownRequest)