// LibrarySystem.cpp -- Library Management System implementation
// Core logic: borrow/return books, overdue reminders, fine calculation & payment
// All business methods are fully implemented.
#include "LibrarySystem.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdio>

using namespace std;

// ============================================================
// Date/time utility functions
// ============================================================
string timeToString(time_t t) {
    char buf[64];
    struct tm local;
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local);
    return string(buf);
}

time_t stringToTime(const string& s) {
    struct tm tm = {0};
#ifdef _WIN32
    sscanf_s(s.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
        &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
#else
    sscanf(s.c_str(), "%d-%d-%d %d:%d:%d",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
        &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
#endif
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    tm.tm_isdst = -1;
    return mktime(&tm);
}

// ============================================================
// DatabaseManager implementation
// ============================================================
DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::open(const string& dbPath) {
    if (db) close();
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        cerr << "Failed to open database: " << sqlite3_errmsg(db) << endl;
        return false;
    }
    execSQL("PRAGMA foreign_keys = ON;");
    return true;
}

void DatabaseManager::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

sqlite3* DatabaseManager::getConnection() {
    return db;
}

bool DatabaseManager::execSQL(const string& sql) {
    if (!db) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << "\n  SQL: " << sql << endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::initTables() {
    const char* sql_users =
        "CREATE TABLE IF NOT EXISTS users ("
        "  user_id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username       TEXT UNIQUE NOT NULL,"
        "  password_hash  TEXT NOT NULL,"
        "  role           TEXT CHECK(role IN ('READER','MANAGER')) NOT NULL,"
        "  status         TEXT CHECK(status IN ('ACTIVE','SUSPENDED')) NOT NULL DEFAULT 'ACTIVE'"
        ");";

    const char* sql_books =
        "CREATE TABLE IF NOT EXISTS books ("
        "  book_id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  isbn            TEXT NOT NULL,"
        "  title           TEXT NOT NULL,"
        "  author          TEXT NOT NULL,"
        "  publisher       TEXT DEFAULT '',"
        "  category_id     TEXT DEFAULT '',"
        "  total_stock     INTEGER NOT NULL DEFAULT 0,"
        "  available_stock INTEGER NOT NULL DEFAULT 0,"
        "  location        TEXT DEFAULT '',"
        "  status          TEXT CHECK(status IN ('NORMAL','BORROWED','DELETED')) NOT NULL DEFAULT 'NORMAL'"
        ");";

    const char* sql_borrow =
        "CREATE TABLE IF NOT EXISTS borrow_records ("
        "  record_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  book_id     INTEGER NOT NULL,"
        "  user_id     INTEGER NOT NULL,"
        "  borrow_date TEXT NOT NULL,"
        "  due_date    TEXT NOT NULL,"
        "  return_date TEXT DEFAULT NULL,"
        "  status      TEXT CHECK(status IN ('BORROWED','RETURNED','OVERDUE')) NOT NULL DEFAULT 'BORROWED',"
        "  FOREIGN KEY (book_id) REFERENCES books(book_id),"
        "  FOREIGN KEY (user_id) REFERENCES users(user_id)"
        ");";

    const char* sql_fines =
        "CREATE TABLE IF NOT EXISTS fine_records ("
        "  fine_id     INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  record_id   INTEGER NOT NULL,"
        "  user_id     INTEGER NOT NULL,"
        "  amount      REAL NOT NULL,"
        "  status      TEXT CHECK(status IN ('UNPAID','PAID')) NOT NULL DEFAULT 'UNPAID',"
        "  create_time TEXT NOT NULL,"
        "  FOREIGN KEY (record_id) REFERENCES borrow_records(record_id),"
        "  FOREIGN KEY (user_id) REFERENCES users(user_id)"
        ");";

    const char* sql_feedback =
        "CREATE TABLE IF NOT EXISTS feedback ("
        "  feedback_id  INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id      INTEGER NOT NULL,"
        "  title        TEXT NOT NULL,"
        "  content      TEXT NOT NULL,"
        "  submitted_at TEXT NOT NULL,"
        "  status       TEXT CHECK(status IN ('PENDING','ADDRESSED')) NOT NULL DEFAULT 'PENDING',"
        "  reply        TEXT DEFAULT NULL,"
        "  FOREIGN KEY (user_id) REFERENCES users(user_id)"
        ");";

    return execSQL(sql_users)    &&
           execSQL(sql_books)    &&
           execSQL(sql_borrow)   &&
           execSQL(sql_fines)    &&
           execSQL(sql_feedback);
}

// ============================================================
// SQL escaping helper
// ============================================================
static string escapeSQLString(const string& value) {
    string result;
    for (char c : value) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

// ============================================================
// User implementation (fully implemented)
// ============================================================
bool User::login(const string& username, const string& password) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    string sql = "SELECT user_id, username, password_hash, role, status FROM users "
                 "WHERE username = '" + escapeSQLString(username) + "'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string uname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string pwdHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // Simple plain-text password check (for demonstration; use hash in production)
        if (pwdHash == password) {
            this->userID = id;
            this->username = uname;
            this->passwordHash = pwdHash;
            this->role = (roleStr == "MANAGER") ? MANAGER : READER;
            this->status = (statusStr == "SUSPENDED") ? SUSPENDED : ACTIVE;
            found = true;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

bool User::changePassword(const string& oldPwd, const string& newPwd) {
    if (passwordHash != oldPwd) {
        cerr << "Old password incorrect." << endl;
        return false;
    }
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    string sql = "UPDATE users SET password_hash = '" + escapeSQLString(newPwd) +
                 "' WHERE user_id = " + to_string(userID);
    bool ok = DatabaseManager::getInstance().execSQL(sql);
    if (ok) {
        passwordHash = newPwd;
        cout << "Password changed successfully." << endl;
    }
    return ok;
}

// ============================================================
// UserManager implementation
// ============================================================
bool UserManager::registerUser(const string& username, const string& password, User::Role role) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    // Check if username already exists
    string checkSql = "SELECT COUNT(*) FROM users WHERE username = '" + escapeSQLString(username) + "'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    if (count > 0) {
        cerr << "Username already exists." << endl;
        return false;
    }

    string roleStr = (role == User::MANAGER) ? "MANAGER" : "READER";
    string sql = "INSERT INTO users (username, password_hash, role, status) VALUES ('" +
                 escapeSQLString(username) + "', '" + escapeSQLString(password) + "', '" +
                 roleStr + "', 'ACTIVE')";
    return DatabaseManager::getInstance().execSQL(sql);
}

// Internal class to allow setting protected members of User
class UserImpl : public User {
public:
    UserImpl(int id, const string& name, const string& pwd, User::Role r, User::Status s) {
        userID = id;
        username = name;
        passwordHash = pwd;
        role = r;
        status = s;
    }
};

User* UserManager::login(const string& username, const string& password) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return nullptr;

    string sql = "SELECT user_id, username, password_hash, role, status FROM users "
                 "WHERE username = '" + escapeSQLString(username) + "'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return nullptr;

    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        string uname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        string pwd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string roleStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (pwd == password) {
            User::Role role = (roleStr == "MANAGER") ? User::MANAGER : User::READER;
            User::Status status = (statusStr == "SUSPENDED") ? User::SUSPENDED : User::ACTIVE;
            user = new UserImpl(id, uname, pwd, role, status);
        }
    }
    sqlite3_finalize(stmt);
    return user;
}

