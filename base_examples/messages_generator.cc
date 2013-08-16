// Get basic type definitions.
#define IPC_MESSAGE_IMPL
#include "messages_generator.h"

// Generate constructors.
#include "../base/ipc/struct_constructor_macros.h"
#include "messages_generator.h"

// Generate destructors.
#include "../base/ipc/struct_destructor_macros.h"
#include "messages_generator.h"

// Generate param traits write methods.
#include "../base/ipc/param_traits_write_macros.h"
namespace IPC {
#include "messages_generator.h"
}  // namespace IPC

// Generate param traits read methods.
#include "../base/ipc/param_traits_read_macros.h"
namespace IPC {
#include "messages_generator.h"
}  // namespace IPC

// Generate param traits log methods.
#include "../base/ipc/param_traits_log_macros.h"
namespace IPC {
#include "messages_generator.h"
}  // namespace IPC