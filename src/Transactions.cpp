#include "Transactions.h"
#include "Database.h"
#include <filesystem>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>

namespace {
using Storage = decltype(createStorage(""));

void configureStorage(Storage& storage) {
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("Cannot enable foreign keys");
    };
    storage.open_forever();
    storage.sync_schema();
}
}

void runTransactionDemo() {
    auto storage = createStorage("callcenter_queries.db");
    configureStorage(storage);

    const auto operatorsBefore = storage.count<CallOperator>();
    const auto callsBefore = storage.count<CallRecord>();
    std::cout << "\n=== COMMIT ===\n"
              << "Operators before: " << operatorsBefore << '\n'
              << "Calls before: " << callsBefore << '\n';

    const bool committed = storage.transaction([&]() {
        const int operatorId = storage.insert(CallOperator{
            0, "Test New Operator", "morning", "active", "206"
        });
        storage.insert(CallRecord{
            0, operatorId, "2026-11-19 12:00", "TEST-016", 120, "resolved"
        });
        return true;
    });
    const auto operatorsAfterCommit = storage.count<CallOperator>();
    const auto callsAfterCommit = storage.count<CallRecord>();
    std::cout << "Committed: " << std::boolalpha << committed << '\n'
              << "Operators after: " << operatorsAfterCommit << '\n'
              << "Calls after: " << callsAfterCommit << '\n';

    if (!committed || operatorsAfterCommit != operatorsBefore + 1 ||
        callsAfterCommit != callsBefore + 1)
        throw std::runtime_error("COMMIT verification failed");

    std::cout << "\n=== ROLLBACK ===\n";
    bool failureDetected = false;
    try {
        storage.transaction([&]() {
            storage.insert(CallOperator{
                0, "Rollback Operator", "night", "active", "207"
            });
            storage.insert(CallRecord{
                0, -1, "2026-11-19 23:00", "TEST-BAD", 60, "resolved"
            });
            return true;
        });
    } catch (const std::exception& ex) {
        failureDetected = true;
        std::cout << "Error: " << ex.what() << '\n';
    }

    const auto accidental = storage.get_all<CallOperator>(
        sqlite_orm::where(
            sqlite_orm::c(&CallOperator::fullName) == "Rollback Operator"
        )
    );
    const bool rollbackCorrect = failureDetected && accidental.empty() &&
        storage.count<CallOperator>() == operatorsAfterCommit &&
        storage.count<CallRecord>() == callsAfterCommit;

    std::cout << "Operators after rollback: " << storage.count<CallOperator>() << '\n'
              << "Calls after rollback: " << storage.count<CallRecord>() << '\n'
              << "Rollback operator exists: " << !accidental.empty() << '\n'
              << "Rollback successful: " << rollbackCorrect << '\n';
    if (!rollbackCorrect) throw std::runtime_error("ROLLBACK verification failed");
}

void runWithoutTransactionDemo() {
    const std::string file = "callcenter_no_transaction.db";
    std::filesystem::remove(file);
    auto storage = createStorage(file);
    configureStorage(storage);

    bool failed = false;
    std::cout << "\n=== WITHOUT TRANSACTION ===\n";
    try {
        storage.insert(CallOperator{
            0, "No Transaction Operator", "evening", "active", "208"
        });
        storage.insert(CallRecord{
            0, -1, "2026-11-19 19:00", "TEST-BAD", 60, "resolved"
        });
    } catch (const std::exception& ex) {
        failed = true;
        std::cout << "Error: " << ex.what() << '\n';
    }

    const auto operatorCount = storage.count<CallOperator>();
    const auto callCount = storage.count<CallRecord>();
    const bool partialSave = failed && operatorCount == 1 && callCount == 0;
    std::cout << "Operators after: " << operatorCount << '\n'
              << "Calls after: " << callCount << '\n'
              << "Partial changes saved: " << std::boolalpha << partialSave << '\n';
    if (!partialSave) throw std::runtime_error("Non-transaction verification failed");
}