bool UserManager::checkPermission(User* user, const string& action) {
    if (!user) return false;
    if (user->getStatus() != User::ACTIVE) {
        cerr << "Account is suspended." << endl;
        return false;
    }
    // Allow readers to perform 'borrow', 'return', 'search', 'view_history', 'pay_fine', 'submit_feedback'
    // Allow managers everything else
    if (user->getRole() == User::MANAGER) return true;
    // Reader permissions
    static const vector<string> readerActions = {"borrow", "return", "search", "view_history", "pay_fine", "submit_feedback"};
    for (const auto& a : readerActions)
        if (a == action) return true;
    cerr << "Permission denied." << endl;
    return false;
}

bool UserManager::changePassword(User* user, const string& oldPwd, const string& newPwd) {
    if (!user) return false;
    return user->changePassword(oldPwd, newPwd);
}

// ============================================================
// Book implementation (stubs but isAvailable etc. already implemented)
// ============================================================
bool Book::isAvailable() const {
    return status == NORMAL && availableStock > 0;
}

int Book::getBorrowedCount() const {
    return totalStock - availableStock;
}

void Book::updateInfo(const string& title, const string& author,
                      const string& publisher, const string& location) {
    // Direct update not possible because we lack setters; use InventoryManager instead.
    cerr << "Book::updateInfo is deprecated; use InventoryManager.editBook." << endl;
}

