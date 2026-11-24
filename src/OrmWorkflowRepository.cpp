#include "OrmWorkflowRepository.h"
#include <sqlite3.h>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {
std::string nowUtc() {
    const auto timestamp = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &timestamp);
#else
    gmtime_r(&timestamp, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}
}

OrmWorkflowRepository::OrmWorkflowRepository(const std::string& databasePath):storage_(createStorage(databasePath)){
 storage_.on_open=[](sqlite3* db){
  if(sqlite3_exec(db,"PRAGMA foreign_keys=ON;",nullptr,nullptr,nullptr)!=SQLITE_OK)
   throw std::runtime_error("Cannot enable SQLite foreign keys");
 };
 storage_.open_forever();
 storage_.sync_schema();
}
int OrmWorkflowRepository::create(const WorkItem& input){
 auto parent=storage_.get_optional<CallOperator>(input.operatorId);
 if(!parent)throw std::invalid_argument("Operator not found");
 int id=0;
 const bool ok=storage_.transaction([&](){
  WorkItem item=input;
  id=storage_.insert(item);
  storage_.insert(WorkHistory{0,id,"","Registered",nowUtc()});
  return true;
 });
 if(!ok)throw std::runtime_error("Failed to create entity");
 return id;
}
std::optional<WorkItem> OrmWorkflowRepository::get(int id){return storage_.get_optional<WorkItem>(id);}
bool OrmWorkflowRepository::changeState(int id,const std::string& from,const std::string& to,bool injectFailure){
 bool changed=false;
 storage_.transaction([&](){
  auto item=storage_.get_optional<WorkItem>(id);
  if(!item||item->status!=from)return false;

  item->status=to;
  storage_.update(*item);
  if(injectFailure)throw std::runtime_error("Injected transaction error");
  storage_.insert(WorkHistory{0,id,from,to,nowUtc()});
  changed=true;
  return true;
 });
 return changed;
}
std::vector<WorkHistory> OrmWorkflowRepository::history(int id){
 using namespace sqlite_orm;
 return storage_.get_all<WorkHistory>(where(c(&WorkHistory::callId)==id),order_by(&WorkHistory::id).asc());
}
