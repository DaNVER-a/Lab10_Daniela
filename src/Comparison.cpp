#include "Comparison.h"
#include "Database.h"
#include <sqlite3.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
using DatabaseHandle = std::unique_ptr<sqlite3, decltype(&sqlite3_close)>;
using StatementHandle = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

void check(int result, sqlite3* db) {
    if (result != SQLITE_OK) throw std::runtime_error(sqlite3_errmsg(db));
}

StatementHandle prepare(sqlite3* db, const char* sql) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &raw, nullptr) != SQLITE_OK) {
        std::string error = sqlite3_errmsg(db);
        sqlite3_finalize(raw);
        throw std::runtime_error(error);
    }
    return StatementHandle(raw, &sqlite3_finalize);
}

std::string readText(sqlite3_stmt* stmt, int column) {
    const auto* value = sqlite3_column_text(stmt, column);
    if (!value) throw std::runtime_error("Unexpected null text");
    return reinterpret_cast<const char*>(value);
}

CallOperator runRawApi() {
    const std::string file = "callcenter_raw_comparison.db";
    std::filesystem::remove(file);
    sqlite3* rawDb = nullptr;
    const int opened = sqlite3_open(file.c_str(), &rawDb);
    DatabaseHandle db(rawDb, &sqlite3_close);
    if (opened != SQLITE_OK) throw std::runtime_error("Cannot open SQLite database");

    const char* createSql =
        "CREATE TABLE Operators ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "fullName TEXT NOT NULL,"
        "shift TEXT NOT NULL,"
        "status TEXT NOT NULL,"
        "extension TEXT NOT NULL);";
    char* errorMessage = nullptr;
    if (sqlite3_exec(db.get(), createSql, nullptr, nullptr, &errorMessage) != SQLITE_OK) {
        const std::string error = errorMessage ? errorMessage : sqlite3_errmsg(db.get());
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    int operatorId = 0;
    {
        auto stmt = prepare(db.get(),
            "INSERT INTO Operators (fullName, shift, status, extension) "
            "VALUES (?, ?, ?, ?);");
        check(sqlite3_bind_text(stmt.get(), 1, "Test Operator", -1, SQLITE_TRANSIENT), db.get());
        check(sqlite3_bind_text(stmt.get(), 2, "morning", -1, SQLITE_TRANSIENT), db.get());
        check(sqlite3_bind_text(stmt.get(), 3, "active", -1, SQLITE_TRANSIENT), db.get());
        check(sqlite3_bind_text(stmt.get(), 4, "300", -1, SQLITE_TRANSIENT), db.get());
        if (sqlite3_step(stmt.get()) != SQLITE_DONE)
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        operatorId = static_cast<int>(sqlite3_last_insert_rowid(db.get()));
    }

    auto stmt = prepare(db.get(),
        "SELECT id, fullName, shift, status, extension FROM Operators WHERE id = ?;");
    check(sqlite3_bind_int(stmt.get(), 1, operatorId), db.get());
    const int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_ROW) {
        if (rc == SQLITE_DONE) throw std::runtime_error("Operator not found (C API)");
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }
    CallOperator result{
        sqlite3_column_int(stmt.get(), 0),
        readText(stmt.get(), 1),
        readText(stmt.get(), 2),
        readText(stmt.get(), 3),
        readText(stmt.get(), 4)
    };
    return result;
}

CallOperator runOrm() {
    const std::string file = "callcenter_orm_comparison.db";
    std::filesystem::remove(file);
    auto storage = createStorage(file);
    storage.sync_schema();
    const int id = storage.insert(CallOperator{
        0, "Test Operator", "morning", "active", "300"
    });
    const auto item = storage.get_optional<CallOperator>(id);
    if (!item) throw std::runtime_error("Operator not found (ORM)");
    return *item;
}
}

void runComparisonDemo() {
    const CallOperator raw = runRawApi();
    const CallOperator orm = runOrm();
    const bool match = raw.id == orm.id &&
        raw.fullName == orm.fullName &&
        raw.shift == orm.shift &&
        raw.status == orm.status &&
        raw.extension == orm.extension;

    std::cout << "\n=== SQLITE C API VS ORM ===\n"
              << "SQLite C API: " << raw.id << " | " << raw.fullName << " | "
              << raw.shift << " | " << raw.status << " | " << raw.extension << '\n'
              << "sqlite_orm:   " << orm.id << " | " << orm.fullName << " | "
              << orm.shift << " | " << orm.status << " | " << orm.extension << '\n'
              << "Results match: " << std::boolalpha << match << '\n';
    if (!match) throw std::runtime_error("API comparison failed");
}
