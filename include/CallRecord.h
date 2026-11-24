#pragma once
#include <string>

struct CallRecord {
    int id = 0;
    int operatorId = 0;
    std::string callDate;
    std::string clientNumber;
    int durationSeconds = 0;
    std::string outcome;
    std::string status = "Registered";
};