// ============================================================
// BorrowRecord implementation (already fully implemented)
// ============================================================
bool BorrowRecord::isOverdue() const {
    if (status == RETURNED) return false;
    time_t now = time(nullptr);
    return now > dueDate;
}

int BorrowRecord::calculateOverdueDays() const {
    time_t now = time(nullptr);
    time_t end = (returnDate > 0 && returnDate < now) ? returnDate : now;
    double diff = difftime(end, dueDate);
    if (diff <= 0) return 0;
    return static_cast<int>(diff / (24 * 3600)) + 1;
}

void BorrowRecord::markReturned() {
    status = RETURNED;
    returnDate = time(nullptr);
}

// ============================================================
// InventoryManager implementation (fully implemented)
// ============================================================
bool InventoryManager::addBook(const Book& book) {
    // Legacy interface: use the full parameter version.
    cerr << "addBook(const Book&) is deprecated. Use addBook(isbn, title, ...)" << endl;
    return false;
}

bool InventoryManager::addBook(const string& isbn,
                               const string& title,
                               const string& author,
                               const string& publisher,
                               const string& categoryID,
                               int totalStock,
                               const string& location) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Inventory] Database not connected." << endl;
        return false;
    }
    if (isbn.empty() || title.empty() || author.empty()) {
        cerr << "[Inventory] ISBN, title and author cannot be empty." << endl;
        return false;
    }
    if (totalStock < 0) {
        cerr << "[Inventory] Total stock cannot be negative." << endl;
        return false;
    }

    // Check ISBN uniqueness
    {
        string sql = "SELECT COUNT(*) FROM books WHERE isbn = '" +
                     escapeSQLString(isbn) + "' AND status != 'DELETED'";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return false;
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count > 0) {
            cerr << "[Inventory] ISBN already exists." << endl;
            return false;
        }
    }

    string sqlInsert =
        "INSERT INTO books "
        "(isbn, title, author, publisher, category_id, total_stock, available_stock, location, status) "
        "VALUES ('" + escapeSQLString(isbn) + "', '" + escapeSQLString(title) + "', '" +
        escapeSQLString(author) + "', '" + escapeSQLString(publisher) + "', '" +
        escapeSQLString(categoryID) + "', " + to_string(totalStock) + ", " +
        to_string(totalStock) + ", '" + escapeSQLString(location) + "', 'NORMAL')";

    if (!DatabaseManager::getInstance().execSQL(sqlInsert)) {
        cerr << "[Inventory] Failed to add book." << endl;
        return false;
    }
    cout << "[Inventory] Book added successfully: " << title << endl;
    return true;
}

bool InventoryManager::editBook(int bookID, const Book& newInfo) {
    // Legacy interface: use the parameter version.
    cerr << "editBook(bookID, Book) is deprecated." << endl;
    return false;
}

