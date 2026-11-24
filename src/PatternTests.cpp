#include "PatternCore.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
class FakeRepository final:public IWorkflowRepository {
public:
 int create(const WorkItem& item)override{int id=++last;WorkItem copy=item;copy.id=id;items[id]=copy;historyRows[id].push_back({0,id,"","Registered","time"});return id;}
 std::optional<WorkItem> get(int id)override{auto i=items.find(id);return i==items.end()?std::nullopt:std::optional<WorkItem>{i->second};}
 bool changeState(int id,const std::string& from,const std::string& to,bool fail)override{
  auto i=items.find(id);if(i==items.end()||i->second.status!=from)return false;
  if(fail)throw std::runtime_error("Simulated rollback");
  i->second.status=to;historyRows[id].push_back({0,id,from,to,"time"});return true;
 }
 std::vector<WorkHistory> history(int id)override{return historyRows[id];}
private:int last=0;std::map<int,WorkItem> items;std::map<int,std::vector<WorkHistory>> historyRows;
};
int main(){
 FakeRepository repo;EventPublisher bus;
 auto logger=std::make_shared<LogObserver>();auto stats=std::make_shared<StatsObserver>();
 bus.subscribe(logger);bus.subscribe(stats);bus.subscribe(logger);
 assert(bus.subscribers()==2);
 WorkflowService service(repo,bus);
 WorkItem a{};a.status="Registered";
 int id=service.registerItem(a);
 assert(repo.history(id).size()==1);
 CommandInvoker invoker;
 invoker.enqueue(std::make_unique<TransitionCommand>(service,id,WorkflowAction::Accept));
 invoker.enqueue(std::make_unique<TransitionCommand>(service,id,WorkflowAction::Finish));
 invoker.run();
 assert(repo.get(id)->status=="Completed");assert(repo.history(id).size()==3);
 bool rejected=false;
 try{service.perform(id,WorkflowAction::Cancel);}catch(const std::logic_error&){rejected=true;}
 assert(rejected);
 int second=service.registerItem(a);service.perform(second,WorkflowAction::Cancel);
 assert(repo.get(second)->status=="Cancelled");
 const auto prior=logger->lines().size();bus.unsubscribe(logger);
 service.registerItem(a);assert(logger->lines().size()==prior);
 assert(stats->count("CallRegistered")==3);
 ScoreContext ctx(std::make_unique<MaxNormalized>());ScoreInput v{7,5,3};
 assert(ctx.evaluate(v)>=0&&ctx.evaluate(v)<=1);
 ctx.setStrategy(std::make_unique<WeightedSum>());assert(ctx.evaluate(v)>0);
 ctx.setStrategy(std::make_unique<NormalizedWeightedSum>());assert(ctx.evaluate(v)>0);
 bool bad=false;try{ctx.evaluate({-1,0,0});}catch(const std::invalid_argument&){bad=true;}
 assert(bad);
 const int rollbackId=service.registerItem(a);
 bad=false;try{service.perform(rollbackId,WorkflowAction::Accept,true);}catch(const std::runtime_error&){bad=true;}
 assert(bad&&repo.get(rollbackId)->status=="Registered"&&repo.history(rollbackId).size()==1);
 std::cout<<"Pattern unit tests: PASS\n";
 return 0;
}
