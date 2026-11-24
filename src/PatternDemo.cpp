#include "PatternDemo.h"
#include "OrmWorkflowRepository.h"
#include "PatternCore.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>

void runPatternDemo(){
 const std::string file="lab10_callcenter_patterns.db";
 std::filesystem::remove(file);

auto storage=createStorage(file);
storage.sync_schema();
int parentId=storage.insert(CallOperator{0,"Daniela Verhunenko","morning","active","101"});

 OrmWorkflowRepository repo(file);
 EventPublisher events;
 auto log=std::make_shared<LogObserver>();
 auto stats=std::make_shared<StatsObserver>();
 events.subscribe(log);events.subscribe(stats);
 WorkflowService service(repo,events);
 const int first=service.registerItem(WorkItem{0,parentId,"2026-11-23 09:00","TEST-101",120,"resolved","Registered"});
 CommandInvoker commands;
 commands.enqueue(std::make_unique<TransitionCommand>(service,first,WorkflowAction::Accept));
 commands.enqueue(std::make_unique<TransitionCommand>(service,first,WorkflowAction::Finish));
 commands.run();
 const int second=service.registerItem(WorkItem{0,parentId,"2026-11-23 09:10","TEST-102",90,"missed","Registered"});
 service.perform(second,WorkflowAction::Cancel);
 events.unsubscribe(stats);
 const int third=service.registerItem(WorkItem{0,parentId,"2026-11-23 09:10","TEST-102",90,"missed","Registered"});
 auto score=ScoreContext(std::make_unique<MaxNormalized>());
 ScoreInput input=ScoreInput{7.0,9.0,180.0};
 std::cout<<"\n=== LAB10 PATTERNS: CallCenter ===\n";
 std::cout<<std::fixed<<std::setprecision(4);
 std::cout<<"Strategy "<<score.algorithm()<<": "<<score.evaluate(input)<<'\n';
 score.setStrategy(std::make_unique<WeightedSum>());
 std::cout<<"Strategy "<<score.algorithm()<<": "<<score.evaluate(input)<<'\n';
 score.setStrategy(std::make_unique<NormalizedWeightedSum>());
 std::cout<<"Strategy "<<score.algorithm()<<": "<<score.evaluate(input)<<'\n';
 std::cout<<"State item "<<first<<": "<<repo.get(first)->status<<" | history="<<repo.history(first).size()<<'\n';
 std::cout<<"State item "<<second<<": "<<repo.get(second)->status<<" | history="<<repo.history(second).size()<<'\n';
 std::cout<<"State item "<<third<<": "<<repo.get(third)->status<<'\n';
  for(const auto& history:repo.history(first)) {
    std::cout << "History: " << history.previousStatus << " -> "
              << history.newStatus << " at " << history.changedAt << '\n';
  }
  bool forbidden=false;
  try{service.perform(first,WorkflowAction::Accept);}
  catch(const std::logic_error&){forbidden=true;}
  std::cout<<"Invalid transition blocked: "<<std::boolalpha<<forbidden<<'\n';
  if(!forbidden)throw std::runtime_error("Invalid state transition accepted");
  
 std::cout<<"Observer log entries="<<log->lines().size()<<" | active observers="<<events.subscribers()<<'\n';
 std::cout<<"Stats registered="<<stats->count("CallRegistered")<<" (unsubscribed before third registration)\n";
 for(const auto& entry:log->lines())std::cout<<"Event: "<<entry<<'\n';
 std::cout<<"Patterns demo: PASS\n";
}
