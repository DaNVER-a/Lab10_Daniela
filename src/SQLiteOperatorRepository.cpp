#include "SQLiteOperatorRepository.h"
#include <sqlite3.h>
#include <stdexcept>

SQLiteOperatorRepository::SQLiteOperatorRepository(const std::string& databaseFile)
    : storage_(createStorage(databaseFile)) {
    storage_.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage_.sync_schema();
}

int SQLiteOperatorRepository::add(const CallOperator& operatorData) {
    return storage_.insert(operatorData);
}

std::optional<CallOperator> SQLiteOperatorRepository::findById(int id) {
    return storage_.get_optional<CallOperator>(id);
}

std::vector<CallOperator> SQLiteOperatorRepository::findAll() {
    return storage_.get_all<CallOperator>();
}

bool SQLiteOperatorRepository::update(const CallOperator& operatorData) {
    if (!storage_.get_optional<CallOperator>(operatorData.id)) return false;
    storage_.update(operatorData);
    return true;
}

bool SQLiteOperatorRepository::remove(int id) {
    if (!storage_.get_optional<CallOperator>(id)) return false;
    storage_.remove<CallOperator>(id);
    return true;
}
