// Library Management System -- Drogon Backend Server
#include "LibrarySystem.h"
#include "controllers/ApiController.h"
#include <drogon/drogon.h>
#include <iostream>

using namespace std;

int main() {
    // 1. Initialize database
    DatabaseManager& db = DatabaseManager::getInstance();
    if (!db.open("library.db")) {
        cerr << "Failed to open database." << endl;
        return 1;
    }
    if (!db.initTables()) {
        cerr << "Failed to initialize database tables." << endl;
        return 1;
    }

    // 2. Insert test data if not exists
    db.execSQL("INSERT OR IGNORE INTO users (user_id, username, password_hash, role, status) "
               "VALUES (1, 'admin', 'hashed_pwd', 'MANAGER', 'ACTIVE')");
    db.execSQL("INSERT OR IGNORE INTO users (user_id, username, password_hash, role, status) "
               "VALUES (2, 'alice', 'hashed_pwd', 'READER', 'ACTIVE')");
    db.execSQL("INSERT OR IGNORE INTO users (user_id, username, password_hash, role, status) "
               "VALUES (3, 'bob',   'hashed_pwd', 'READER', 'ACTIVE')");

    db.execSQL("INSERT OR IGNORE INTO books (book_id, isbn, title, author, publisher, "
               "total_stock, available_stock, location, status) "
               "VALUES (1, '978-7-111-12345-6', 'C++ Primer Plus', 'Stephen Prata', "
               "'Pearson', 5, 5, 'A-01-01', 'NORMAL')");
    db.execSQL("INSERT OR IGNORE INTO books (book_id, isbn, title, author, publisher, "
               "total_stock, available_stock, location, status) "
               "VALUES (2, '978-7-111-67890-1', 'Data Structures & Algorithm Analysis', "
               "'M. A. Weiss', 'Pearson', 3, 3, 'A-01-02', 'NORMAL')");
    db.execSQL("INSERT OR IGNORE INTO books (book_id, isbn, title, author, publisher, "
               "total_stock, available_stock, location, status) "
               "VALUES (3, '978-7-121-12345-6', 'Effective Modern C++', 'Scott Meyers', "
               "'O''Reilly', 2, 2, 'B-02-01', 'NORMAL')");

    cout << "Database initialized with test data." << endl;

    // 3. Start Drogon HTTP server
    cout << "\nStarting Library Management System API Server..." << endl;
    cout << "Listen on http://0.0.0.0:8080" << endl;
    cout << "Try: curl http://localhost:8080/api/books\n" << endl;

    try {
        drogon::app()
            .addListener("0.0.0.0", 8080)
            .setThreadNum(0)   // auto-detect
            .setLogLevel(trantor::Logger::kWarn)
            .run();
    } catch (const std::exception& e) {
        cerr << "Server error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
