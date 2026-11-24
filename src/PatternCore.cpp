#include "PatternCore.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {
class InitialState final:public IWorkflowState {
public: std::string name() const override{return "Registered";}
std::string next(WorkflowAction a) const override{
 if(a==WorkflowAction::Accept)return "Accepted";
 if(a==WorkflowAction::Cancel)return "Cancelled";
 throw std::logic_error("Invalid transition from Registered");
}
};
class InProgressState final:public IWorkflowState {
public: std::string name() const override{return "Accepted";}
std::string next(WorkflowAction a) const override{
 if(a==WorkflowAction::Finish)return "Completed";
 if(a==WorkflowAction::Cancel)return "Cancelled";
 throw std::logic_error("Invalid transition from Accepted");
}
};
class FinishedState final:public IWorkflowState {
public: std::string name() const override{return "Completed";}
std::string next(WorkflowAction) const override{throw std::logic_error("Terminal state Completed");}
};
class CancelledState final:public IWorkflowState {
public: std::string name() const override{return "Cancelled";}
std::string next(WorkflowAction) const override{throw std::logic_error("Terminal state Cancelled");}
};
double normalized(double number,double denominator){
 if(!std::isfinite(number)||number<0)throw std::invalid_argument("Score inputs must be nonnegative and finite");
 return std::clamp(number/denominator,0.0,1.0);
}
}
std::unique_ptr<IWorkflowState> makeWorkflowState(const std::string& state){
 if(state=="Registered")return std::make_unique<InitialState>();
 if(state=="Accepted")return std::make_unique<InProgressState>();
 if(state=="Completed")return std::make_unique<FinishedState>();
 if(state=="Cancelled")return std::make_unique<CancelledState>();
 throw std::invalid_argument("Unknown workflow state: "+state);
}
std::string MaxNormalized::name() const{return "Max normalized";}
std::string WeightedSum::name() const{return "Weighted sum";}
std::string NormalizedWeightedSum::name() const{return "Normalized weighted sum";}
double MaxNormalized::calculate(const ScoreInput& v)const{
 auto [x,y,z]=v;
 return std::max({normalized(x,10.0),normalized(y,10.0),normalized(z,600.0)});
}
double WeightedSum::calculate(const ScoreInput& v)const{
 for(double n:{v.first,v.second,v.third})if(!std::isfinite(n)||n<0)throw std::invalid_argument("Invalid score");
 return .50*v.first+.30*v.second+.20*v.third;
}
double NormalizedWeightedSum::calculate(const ScoreInput& v)const{
 return .50*normalized(v.first,10.0)+.30*normalized(v.second,10.0)+.20*normalized(v.third,600.0);
}
ScoreContext::ScoreContext(std::unique_ptr<IScoreStrategy> s){setStrategy(std::move(s));}
void ScoreContext::setStrategy(std::unique_ptr<IScoreStrategy> s){
 if(!s)throw std::invalid_argument("Null strategy");
 current_=std::move(s);
}
double ScoreContext::evaluate(const ScoreInput& input)const{return current_->calculate(input);}
std::string ScoreContext::algorithm()const{return current_->name();}
void LogObserver::onEvent(const DomainEvent& event){
 lines_.push_back(event.kind+" | id="+std::to_string(event.itemId)+" | "+event.state);
}
const std::vector<std::string>& LogObserver::lines()const{return lines_;}
void StatsObserver::onEvent(const DomainEvent& event){++events_[event.kind];}
int StatsObserver::count(const std::string& type)const{auto it=events_.find(type);return it==events_.end()?0:it->second;}
void EventPublisher::subscribe(const std::shared_ptr<IObserver>& observer){
 if(!observer)throw std::invalid_argument("Null observer");
 for(auto& w:subscribers_)if(w.lock()==observer)return;
 subscribers_.push_back(observer);
}
void EventPublisher::unsubscribe(const std::shared_ptr<IObserver>& observer){
 std::erase_if(subscribers_,[&](const auto& w){auto p=w.lock();return !p||p==observer;});
}
std::size_t EventPublisher::subscribers(){
 std::erase_if(subscribers_,[](const auto& w){return w.expired();});
 return subscribers_.size();
}
void EventPublisher::publish(const DomainEvent& event){
 std::erase_if(subscribers_,[](const auto& w){return w.expired();});
 for(auto& ref:subscribers_)if(auto o=ref.lock())o->onEvent(event);
}
WorkflowService::WorkflowService(IWorkflowRepository& repository,EventPublisher& events):repository_(repository),events_(events){}
int WorkflowService::registerItem(const WorkItem& item){
 WorkItem copy=item;copy.status="Registered";
 int id=repository_.create(copy);
 events_.publish({"CallRegistered",id,"Registered"});
 return id;
}
void WorkflowService::perform(int itemId,WorkflowAction action,bool injectFailure){
 auto item=repository_.get(itemId);
 if(!item)throw std::out_of_range("Item not found");
 auto state=makeWorkflowState(item->status);
 const auto next=state->next(action);
 if(!repository_.changeState(itemId,item->status,next,injectFailure))throw std::runtime_error("Concurrent state modification");
 const std::string type = (next=="Accepted" ? "CallAccepted" : (next=="Completed" ? "CallCompleted" : "ItemCancelled"));
 events_.publish({type,itemId,next});
}
TransitionCommand::TransitionCommand(WorkflowService& service,int id,WorkflowAction action):service_(service),itemId_(id),action_(action){}
void TransitionCommand::execute(){service_.perform(itemId_,action_);}
void CommandInvoker::enqueue(std::unique_ptr<ICommand> cmd){if(!cmd)throw std::invalid_argument("Null command");commands_.push_back(std::move(cmd));}
void CommandInvoker::run(){for(auto& cmd:commands_)cmd->execute();commands_.clear();}
