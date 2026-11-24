#include "Queries.h"
#include "Database.h"
#include <iomanip>
#include <iostream>
#include <stdexcept>

void runQueryDemo() {
    using namespace sqlite_orm;
    auto storage = createStorage("callcenter_queries.db");
    storage.sync_schema();

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n=== FILTERING: MORNING SHIFT ===\n";
    const auto morning = storage.get_all<CallOperator>(
        where(c(&CallOperator::shift) == "morning")
    );
    for (const auto& item : morning)
        std::cout << item.id << " | " << item.fullName << " | " << item.shift << '\n';

    std::cout << "\n=== SORTING: OPERATOR NAME ===\n";
    const auto sorted = storage.get_all<CallOperator>(
        order_by(&CallOperator::fullName).asc()
    );
    for (const auto& item : sorted)
        std::cout << item.id << " | " << item.fullName << '\n';

    std::cout << "\n=== TOP 3 LONGEST CALLS ===\n";
    const auto longest = storage.get_all<CallRecord>(
        order_by(&CallRecord::durationSeconds).desc(), limit(3)
    );
    for (const auto& call : longest)
        std::cout << call.id << " | " << call.clientNumber << " | "
                  << call.durationSeconds << " sec\n";

    std::cout << "\n=== AGGREGATION ===\n";
    const auto count = storage.count<CallRecord>();
    auto total = storage.sum(&CallRecord::durationSeconds);
    const double average = storage.avg(&CallRecord::durationSeconds);
    std::cout << "Call count: " << count << '\n'
              << "Total duration (sec): " << (total ? *total : 0) << '\n'
              << "Average duration (sec): " << average << '\n';

    if (count != 15 || !total || *total != 3555)
        throw std::runtime_error("Aggregate verification failed");

    std::cout << "\n=== GROUP BY OPERATOR ===\n";
    const auto grouped = storage.select(
        columns(&CallRecord::operatorId, sum(&CallRecord::durationSeconds)),
        group_by(&CallRecord::operatorId)
    );
    for (const auto& [id, duration] : grouped)
        std::cout << "Operator ID: " << id << " | Total duration: "
                  << (duration ? *duration : 0) << " sec\n";

    std::cout << "\n=== INNER JOIN ===\n";
    const auto joined = storage.select(
        columns(
            &CallOperator::fullName,
            &CallRecord::callDate,
            &CallRecord::clientNumber,
            &CallRecord::durationSeconds,
            &CallRecord::outcome
        ),
        inner_join<CallRecord>(
            on(c(&CallOperator::id) == &CallRecord::operatorId)
        ),
        order_by(&CallRecord::id)
    );
    if (joined.size() != 15) throw std::runtime_error("JOIN verification failed");
    for (const auto& [name, date, client, seconds, outcome] : joined)
        std::cout << name << " | " << date << " | " << client
                  << " | " << seconds << " | " << outcome << '\n';
}
