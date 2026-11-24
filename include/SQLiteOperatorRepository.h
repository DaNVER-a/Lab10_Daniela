#pragma once
#include <string>
#include "IOperatorRepository.h"
#include "Database.h"

class SQLiteOperatorRepository : public IOperatorRepository {
public:
    explicit SQLiteOperatorRepository(const std::string& databaseFile);
    int add(const CallOperator& operatorData) override;
    std::optional<CallOperator> findById(int id) override;
    std::vector<CallOperator> findAll() override;
    bool update(const CallOperator& operatorData) override;
    bool remove(int id) override;

private:
    using Storage = decltype(createStorage(""));
    Storage storage_;
};
