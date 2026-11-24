#include "TestData.h"
#include "Database.h"
#include "CallDataAccess.h"
#include <filesystem>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <vector>

void prepareTestData() {
    const std::string file = "callcenter_queries.db";
    std::filesystem::remove(file);

    auto storage = createStorage(file);
    storage.on_open = [](sqlite3* db) {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK)
            throw std::runtime_error("Cannot enable foreign keys");
    };
    storage.open_forever();
    storage.sync_schema();

    const std::vector<CallOperator> operators = {
        {0, "Olena Kovalenko",   "morning", "active",   "101"},
        {0, "Andrii Melnyk",     "evening", "active",   "102"},
        {0, "Iryna Shevchenko",  "morning", "training", "103"},
        {0, "Maksym Bondarenko", "night",   "active",   "104"},
        {0, "Sofia Hrytsenko",   "evening", "active",   "105"}
    };

    std::vector<int> ids;
    for (const auto& item : operators) ids.push_back(storage.insert(item));

    const std::vector<CallRecord> calls = {
        {0, ids[0], "2026-11-19 09:00", "TEST-001", 180, "resolved"},
        {0, ids[0], "2026-11-19 09:25", "TEST-002", 240, "resolved"},
        {0, ids[0], "2026-11-19 10:10", "TEST-003",  90, "missed"},

        {0, ids[1], "2026-11-19 14:10", "TEST-004", 300, "resolved"},
        {0, ids[1], "2026-11-19 15:00", "TEST-005",  60, "transferred"},
        {0, ids[1], "2026-11-19 15:40", "TEST-006", 420, "resolved"},

        {0, ids[2], "2026-11-19 10:15", "TEST-007", 125, "resolved"},
        {0, ids[2], "2026-11-19 11:00", "TEST-008", 200, "transferred"},
        {0, ids[2], "2026-11-19 11:30", "TEST-009",  75, "missed"},

        {0, ids[3], "2026-11-19 22:00", "TEST-010", 600, "resolved"},
        {0, ids[3], "2026-11-19 23:10", "TEST-011", 150, "resolved"},
        {0, ids[3], "2026-11-19 23:50", "TEST-012", 360, "transferred"},

        {0, ids[4], "2026-11-19 17:00", "TEST-013",  45, "missed"},
        {0, ids[4], "2026-11-19 18:00", "TEST-014", 500, "resolved"},
        {0, ids[4], "2026-11-19 18:45", "TEST-015", 210, "resolved"}
    };

    CallDataAccess callAccess(storage);
    for (const auto& item : calls) callAccess.add(item);
    if (callAccess.findByOperator(ids[0]).size() != 3)
        throw std::runtime_error("Call relationship verification failed");

    if (storage.count<CallOperator>() != 5 || storage.count<CallRecord>() != 15)
        throw std::runtime_error("Test data count mismatch");

    std::cout << "\n=== TEST DATABASE ===\n"
              << "Operators: " << storage.count<CallOperator>() << '\n'
              << "Calls: " << storage.count<CallRecord>() << '\n';
}
