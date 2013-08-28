#ifndef DB_SERVICE_H_
#define DB_SERVICE_H_

#include <vector>

#include "../base/memory/ref_counted.h"
#include "../base/sql/connection.h"
#include "../base/sql/init_status.h"
#include "../base/sql/meta_table.h"
#include "../base/memory/scoped_ptr.h"

#define TABLE_NAME "leon"
#define ROW_FIELDS \
  "leon.id, leon.pipe_name, leon.process_id, leon.routing_id, leon.created_time"

class DBService: public base::RefCountedThreadSafe<DBService> {
public:
  DBService();

  virtual ~DBService();

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

  typedef int64 ID;
  
  class LeonRow {
  public:
    LeonRow();

    virtual ~LeonRow();

    LeonRow& operator=(const LeonRow& other);

    ID id() const { return id_; }

    void set_id(ID id) { id_ = id; }

    const std::string& pipe_name() const { return pipe_name_; }

    void set_pipe_name(const std::string& name) { pipe_name_ = name; }

    int process_id() const { return process_id_; }

    void set_process_id(int id) { process_id_ = id; }

    int routing_id() const { return routing_id_; }

    void set_routing_id(int id) { routing_id_ = id; }

    base::Time created_time() const { return created_time_; }

    void set_created_time(base::Time time = base::Time::Now()) { created_time_ = time; }

  private:
    friend class DBService;

    ID id_;

    std::string pipe_name_;

    int process_id_;

    int routing_id_;

    base::Time created_time_;
  };
  typedef std::vector<LeonRow> LeonRows;

  bool GetLeonRowById(ID id, LeonRow* info);

  bool GetAllLeonRows(LeonRows* infos);

  ID GetLeonRowForName(const std::string& name, LeonRow* info);

  bool UpdateRow(ID id, const LeonRow& info);

  ID AddRow(const LeonRow& info);

  bool DeleteRow(ID id);

  sql::Connection* GetDB() { return db_.get(); };

  sql::MetaTable& GetMetaTable() { return meta_table_; }

  void Init(const FilePath& database_name);

  void CloseDatabase();

  bool CreateLeonTable();

private:
  static void FillRow(sql::Statement& s, LeonRow* i);
  scoped_ptr<sql::Connection> db_;
  sql::MetaTable meta_table_;

  DISALLOW_COPY_AND_ASSIGN(DBService);
};
#endif