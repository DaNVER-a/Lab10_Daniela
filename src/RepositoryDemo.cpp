#include "RepositoryDemo.h"
#include "IOperatorRepository.h"
#include "SQLiteOperatorRepository.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

void runRepositoryDemo() {
    const std::string file = "callcenter_repository_demo.db";
    std::filesystem::remove(file);

    SQLiteOperatorRepository sqliteRepository(file);
    IOperatorRepository& repository = sqliteRepository;

    std::cout << "\n=== REPOSITORY CRUD ===\n";
    int id = repository.add({0, "Daniela Verhunenko", "morning", "active", "101"});
    std::cout << "CREATE\nOperator ID: " << id << '\n';

    auto found = repository.findById(id);
    if (!found) throw std::runtime_error("READ failed");
    std::cout << "READ\n" << found->fullName << " | " << found->shift << " | "
              << found->status << " | " << found->extension << '\n';

    found->status = "training";
    bool updated = repository.update(*found);
    auto afterUpdate = repository.findById(id);
    if (!updated || !afterUpdate || afterUpdate->status != "training")
        throw std::runtime_error("UPDATE failed");
    std::cout << "UPDATE\nUpdated: " << std::boolalpha << updated
              << "\nNew status: " << afterUpdate->status << '\n';

    auto all = repository.findAll();
    std::cout << "READ ALL\nCount: " << all.size() << '\n';
    for (const auto& item : all) {
        std::cout << item.id << " | " << item.fullName << " | " << item.status << '\n';
    }

    bool removed = repository.remove(id);
    auto afterDelete = repository.findById(id);
    if (!removed || afterDelete) throw std::runtime_error("DELETE failed");
    std::cout << "DELETE\nRemoved: " << removed
              << "\nOperator exists after deletion: " << afterDelete.has_value() << '\n';

    if (repository.update({99999, "Not found", "morning", "active", "999"}) ||
        repository.remove(99999) || repository.findById(99999))
        throw std::runtime_error("Missing record verification failed");
    std::cout << "Missing record handled: true\n";
}
