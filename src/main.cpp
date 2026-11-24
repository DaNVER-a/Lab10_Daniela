#include <exception>
#include <iostream>
#include "RepositoryDemo.h"
#include "TestData.h"
#include "Queries.h"
#include "Transactions.h"
#include "Comparison.h"
#include "PatternDemo.h"

int main() {
    try {
        runRepositoryDemo();
        prepareTestData();
        runQueryDemo();
        runTransactionDemo();
        runWithoutTransactionDemo();
        runComparisonDemo();
        runPatternDemo();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Application error: " << ex.what() << '\n';
        return 1;
    }
}