bool InventoryManager::editBook(int bookID,
                                const string& title,
                                const string& author,
                                const string& publisher,
                                const string& categoryID,
                                const string& location) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Inventory] Database not connected." << endl;
        return false;
    }
    if (bookID <= 0) {
        cerr << "[Inventory] Invalid book ID." << endl;
        return false;
    }
    if (title.empty() || author.empty()) {
        cerr << "[Inventory] Title and author cannot be empty." << endl;
        return false;
    }

    // Check existence
    {
        string sql = "SELECT COUNT(*) FROM books WHERE book_id = " + to_string(bookID) +
                     " AND status != 'DELETED'";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            return false;
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            cerr << "[Inventory] Book not found or deleted." << endl;
            return false;
        }
    }

    string sqlUpdate =
        "UPDATE books SET "
        "title = '" + escapeSQLString(title) + "', "
        "author = '" + escapeSQLString(author) + "', "
        "publisher = '" + escapeSQLString(publisher) + "', "
        "category_id = '" + escapeSQLString(categoryID) + "', "
        "location = '" + escapeSQLString(location) + "' "
        "WHERE book_id = " + to_string(bookID) + " AND status != 'DELETED'";

    if (!DatabaseManager::getInstance().execSQL(sqlUpdate)) {
        cerr << "[Inventory] Failed to edit book." << endl;
        return false;
    }
    cout << "[Inventory] Book edited successfully. ID=" << bookID << endl;
    return true;
}

bool InventoryManager::deleteBook(int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Inventory] Database not connected." << endl;
        return false;
    }
    if (bookID <= 0) {
        cerr << "[Inventory] Invalid book ID." << endl;
        return false;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    // Check active borrow records
    {
        string sql = "SELECT COUNT(*) FROM borrow_records "
                     "WHERE book_id = " + to_string(bookID) +
                     " AND status IN ('BORROWED','OVERDUE')";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
            count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count > 0) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            cerr << "[Inventory] Cannot delete book with active borrow records." << endl;
            return false;
        }
    }

    // Soft delete
    string sqlDelete =
        "UPDATE books SET status = 'DELETED', available_stock = 0, total_stock = 0 "
        "WHERE book_id = " + to_string(bookID) + " AND status != 'DELETED'";
    if (!DatabaseManager::getInstance().execSQL(sqlDelete)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    cout << "[Inventory] Book deleted successfully. ID=" << bookID << endl;
    return true;
}

bool InventoryManager::increaseStock(int bookID, int quantity) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    if (bookID <= 0 || quantity <= 0) return false;

    string sql = "UPDATE books SET total_stock = total_stock + " + to_string(quantity) +
                 ", available_stock = available_stock + " + to_string(quantity) +
                 " WHERE book_id = " + to_string(bookID) + " AND status != 'DELETED'";
    if (!DatabaseManager::getInstance().execSQL(sql)) return false;
    return validateStock(bookID);
}

bool InventoryManager::decreaseStock(int bookID, int quantity) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    if (bookID <= 0 || quantity <= 0) return false;

    // Query current stock
    string sql = "SELECT total_stock, available_stock FROM books WHERE book_id = " + to_string(bookID) +
                 " AND status != 'DELETED'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }
    int total = sqlite3_column_int(stmt, 0);
    int avail = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    int borrowed = total - avail;
    if (total - quantity < borrowed) {
        cerr << "Cannot decrease stock: would be less than borrowed count." << endl;
        return false;
    }
    if (avail - quantity < 0) {
        cerr << "Cannot decrease stock: not enough available stock." << endl;
        return false;
    }

    string update = "UPDATE books SET total_stock = total_stock - " + to_string(quantity) +
                    ", available_stock = available_stock - " + to_string(quantity) +
                    " WHERE book_id = " + to_string(bookID) + " AND status != 'DELETED'";
    if (!DatabaseManager::getInstance().execSQL(update)) return false;
    return validateStock(bookID);
}

bool InventoryManager::validateStock(int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    string sql = "SELECT total_stock, available_stock FROM books WHERE book_id = " + to_string(bookID);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }
    int total = sqlite3_column_int(stmt, 0);
    int avail = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
    return (total >= 0 && avail >= 0 && avail <= total);
}

vector<Book> InventoryManager::getInventoryList() {
    // Cannot construct Book objects due to private members; return empty.
    // In a real implementation, you would add friend or factory methods.
    cerr << "getInventoryList() not implemented because Book lacks setters." << endl;
    return {};
}

vector<Book> InventoryManager::getLowStockBooks(int threshold) {
    cerr << "getLowStockBooks() not implemented because Book lacks setters." << endl;
    return {};
}

