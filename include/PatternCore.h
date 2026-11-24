#pragma once
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "CallRecord.h"
#include "CallStatusHistory.h"

using WorkItem = CallRecord;
using WorkHistory = CallStatusHistory;
enum class WorkflowAction { Accept, Finish, Cancel };

class IWorkflowState {
public:
    virtual ~IWorkflowState()=default;
    virtual std::string name() const=0;
    virtual std::string next(WorkflowAction action) const=0;
};
std::unique_ptr<IWorkflowState> makeWorkflowState(const std::string& state);

struct ScoreInput {double first=0; double second=0; double third=0;};
class IScoreStrategy {
public:
    virtual ~IScoreStrategy()=default;
    virtual std::string name() const=0;
    virtual double calculate(const ScoreInput&) const=0;
};
class MaxNormalized final : public IScoreStrategy {
public: std::string name() const override; double calculate(const ScoreInput&) const override;
};
class WeightedSum final : public IScoreStrategy {
public: std::string name() const override; double calculate(const ScoreInput&) const override;
};
class NormalizedWeightedSum final : public IScoreStrategy {
public: std::string name() const override; double calculate(const ScoreInput&) const override;
};
class ScoreContext {
public:
    explicit ScoreContext(std::unique_ptr<IScoreStrategy> strategy);
    void setStrategy(std::unique_ptr<IScoreStrategy> strategy);
    double evaluate(const ScoreInput& input) const;
    std::string algorithm() const;
private: std::unique_ptr<IScoreStrategy> current_;
};

struct DomainEvent {std::string kind; int itemId; std::string state;};
class IObserver {
public:
    virtual ~IObserver()=default;
    virtual void onEvent(const DomainEvent& event)=0;
};
class LogObserver final : public IObserver {
public:
    void onEvent(const DomainEvent& event) override;
    const std::vector<std::string>& lines() const;
private: std::vector<std::string> lines_;
};
class StatsObserver final : public IObserver {
public:
    void onEvent(const DomainEvent& event) override;
    int count(const std::string& type) const;
private: std::map<std::string,int> events_;
};
class EventPublisher {
public:
    void subscribe(const std::shared_ptr<IObserver>& observer);
    void unsubscribe(const std::shared_ptr<IObserver>& observer);
    void publish(const DomainEvent& event);
    std::size_t subscribers();
private: std::vector<std::weak_ptr<IObserver>> subscribers_;
};

class IWorkflowRepository {
public:
    virtual ~IWorkflowRepository()=default;
    virtual int create(const WorkItem& item)=0;
    virtual std::optional<WorkItem> get(int itemId)=0;
    virtual bool changeState(int itemId, const std::string& from,
                             const std::string& to, bool injectFailure=false)=0;
    virtual std::vector<WorkHistory> history(int itemId)=0;
};
class WorkflowService {
public:
    WorkflowService(IWorkflowRepository& repository, EventPublisher& events);
    int registerItem(const WorkItem& item);
    void perform(int itemId, WorkflowAction action, bool injectFailure=false);
private:
    IWorkflowRepository& repository_;
    EventPublisher& events_;
};

// Command: queue actions without coupling the caller to the concrete service operations.
class ICommand {public: virtual ~ICommand()=default; virtual void execute()=0;};
class TransitionCommand final : public ICommand {
public:
    TransitionCommand(WorkflowService& service, int id, WorkflowAction action);
    void execute() override;
private: WorkflowService& service_; int itemId_; WorkflowAction action_;
};
class CommandInvoker {
public: void enqueue(std::unique_ptr<ICommand> command); void run();
private: std::vector<std::unique_ptr<ICommand>> commands_;
};
