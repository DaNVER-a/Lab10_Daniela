#pragma once
#include "PatternCore.h"
#include "Database.h"
#include <string>
class OrmWorkflowRepository : public IWorkflowRepository {
public:
 explicit OrmWorkflowRepository(const std::string& databasePath);
 int create(const WorkItem& item) override;
 std::optional<WorkItem> get(int itemId) override;
 bool changeState(int itemId,const std::string& from,const std::string& to,bool injectFailure=false) override;
 std::vector<WorkHistory> history(int itemId) override;
private:
 using Storage=decltype(createStorage(""));
 Storage storage_;
};
