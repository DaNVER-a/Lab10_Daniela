#pragma once
#include <string>
#include <sqlite_orm/sqlite_orm.h>
#include "CallOperator.h"
#include "CallRecord.h"
#include "CallStatusHistory.h"

inline auto createStorage(const std::string& filename) {
    using namespace sqlite_orm;

    return make_storage(
        filename,
        make_table(
            "Operators",
            make_column("id", &CallOperator::id, primary_key().autoincrement()),
            make_column("fullName", &CallOperator::fullName, not_null()),
            make_column("shift", &CallOperator::shift, not_null()),
            make_column("status", &CallOperator::status, not_null()),
            make_column("extension", &CallOperator::extension, not_null())
        ),
        make_table(
            "Calls",
            make_column("id", &CallRecord::id, primary_key().autoincrement()),
            make_column("operatorId", &CallRecord::operatorId),
            make_column("callDate", &CallRecord::callDate, not_null()),
            make_column("clientNumber", &CallRecord::clientNumber, not_null()),
            make_column("durationSeconds", &CallRecord::durationSeconds),
            make_column("outcome", &CallRecord::outcome, not_null()),
            make_column("status", &CallRecord::status, not_null()),
            foreign_key(&CallRecord::operatorId).references(&CallOperator::id)
        ),
        make_table(
            "CallStatusHistory",
            make_column("id", &CallStatusHistory::id, primary_key().autoincrement()),
            make_column("callId", &CallStatusHistory::callId),
            make_column("previousStatus", &CallStatusHistory::previousStatus, not_null()),
            make_column("newStatus", &CallStatusHistory::newStatus, not_null()),
            make_column("changedAt", &CallStatusHistory::changedAt, not_null()),
            foreign_key(&CallStatusHistory::callId).references(&CallRecord::id)
        )
    );
}