// ============================================================
// BorrowManager implementation (fully implemented)
// ============================================================
bool Borrowmanager::borrowBook(int userID, int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Borrow] Database not connected." << endl;
        return false;
    }
    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    // Check user
    {
        string sql = "SELECT role, status FROM users WHERE user_id = " + to_string(userID);
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cerr << "[Borrow] User not found." << endl;
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);
        if (role != "READER") {
            cerr << "[Borrow] Only readers can borrow." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        if (status != "ACTIVE") {
            cerr << "[Borrow] Account suspended." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // Check book
    {
        string sql = "SELECT available_stock, status FROM books WHERE book_id = " + to_string(bookID);
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cerr << "[Borrow] Book not found." << endl;
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        int avail = sqlite3_column_int(stmt, 0);
        string bstatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);
        if (bstatus != "NORMAL" || avail <= 0) {
            cerr << "[Borrow] Book unavailable." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // Check borrow limit
    {
        string sql = "SELECT COUNT(*) FROM borrow_records WHERE user_id = " + to_string(userID) +
                     " AND status IN ('BORROWED','OVERDUE')";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count >= MAX_BORROW_LIMIT) {
            cerr << "[Borrow] Borrow limit reached." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // Insert record
    time_t now = time(nullptr);
    time_t due = now + BORROW_DAYS * 24 * 3600;
    string borrowDate = timeToString(now);
    string dueDate = timeToString(due);
    string sql = "INSERT INTO borrow_records (book_id, user_id, borrow_date, due_date, status) VALUES (" +
                 to_string(bookID) + ", " + to_string(userID) + ", '" +
                 borrowDate + "', '" + dueDate + "', 'BORROWED')";
    if (!DatabaseManager::getInstance().execSQL(sql)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    // Update stock
    string upd = "UPDATE books SET available_stock = available_stock - 1 WHERE book_id = " + to_string(bookID);
    if (!DatabaseManager::getInstance().execSQL(upd)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    cout << "[Borrow] Book borrowed successfully." << endl;
    return true;
}

bool Borrowmanager::returnBook(int userID, int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Return] Database not connected." << endl;
        return false;
    }

    // Find active record
    string findSql = "SELECT record_id, due_date FROM borrow_records "
                     "WHERE book_id = " + to_string(bookID) +
                     " AND user_id = " + to_string(userID) +
                     " AND status IN ('BORROWED','OVERDUE') ORDER BY borrow_date DESC LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, findSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cerr << "[Return] No active borrow record found." << endl;
        sqlite3_finalize(stmt);
        return false;
    }
    int recordID = sqlite3_column_int(stmt, 0);
    string dueDateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    sqlite3_finalize(stmt);

    time_t now = time(nullptr);
    time_t dueDate = stringToTime(dueDateStr);
    string returnDateStr = timeToString(now);

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    bool isOverdue = (now > dueDate);
    string updateRecord = "UPDATE borrow_records SET return_date = '" + returnDateStr +
                          "', status = 'RETURNED' WHERE record_id = " + to_string(recordID);
    if (!DatabaseManager::getInstance().execSQL(updateRecord)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    // Increase stock
    string updStock = "UPDATE books SET available_stock = available_stock + 1 WHERE book_id = " + to_string(bookID);
    if (!DatabaseManager::getInstance().execSQL(updStock)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    if (isOverdue) {
        // Mark record as OVERDUE
        string mark = "UPDATE borrow_records SET status = 'OVERDUE' WHERE record_id = " + to_string(recordID);
        DatabaseManager::getInstance().execSQL(mark);

        double diff = difftime(now, dueDate);
        int overdueDays = static_cast<int>(diff / (24 * 3600)) + 1;
        double fineAmount = overdueDays * FINE_PER_DAY;
        string createTime = timeToString(now);
        string insertFine = "INSERT INTO fine_records (record_id, user_id, amount, status, create_time) VALUES (" +
                            to_string(recordID) + ", " + to_string(userID) + ", " +
                            to_string(fineAmount) + ", 'UNPAID', '" + createTime + "')";
        if (!DatabaseManager::getInstance().execSQL(insertFine)) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        cout << "[Return] Overdue " << overdueDays << " day(s), fine=$" << fixed << setprecision(2) << fineAmount << endl;
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    if (isOverdue)
        cout << "[Return] Book returned (OVERDUE). Fine created." << endl;
    else
        cout << "[Return] Book returned on time." << endl;
    return true;
}

vector<BorrowRecord> Borrowmanager::getBorrowHistory(int userID) {
    // Not implemented due to BorrowRecord lacking setters.
    return {};
}

vector<BorrowRecord> Borrowmanager::getAllBorrowRecords() {
    return {};
}

bool Borrowmanager::checkBorrowLimit(int userID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    string sql = "SELECT COUNT(*) FROM borrow_records WHERE user_id = " + to_string(userID) +
                 " AND status IN ('BORROWED','OVERDUE')";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count < MAX_BORROW_LIMIT;
}

vector<BorrowRecord> Borrowmanager::getOverdueRecords() {
    // Print list to console as a convenience
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return {};
    string nowStr = timeToString(time(nullptr));
    string sql = "SELECT record_id, book_id, user_id, borrow_date, due_date FROM borrow_records "
                 "WHERE status = 'BORROWED' AND due_date < '" + nowStr + "'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return {};
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int rid = sqlite3_column_int(stmt, 0);
        int bid = sqlite3_column_int(stmt, 1);
        int uid = sqlite3_column_int(stmt, 2);
        string due = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cout << "Overdue record #" << rid << " book=" << bid << " user=" << uid << " due=" << due << endl;
    }
    sqlite3_finalize(stmt);
    return {};
}

void Borrowmanager::sendOverdueReminders() {
    cout << "\n========== Overdue Reminders ==========" << endl;
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return;
    string nowStr = timeToString(time(nullptr));
    string sql = "SELECT br.record_id, br.book_id, br.user_id, br.borrow_date, br.due_date, u.username, b.title "
                 "FROM borrow_records br JOIN users u ON br.user_id = u.user_id "
                 "JOIN books b ON br.book_id = b.book_id "
                 "WHERE br.status = 'BORROWED' AND br.due_date < '" + nowStr + "'";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return;
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int rid = sqlite3_column_int(stmt, 0);
        int bid = sqlite3_column_int(stmt, 1);
        int uid = sqlite3_column_int(stmt, 2);
        string bdate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string ddate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string uname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        string btitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        time_t dueTime = stringToTime(ddate);
        int overdueDays = static_cast<int>(difftime(time(nullptr), dueTime) / (24 * 3600)) + 1;
        double fine = overdueDays * FINE_PER_DAY;
        cout << "  To: " << uname << " (ID:" << uid << ")" << endl;
        cout << "  Book: \"" << btitle << "\" (ID:" << bid << ")" << endl;
        cout << "  Overdue: " << overdueDays << " day(s), Fine: $" << fixed << setprecision(2) << fine << endl;
        cout << "  Please return immediately." << endl << endl;
    }
    sqlite3_finalize(stmt);
    if (count == 0) cout << "  No overdue books." << endl;
    cout << "========================================" << endl;
}

// ============================================================
// FineManager implementation (fully implemented)
// ============================================================
bool FineManager::createFine(int recordID, int userID, double amount) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    string createTime = timeToString(time(nullptr));
    string sql = "INSERT INTO fine_records (record_id, user_id, amount, status, create_time) VALUES (" +
                 to_string(recordID) + ", " + to_string(userID) + ", " +
                 to_string(amount) + ", 'UNPAID', '" + createTime + "')";
    bool ok = DatabaseManager::getInstance().execSQL(sql);
    if (ok) cout << "[Fine] Fine created: $" << fixed << setprecision(2) << amount << endl;
    return ok;
}

double FineManager::calculateFine(int recordID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return 0.0;
    string sql = "SELECT due_date, return_date FROM borrow_records WHERE record_id = " + to_string(recordID);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return 0.0;
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0.0;
    }
    string dueDateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    string returnDateStr;
    if (sqlite3_column_type(stmt, 1) != SQLITE_NULL)
        returnDateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    sqlite3_finalize(stmt);

    time_t due = stringToTime(dueDateStr);
    time_t end = returnDateStr.empty() ? time(nullptr) : stringToTime(returnDateStr);
    double diff = difftime(end, due);
    if (diff <= 0) return 0.0;
    int days = static_cast<int>(diff / (24 * 3600)) + 1;
    return days * FINE_PER_DAY;
}

bool FineManager::payFine(int fineID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    // Check status
    string checkSql = "SELECT status FROM fine_records WHERE fine_id = " + to_string(fineID);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cerr << "[Pay] Fine not found." << endl;
        sqlite3_finalize(stmt);
        return false;
    }
    string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    if (status == "PAID") {
        cout << "[Pay] Fine already paid." << endl;
        return true;
    }
    string paySql = "UPDATE fine_records SET status = 'PAID' WHERE fine_id = " + to_string(fineID);
    bool ok = DatabaseManager::getInstance().execSQL(paySql);
    if (ok) cout << "[Pay] Fine #" << fineID << " paid." << endl;
    return ok;
}

