#pragma once
#include <optional>
#include <vector>
#include "Database.h"

class CallDataAccess {
public:
    using Storage = decltype(createStorage(""));
    explicit CallDataAccess(Storage& storage);
    int add(const CallRecord& record);
    std::optional<CallRecord> findById(int id);
    std::vector<CallRecord> findByOperator(int operatorId);

private:
    Storage& storage_;
};
