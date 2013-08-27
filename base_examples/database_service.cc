#include "database_service.h"
#include "../base/sql/transaction.h"

namespace {
  static const int kCurrentVersionNumber = 48;
  static const int kCompatibleVersionNumber = 48;
}

DatabaseService::DatabaseService() {}

DatabaseService::~DatabaseService() {}

sql::InitStatus DatabaseService::Init(const FilePath& database_name,
  sql::ErrorDelegate* error_delegate) {
  db_.set_error_delegate(error_delegate);

  db_.set_page_size(4096);

  db_.set_cache_size(6000);

  if (!db_.Open(database_name))
    return sql::INIT_FAILURE;

  sql::Transaction committer(&db_);
  if (!committer.Begin())
    return sql::INIT_FAILURE;
  db_.Preload();

  if (!meta_table_.Init(&db_, kCurrentVersionNumber, kCompatibleVersionNumber))
    return sql::INIT_FAILURE;

  if (!CreateTable1() || !CreateTable2())
    return sql::INIT_FAILURE;

  return committer.Commit() ? sql::INIT_OK : sql::INIT_FAILURE;
}

bool DatabaseService::CreateTable1() {
  if (GetDB().DoesTableExist("table1"))
    return true;

  return GetDB().Execute("CREATE TABLE table1("
                         "id INTEGER PRIMARY KEY,"
                         "url LONGVARCHAR,"
                         "safe INTEGER DEFAULT 0 NOT NULL,"
                         "time INTEGER NOT NULL)");
}

bool DatabaseService::CreateTable2() {
  if (GetDB().DoesTableExist("table1"))
    return true;

  return GetDB().Execute("CREATE TABLE table1("
                         "id INTEGER PRIMARY KEY,"
                         "url LONGVARCHAR,"
                         "safe INTEGER DEFAULT 0 NOT NULL,"
                         "time INTEGER NOT NULL)");
}