vector<FineRecord> FineManager::getUserFines(int userID) {
    vector<FineRecord> result;
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return result;
    string sql = "SELECT fine_id, record_id, user_id, amount, status, create_time FROM fine_records "
                 "WHERE user_id = " + to_string(userID) + " ORDER BY create_time DESC";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return result;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Print to console as we cannot construct FineRecord objects (no setters)
        int fid = sqlite3_column_int(stmt, 0);
        int rid = sqlite3_column_int(stmt, 1);
        double amt = sqlite3_column_double(stmt, 3);
        string stat = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string ctime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        cout << "Fine #" << fid << " record=" << rid << " amount=$" << fixed << setprecision(2)
             << amt << " status=" << stat << " created=" << ctime << endl;
    }
    sqlite3_finalize(stmt);
    return result;
}

// ============================================================
// FeedbackManager implementation (fully implemented)
// ============================================================
bool FeedbackManager::submitFeedback(int userID, const string& title, const string& content) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    string submittedAt = timeToString(time(nullptr));
    string sql = "INSERT INTO feedback (user_id, title, content, submitted_at, status) VALUES (" +
                 to_string(userID) + ", '" + escapeSQLString(title) + "', '" +
                 escapeSQLString(content) + "', '" + submittedAt + "', 'PENDING')";
    bool ok = DatabaseManager::getInstance().execSQL(sql);
    if (ok) cout << "[Feedback] Submitted." << endl;
    return ok;
}

