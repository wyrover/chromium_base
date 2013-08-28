#include "main_process.h"
#include "../base/message_loop.h"
#include "../base/bind.h"
#include "../base/bind_helpers.h"
#include "../base/file_path.h"
#include "../base/path_service.h"
#include "../base/run_loop.h"
#include "../base/threading/thread_restrictions.h"
#include "result_codes.h"
#include "thread_impl.h"
#include "db_service.h"

MainProcess* g_main_process = NULL;

MainProcess::MainProcess()
  : result_code_(RESULT_CODE_NORMAL_EXIT) {
    g_main_process = this;
}

MainProcess::~MainProcess() {
  g_main_process = NULL;
}

void MainProcess::Init() {}

void MainProcess::StartMainMessageLoop() {
  if (!MessageLoop::current())
    main_message_loop_.reset(new MessageLoop(MessageLoop::TYPE_UI));

  const char* kThreadName = "UiThread";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_.get())
    main_message_loop_->set_thread_name(kThreadName);

  main_thread_.reset(new ThreadImpl(ThreadHelper::UI, MessageLoop::current()));
}

void MainProcess::CreateThreads() {

  base::Thread::Options default_options;
  base::Thread::Options io_message_loop_options;
  io_message_loop_options.message_loop_type = MessageLoop::TYPE_IO;
  base::Thread::Options ui_message_loop_options;
  ui_message_loop_options.message_loop_type = MessageLoop::TYPE_UI;

  for (size_t thread_id = ThreadHelper::UI + 1;
    thread_id < ThreadHelper::ID_COUNT; ++thread_id) {
    scoped_ptr<SubThread>* thread_to_start = NULL;
    base::Thread::Options* options = &default_options;

    switch (thread_id) {
    case ThreadHelper::FILE:
      thread_to_start = &file_thread_;
      options = &ui_message_loop_options;
      break;
    case ThreadHelper::IO:
      thread_to_start = &io_thread_;
      options = &io_message_loop_options;
      break;
    case ThreadHelper::DB:
      thread_to_start = &db_thread_;
      options = &ui_message_loop_options;
      break;
    case ThreadHelper::UI:
    default:
      NOTREACHED();
      break;
    }

    ThreadHelper::ID id = static_cast<ThreadHelper::ID>(thread_id);
    (*thread_to_start).reset(new SubThread(id));
    (*thread_to_start)->StartWithOptions(*options);
  }

  base::ThreadRestrictions::SetIOAllowed(false);
  base::ThreadRestrictions::DisallowWaiting();
}

void MainProcess::RunMainMessageLoop() {
  base::RunLoop run_loop;
  run_loop.Run();
}

void MainProcess::ShutdownThreadAndCleanUp() {
  base::ThreadRestrictions::SetIOAllowed(true);

  ThreadHelper::PostTask(ThreadHelper::IO, FROM_HERE, base::Bind(base::IgnoreResult(
    &base::ThreadRestrictions::SetIOAllowed), true));

  for (size_t thread_id = ThreadHelper::ID_COUNT - 1;
    thread_id >= ThreadHelper::UI + 1; --thread_id) {
    scoped_ptr<SubThread>* thread_to_stop = NULL;

    switch (thread_id) {
    case ThreadHelper::FILE:
      thread_to_stop = &file_thread_;
      break;
    case ThreadHelper::IO:
      thread_to_stop = &io_thread_;
      break;
    case ThreadHelper::DB:
      thread_to_stop = &db_thread_;
      break;
    case ThreadHelper::UI:
    default:
      NOTREACHED();
      break;
    }

    ThreadHelper::ID id = static_cast<ThreadHelper::ID>(thread_id);
  }
}

DBService* MainProcess::db_service() {
  if (!db_service_.get()) {
    db_service_ = new DBService();
    FilePath db_path;
    PathService::Get(base::DIR_EXE, &db_path);
    db_path = db_path.Append(L"user data");
    db_thread_->message_loop()->PostTask(FROM_HERE,
      base::Bind(&DBService::Init, db_service_.get(), db_path));
  }
  return db_service_.get();
}