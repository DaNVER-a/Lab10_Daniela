cmake_minimum_required(VERSION 3.21)
project(Lab10_CallCenter LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
add_executable(Lab10_CallCenter
    src/main.cpp
   src/SQLiteOperatorRepository.cpp
   src/CallDataAccess.cpp
   src/RepositoryDemo.cpp
   src/TestData.cpp
   src/Queries.cpp
   src/Transactions.cpp
   src/Comparison.cpp
   src/PatternCore.cpp
   src/OrmWorkflowRepository.cpp
   src/PatternDemo.cpp
)
target_include_directories(Lab10_CallCenter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Lab10_CallCenter PRIVATE sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)

add_executable(Lab10_CallCenter_Tests src/PatternCore.cpp src/PatternTests.cpp)
target_include_directories(Lab10_CallCenter_Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Lab10_CallCenter_DBTests src/PatternCore.cpp src/OrmWorkflowRepository.cpp src/PatternIntegrationTests.cpp)
target_include_directories(Lab10_CallCenter_DBTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Lab10_CallCenter_DBTests PRIVATE sqlite_orm::sqlite_orm unofficial::sqlite3::sqlite3)
enable_testing()
add_test(NAME PatternUnitTests COMMAND Lab10_CallCenter_Tests)
add_test(NAME PatternDatabaseTests COMMAND Lab10_CallCenter_DBTests)