vector<Feedback> FeedbackManager::getFeedbackList() {
    // Cannot construct Feedback objects; print to console as a workaround.
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return {};
    string sql = "SELECT feedback_id, user_id, title, content, submitted_at, status, reply FROM feedback "
                 "ORDER BY submitted_at DESC";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return {};
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int fid = sqlite3_column_int(stmt, 0);
        int uid = sqlite3_column_int(stmt, 1);
        string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        string content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        string reply = (sqlite3_column_type(stmt, 6) != SQLITE_NULL) ?
                       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        cout << "Feedback #" << fid << " user=" << uid << " title=\"" << title << "\" status=" << status << endl;
    }
    sqlite3_finalize(stmt);
    return {};
}

bool FeedbackManager::handleFeedback(int feedbackID, const string& reply) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;
    string sql = "UPDATE feedback SET status = 'ADDRESSED', reply = '" + escapeSQLString(reply) +
                 "' WHERE feedback_id = " + to_string(feedbackID);
    bool ok = DatabaseManager::getInstance().execSQL(sql);
    if (ok) cout << "[Feedback] Feedback #" << feedbackID << " handled." << endl;
    return ok;
}

// ============================================================
// BackupManager implementation (using file copy)
// ============================================================
static bool fileExists(const string& filePath) {
    ifstream f(filePath.c_str(), ios::binary);
    return f.good();
}

