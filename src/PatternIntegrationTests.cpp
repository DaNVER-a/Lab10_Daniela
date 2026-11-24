#include "OrmWorkflowRepository.h"
#include "PatternCore.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main(){
 const std::string file="lab10_callcenter_integration_test.db";
 std::filesystem::remove(file);
 auto storage=createStorage(file);
 storage.sync_schema();
 int parentId=storage.insert(CallOperator{0,"Test","morning","active","101"});
 OrmWorkflowRepository repo(file);EventPublisher bus;
 WorkflowService service(repo,bus);
 int id=service.registerItem(WorkItem{0,parentId,"2026-11-23 10:00","TEST-001",60,"resolved","Registered"});
 assert(repo.get(id)->status=="Registered"&&repo.history(id).size()==1);
 service.perform(id,WorkflowAction::Accept);
 assert(repo.get(id)->status=="Accepted"&&repo.history(id).size()==2);
 bool rollbackCaught=false;
 try{service.perform(id,WorkflowAction::Finish,true);}catch(const std::runtime_error&){rollbackCaught=true;}
 assert(rollbackCaught&&repo.get(id)->status=="Accepted"&&repo.history(id).size()==2);
 service.perform(id,WorkflowAction::Finish);
 assert(repo.get(id)->status=="Completed"&&repo.history(id).size()==3);

 bool invalid=false;try{service.perform(id,WorkflowAction::Accept);}catch(const std::logic_error&){invalid=true;}
 assert(invalid);
 std::cout<<"SQLite/ORM integration tests: PASS\n";
 return 0;
}
