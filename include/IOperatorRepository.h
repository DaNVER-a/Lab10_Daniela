#pragma once
#include <optional>
#include <vector>
#include "CallOperator.h"

class IOperatorRepository {
public:
    virtual ~IOperatorRepository() = default;
    virtual int add(const CallOperator& operatorData) = 0;
    virtual std::optional<CallOperator> findById(int id) = 0;
    virtual std::vector<CallOperator> findAll() = 0;
    virtual bool update(const CallOperator& operatorData) = 0;
    virtual bool remove(int id) = 0;
};
