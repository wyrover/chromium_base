#ifndef DATABASE_SERVICE_H_
#define DATABASE_SERVICE_H_

#include "../base/memory/ref_counted.h"
#include "../base/sql/connection.h"
#include "../base/sql/init_status.h"
#include "../base/sql/meta_table.h"
#include "../base/memory/scoped_ptr.h"

class DatabaseService {
public:
  DatabaseService();

  virtual ~DatabaseService();

  class TransactionScoper {
  public:
    explicit TransactionScoper(sql::Connection* db) : db_(db) {
      db_->BeginTransaction();
    }
    ~TransactionScoper() {
      db_->CommitTransaction();
    }
  private:
    sql::Connection* db_;
  };

  sql::Connection& GetDB() { return db_; };

  sql::MetaTable& GetMetaTable();

  sql::InitStatus Init(const FilePath& database_name,
    sql::ErrorDelegate* error_delegate);

  bool CreateTable1();

  bool CreateTable2();

private:
  sql::Connection db_;
  sql::MetaTable meta_table_;

  DISALLOW_COPY_AND_ASSIGN(DatabaseService);
};
#endif