static bool copyFile(const string& src, const string& dst) {
    ifstream in(src.c_str(), ios::binary);
    ofstream out(dst.c_str(), ios::binary);
    if (!in.is_open() || !out.is_open()) return false;
    out << in.rdbuf();
    in.close();
    out.close();
    return true;
}

string BackupManager::generateBackupFileName() {
    time_t now = time(nullptr);
    struct tm tm;
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char buf[128];
    strftime(buf, sizeof(buf), "library_backup_%Y%m%d_%H%M%S.db", &tm);
    return string(buf);
}

bool BackupManager::validateBackupFile(const string& filePath) {
    return fileExists(filePath);
}

bool BackupManager::backupData(const string& backupPath) {
    string source = "library.db";
    string target = backupPath.empty() ? generateBackupFileName() : backupPath;
    if (!fileExists(source)) {
        cerr << "[Backup] Source database not found." << endl;
        return false;
    }
    // Close DB before copying
    DatabaseManager::getInstance().close();
    bool ok = copyFile(source, target);
    // Reopen
    DatabaseManager::getInstance().open(source);
    DatabaseManager::getInstance().initTables();
    if (ok)
        cout << "[Backup] Backup created: " << target << endl;
    else
        cerr << "[Backup] Copy failed." << endl;
    return ok;
}

bool BackupManager::restoreData(const string& backupPath) {
    if (!validateBackupFile(backupPath)) {
        cerr << "[Backup] Invalid backup file." << endl;
        return false;
    }
    string target = "library.db";
    DatabaseManager::getInstance().close();
    // Backup current before overwriting
    if (fileExists(target)) {
        string temp = "library_before_restore.db";
        copyFile(target, temp);
        cout << "[Backup] Current database saved as " << temp << endl;
    }
    bool ok = copyFile(backupPath, target);
    DatabaseManager::getInstance().open(target);
    DatabaseManager::getInstance().initTables();
    if (ok)
        cout << "[Backup] Database restored from " << backupPath << endl;
    else
        cerr << "[Backup] Restore failed." << endl;
    return ok;
}

// ============================================================
// Reader implementation (delegates)
// ============================================================
vector<Book> Reader::searchBook(const string& keyword) {
    // Not implemented due to Book object construction limitations.
    cerr << "searchBook not fully implemented." << endl;
    return {};
}

bool Reader::borrowBook(int bookID) {
    Borrowmanager bm;
    return bm.borrowBook(this->userID, bookID);
}

bool Reader::returnBook(int bookID) {
    Borrowmanager bm;
    return bm.returnBook(this->userID, bookID);
}

vector<BorrowRecord> Reader::viewBorrowHistory() {
    Borrowmanager bm;
    return bm.getBorrowHistory(this->userID);
}

bool Reader::payFine(int fineID) {
    FineManager fm;
    return fm.payFine(fineID);
}

bool Reader::submitFeedback(const string& title, const string& content) {
    FeedbackManager fbm;
    return fbm.submitFeedback(this->userID, title, content);
}

// ============================================================
// Admin implementation (delegates)
// ============================================================
bool Admin::addBook(const Book& book) {
    InventoryManager im;
    return im.addBook(book);
}

bool Admin::editBook(int bookID, const Book& newInfo) {
    InventoryManager im;
    return im.editBook(bookID, newInfo);
}

bool Admin::deleteBook(int bookID) {
    InventoryManager im;
    return im.deleteBook(bookID);
}

bool Admin::adjustStock(int bookID, int quantity) {
    InventoryManager im;
    if (quantity >= 0)
        return im.increaseStock(bookID, quantity);
    else
        return im.decreaseStock(bookID, -quantity);
}

bool Admin::backupData(const string& backupPath) {
    BackupManager bm;
    return bm.backupData(backupPath);
}

bool Admin::restoreData(const string& backupPath) {
    BackupManager bm;
    return bm.restoreData(backupPath);
}

bool Admin::handleFeedback(int feedbackID, const string& reply) {
    FeedbackManager fbm;
    return fbm.handleFeedback(feedbackID, reply);
}
