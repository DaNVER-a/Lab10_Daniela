#include "CallDataAccess.h"

CallDataAccess::CallDataAccess(Storage& storage) : storage_(storage) {}

int CallDataAccess::add(const CallRecord& record) {
    return storage_.insert(record);
}

std::optional<CallRecord> CallDataAccess::findById(int id) {
    return storage_.get_optional<CallRecord>(id);
}

std::vector<CallRecord> CallDataAccess::findByOperator(int operatorId) {
    return storage_.get_all<CallRecord>(
        sqlite_orm::where(
            sqlite_orm::c(&CallRecord::operatorId) == operatorId
        )
    );
}
