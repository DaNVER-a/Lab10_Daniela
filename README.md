# Laboratory Work 10 — TPP

**Student:** Вергуненко Даніела Сергіївна  
**Group:** FIT-3-17  
**Project:** Робота кол-центру

This C++20 project extends the previous SQLite + `sqlite_orm` + Repository system with State, Strategy, Observer and an additional Command pattern.

All primary entities and one-to-many relationships from the previous assignment remain in place. Status transitions and history rows are stored atomically using SQLite transactions.

## Programs

- `Lab10_CallCenter` — original database demos and the four new patterns
- `Lab10_CallCenter_Tests` — self-contained logic tests
- `Lab10_CallCenter_DBTests` — SQLite/ORM transaction tests

## Build

Open this folder in Microsoft Visual Studio with the C++ workload and select the `default` CMake preset. Use **Build All**. Dependencies (`sqlite3`, `sqlite-orm`) are managed by vcpkg.

Full setup instructions and report diagrams are in the `docs/` folder.
