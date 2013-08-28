#include "db_service.h"
#include "../base/sql/transaction.h"
#include "../base/sql/statement.h"

namespace {
  static const int kCurrentVersionNumber = 48;
  static const int kCompatibleVersionNumber = 48;
}

DBService::LeonRow::LeonRow() {
  id_ = 0;
  process_id_ = 0;
  routing_id_ = 0;
  created_time_ = base::Time();
}

DBService::LeonRow::~LeonRow() {}

DBService::LeonRow& DBService::LeonRow::operator=(const DBService::LeonRow& other) {
  id_ = other.id_;
  pipe_name_ = other.pipe_name_;
  process_id_ = other.process_id_;
  routing_id_ = other.routing_id_;
  created_time_ = other.created_time_;

  return *this;
}

DBService::DBService() {}

DBService::~DBService() {
  CloseDatabase();
}

void DBService::Init(const FilePath& database_name) {
  db_.reset(new sql::Connection());
  db_->set_page_size(4096);
  db_->set_cache_size(6000);

  if (db_->Open(database_name)) {

    sql::Transaction committer(db_.get());
    if (!committer.Begin())
      goto Cleanup;
    db_->Preload();
    if (!meta_table_.Init(db_.get(), kCurrentVersionNumber, kCompatibleVersionNumber))
      goto Cleanup;
    if (!CreateLeonTable())
      goto Cleanup;
    if (!committer.Commit())
      goto Cleanup;
    db_->Execute("PRAGMA locking_mode=EXCLUSIVE");
    db_->BeginTransaction();
    return;
  }
Cleanup:
  db_->BeginTransaction();
  CloseDatabase();
}

void DBService::CloseDatabase() {
  if (db_.get()) {
    db_->CommitTransaction();
    db_.reset();
  }
}

bool DBService::CreateLeonTable() {
  if (GetDB()->DoesTableExist(TABLE_NAME))
    return true;

  return GetDB()->Execute("CREATE TABLE " TABLE_NAME "("
                         "id INTEGER PRIMARY KEY,"
                         "pipe_name LONGVARCHAR,"
                         "process_id INTEGER DEFAULT 0 NOT NULL,"
                         "routing_id INTEGER DEFAULT 0 NOT NULL,"
                         "created_time INTEGER NOT NULL)");
}

bool DBService::GetLeonRowById(DBService::ID id, DBService::LeonRow* info) {
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE,
    "SELECT" ROW_FIELDS "FROM" TABLE_NAME "WHERE id=?"));
  statement.BindInt64(0, id);
  if (statement.Step()) {
    FillRow(statement, info);
    return true;
  }
  return false;
}

bool DBService::GetAllLeonRows(DBService::LeonRows* infos) {
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE,
    "SELECT" ROW_FIELDS "FROM" TABLE_NAME));
  while(statement.Step()) {
    LeonRow info;
    FillRow(statement, &info);
    infos->push_back(info);
  }
  return true;
}

DBService::ID DBService::GetLeonRowForName(const std::string& name, DBService::LeonRow* info) {
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE,
    "SELECT" ROW_FIELDS "FROM" TABLE_NAME "WHERE pipe_name=?"));
  statement.BindString(0, name);
  if (statement.Step()) {
    if (info)
      FillRow(statement, info);
    return statement.ColumnInt64(0);
  }
  return 0;
}

bool DBService::UpdateRow(DBService::ID id, const DBService::LeonRow& info) {
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE,
    "UPDATE " TABLE_NAME "SET pipe_name=?, process_id=?, routing_id=?, time=? WHERE id=?"));
  statement.BindString(0, info.pipe_name());
  statement.BindInt(1, info.process_id());
  statement.BindInt(2, info.routing_id());
  statement.BindInt64(3, info.created_time().ToInternalValue());
  statement.BindInt64(4, id);

  return statement.Run();
}

DBService::ID DBService::AddRow(const LeonRow& info) {
  std::string sql("INSERT INTO " TABLE_NAME "(pipe_name, process_id, routing_id, created_time) VALUES(?, ?, ?, ?)");
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
  statement.BindString(0, info.pipe_name());
  statement.BindInt(1, info.process_id());
  statement.BindInt(2, info.routing_id());
  statement.BindInt64(3, info.created_time().ToInternalValue());

  if (!statement.Run())
    return 0;
  return GetDB()->GetLastInsertRowId();
}

bool DBService::DeleteRow(DBService::ID id) {
  sql::Statement statement(GetDB()->GetCachedStatement(SQL_FROM_HERE,
    "DELETE FROM " TABLE_NAME "WHERE id=?"));
  statement.BindInt64(0, id);
  return statement.Run();
}

void DBService::FillRow(sql::Statement& s, DBService::LeonRow* i) {
  i->id_ = s.ColumnInt64(0);
  i->pipe_name_ = s.ColumnString(1);
  i->process_id_ = s.ColumnInt(2);
  i->routing_id_ = s.ColumnInt(3);
  i->created_time_ = base::Time::FromInternalValue(s.ColumnInt64(4));
}