// LibrarySystem.c -- Library Management System implementation
// Core logic: borrow/return books, overdue reminders, fine calculation & payment
// Other business methods are left as stubs (returning defaults)
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
// User stubs
// ============================================================
bool User::login(const string& username, const string& password) {
    // TODO: authenticate from database
    return false;
}

bool User::changePassword(const string& oldPwd, const string& newPwd) {
    // TODO: change password in database
    return false;
}

// ============================================================
// Book stubs
// ============================================================
bool Book::isAvailable() const {
    return status == NORMAL && availableStock > 0;
}

int Book::getBorrowedCount() const {
    return totalStock - availableStock;
}

void Book::updateInfo(const string& title, const string& author,
                      const string& publisher, const string& location) {
    // TODO: update book info in database
}

// ============================================================
// BorrowRecord stubs
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
// UserManager stubs
// ============================================================
bool UserManager::registerUser(const string& username, const string& password, User::Role role) {
    // TODO: register user
    return false;
}

User* UserManager::login(const string& username, const string& password) {
    // TODO: user login
    return nullptr;
}

bool UserManager::checkPermission(User* user, const string& action) {
    // TODO: permission check
    return false;
}

bool UserManager::changePassword(User* user, const string& oldPwd, const string& newPwd) {
    // TODO: change password
    return false;
}

// ============================================================
// InventoryManager implementation
// ============================================================

static string escapeSQLString(const string& value) {
    string result;
    int i;

    for (i = 0; i < (int)value.length(); i++) {
        if (value[i] == '\'') {
            result += "''";
        } else {
            result += value[i];
        }
    }

    return result;
}

bool InventoryManager::addBook(const Book& book) {
    // 当前 Book 类缺少完整 setter，外部难以构造完整 Book。
    // 该函数保留是为了兼容原接口，实际 API 使用下面的重载 addBook。
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

    // 检查 ISBN 是否重复。因为当前建表语句里 isbn 没有 UNIQUE，所以这里手动检查。
    {
        string sql = "SELECT COUNT(*) FROM books WHERE isbn = '" +
                     escapeSQLString(isbn) +
                     "' AND status != 'DELETED'";

        sqlite3_stmt* stmt = NULL;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            cerr << "[Inventory] Failed to check ISBN." << endl;
            return false;
        }

        int count = 0;

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);

        if (count > 0) {
            cerr << "[Inventory] ISBN already exists." << endl;
            return false;
        }
    }

    string sqlInsert =
        "INSERT INTO books "
        "(isbn, title, author, publisher, category_id, total_stock, available_stock, location, status) "
        "VALUES ('" +
        escapeSQLString(isbn) + "', '" +
        escapeSQLString(title) + "', '" +
        escapeSQLString(author) + "', '" +
        escapeSQLString(publisher) + "', '" +
        escapeSQLString(categoryID) + "', " +
        to_string(totalStock) + ", " +
        to_string(totalStock) + ", '" +
        escapeSQLString(location) + "', 'NORMAL')";

    if (!DatabaseManager::getInstance().execSQL(sqlInsert)) {
        cerr << "[Inventory] Failed to add book." << endl;
        return false;
    }

    cout << "[Inventory] Book added successfully: " << title << endl;
    return true;
}

bool InventoryManager::editBook(int bookID, const Book& newInfo) {
    // 当前 Book 类缺少完整 getter/setter，保留原接口兼容。
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

    // 检查图书是否存在且未删除
    {
        string sql = "SELECT COUNT(*) FROM books WHERE book_id = " +
                     to_string(bookID) +
                     " AND status != 'DELETED'";

        sqlite3_stmt* stmt = NULL;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            cerr << "[Inventory] Failed to check book." << endl;
            return false;
        }

        int count = 0;

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

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
        "WHERE book_id = " + to_string(bookID) +
        " AND status != 'DELETED'";

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

    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

    // 检查是否存在未归还记录
    {
        string sql = "SELECT COUNT(*) FROM borrow_records "
                     "WHERE book_id = " + to_string(bookID) +
                     " AND status IN ('BORROWED','OVERDUE')";

        sqlite3_stmt* stmt = NULL;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
            cerr << "[Inventory] Failed to check borrow records." << endl;
            return false;
        }

        int count = 0;

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);

        if (count > 0) {
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
            cerr << "[Inventory] Cannot delete book with active borrow records." << endl;
            return false;
        }
    }

    // 软删除：标记为 DELETED，同时库存清零
    string sqlDelete =
        "UPDATE books SET "
        "status = 'DELETED', "
        "available_stock = 0, "
        "total_stock = 0 "
        "WHERE book_id = " + to_string(bookID) +
        " AND status != 'DELETED'";

    if (!DatabaseManager::getInstance().execSQL(sqlDelete)) {
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        cerr << "[Inventory] Failed to delete book." << endl;
        return false;
    }

    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

    cout << "[Inventory] Book deleted successfully. ID=" << bookID << endl;
    return true;
}

bool InventoryManager::increaseStock(int bookID, int quantity) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    if (!db) {
        cerr << "[Inventory] Database not connected." << endl;
        return false;
    }

    if (bookID <= 0 || quantity <= 0) {
        cerr << "[Inventory] Invalid book ID or quantity." << endl;
        return false;
    }

    string sql =
        "UPDATE books SET "
        "total_stock = total_stock + " + to_string(quantity) + ", "
        "available_stock = available_stock + " + to_string(quantity) + " "
        "WHERE book_id = " + to_string(bookID) +
        " AND status != 'DELETED'";

    if (!DatabaseManager::getInstance().execSQL(sql)) {
        cerr << "[Inventory] Failed to increase stock." << endl;
        return false;
    }

    return validateStock(bookID);
}

bool InventoryManager::decreaseStock(int bookID, int quantity) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    if (!db) {
        cerr << "[Inventory] Database not connected." << endl;
        return false;
    }

    if (bookID <= 0 || quantity <= 0) {
        cerr << "[Inventory] Invalid book ID or quantity." << endl;
        return false;
    }

    int totalStock = 0;
    int availableStock = 0;

    // 查询当前库存
    {
        string sql = "SELECT total_stock, available_stock FROM books "
                     "WHERE book_id = " + to_string(bookID) +
                     " AND status != 'DELETED'";

        sqlite3_stmt* stmt = NULL;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
            cerr << "[Inventory] Failed to query stock." << endl;
            return false;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            cerr << "[Inventory] Book not found." << endl;
            return false;
        }

        totalStock = sqlite3_column_int(stmt, 0);
        availableStock = sqlite3_column_int(stmt, 1);

        sqlite3_finalize(stmt);
    }

    int borrowedCount = totalStock - availableStock;

    if (totalStock - quantity < borrowedCount) {
        cerr << "[Inventory] Cannot decrease stock: total stock would be less than borrowed count." << endl;
        return false;
    }

    if (availableStock - quantity < 0) {
        cerr << "[Inventory] Cannot decrease stock: available stock is not enough." << endl;
        return false;
    }

    string sqlUpdate =
        "UPDATE books SET "
        "total_stock = total_stock - " + to_string(quantity) + ", "
        "available_stock = available_stock - " + to_string(quantity) + " "
        "WHERE book_id = " + to_string(bookID) +
        " AND status != 'DELETED'";

    if (!DatabaseManager::getInstance().execSQL(sqlUpdate)) {
        cerr << "[Inventory] Failed to decrease stock." << endl;
        return false;
    }

    return validateStock(bookID);
}

bool InventoryManager::validateStock(int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    if (!db) {
        return false;
    }

    string sql = "SELECT total_stock, available_stock FROM books WHERE book_id = " +
                 to_string(bookID);

    sqlite3_stmt* stmt = NULL;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    int totalStock = sqlite3_column_int(stmt, 0);
    int availableStock = sqlite3_column_int(stmt, 1);

    sqlite3_finalize(stmt);

    if (totalStock < 0) {
        return false;
    }

    if (availableStock < 0) {
        return false;
    }

    if (availableStock > totalStock) {
        return false;
    }

    return true;
}

vector<Book> InventoryManager::getInventoryList() {
    vector<Book> result;

    // 由于 Book 成员 private 且没有完整 setter，
    // 这里暂时返回空 vector。
    // API 层会直接查询数据库生成 JSON。
    return result;
}

vector<Book> InventoryManager::getLowStockBooks(int threshold) {
    vector<Book> result;

    // 同上，API 层直接查询数据库。
    return result;
}

// ============================================================
// BackupManager implementation
// ============================================================

static bool fileExistsOld(const string& filePath) {
    ifstream file(filePath.c_str(), ios::binary);
    bool exists = file.good();
    file.close();
    return exists;
}

static bool copyFileOldStyle(const string& sourcePath, const string& targetPath) {
    ifstream source(sourcePath.c_str(), ios::binary);
    ofstream target(targetPath.c_str(), ios::binary);

    if (!source.is_open() || !target.is_open()) {
        return false;
    }

    target << source.rdbuf();

    source.close();
    target.close();

    return true;
}

string BackupManager::generateBackupFileName() {
    time_t now = time(NULL);
    struct tm localTime;

#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    char buffer[128];

#ifdef _WIN32
    sprintf_s(buffer, sizeof(buffer),
              "library_backup_%04d%02d%02d_%02d%02d%02d.db",
              localTime.tm_year + 1900,
              localTime.tm_mon + 1,
              localTime.tm_mday,
              localTime.tm_hour,
              localTime.tm_min,
              localTime.tm_sec);
#else
    sprintf(buffer,
            "library_backup_%04d%02d%02d_%02d%02d%02d.db",
            localTime.tm_year + 1900,
            localTime.tm_mon + 1,
            localTime.tm_mday,
            localTime.tm_hour,
            localTime.tm_min,
            localTime.tm_sec);
#endif

    return string(buffer);
}

bool BackupManager::validateBackupFile(const string& filePath) {
    if (filePath.empty()) {
        return false;
    }

    if (!fileExistsOld(filePath)) {
        return false;
    }

    return true;
}

bool BackupManager::backupData(const string& backupPath) {
    string sourceDbPath = "library.db";
    string targetPath = backupPath;

    if (!fileExistsOld(sourceDbPath)) {
        cerr << "[Backup] Source database file does not exist." << endl;
        return false;
    }

    if (targetPath.empty()) {
        targetPath = generateBackupFileName();
    }

    // 为避免数据库仍有连接导致复制不完整，先关闭数据库
    DatabaseManager::getInstance().close();

    bool ok = copyFileOldStyle(sourceDbPath, targetPath);

    // 重新打开数据库
    DatabaseManager::getInstance().open(sourceDbPath);
    DatabaseManager::getInstance().initTables();

    if (!ok) {
        cerr << "[Backup] Failed to copy database file." << endl;
        return false;
    }

    cout << "[Backup] Backup created: " << targetPath << endl;
    return true;
}

bool BackupManager::restoreData(const string& backupPath) {
    string targetDbPath = "library.db";
    string beforeRestorePath = "library_before_restore.db";

    if (!validateBackupFile(backupPath)) {
        cerr << "[Backup] Invalid backup file." << endl;
        return false;
    }

    // 先关闭数据库连接
    DatabaseManager::getInstance().close();

    // 恢复前先备份当前数据库
    if (fileExistsOld(targetDbPath)) {
        copyFileOldStyle(targetDbPath, beforeRestorePath);
    }

    bool ok = copyFileOldStyle(backupPath, targetDbPath);

    // 重新打开数据库
    DatabaseManager::getInstance().open(targetDbPath);
    DatabaseManager::getInstance().initTables();

    if (!ok) {
        cerr << "[Backup] Failed to restore database." << endl;
        return false;
    }

    cout << "[Backup] Database restored from: " << backupPath << endl;
    return true;
}

// ============================================================
// BorrowManager -- Core: borrow book
// ============================================================
bool Borrowmanager::borrowBook(int userID, int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Borrow] Database not connected." << endl;
        return false;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    // 1. Check user exists and is an active READER
    {
        string sql = "SELECT role, status FROM users WHERE user_id = " + to_string(userID);
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            cerr << "[Borrow] User not found (ID=" << userID << ")." << endl;
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        string userStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);

        if (role != "READER") {
            cerr << "[Borrow] Only readers can borrow books." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        if (userStatus != "ACTIVE") {
            cerr << "[Borrow] Reader account is suspended." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // 2. Check book exists and is available
    {
        string sql = "SELECT available_stock, status FROM books WHERE book_id = " + to_string(bookID);
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            cerr << "[Borrow] Book not found (ID=" << bookID << ")." << endl;
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        int avail = sqlite3_column_int(stmt, 0);
        string bstatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);

        if (bstatus != "NORMAL") {
            cerr << "[Borrow] Book is unavailable (status=" << bstatus << ")." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
        if (avail <= 0) {
            cerr << "[Borrow] No available stock for book ID=" << bookID << "." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // 3. Check borrow limit
    {
        string sql = "SELECT COUNT(*) FROM borrow_records "
                     "WHERE user_id = " + to_string(userID) +
                     "  AND status IN ('BORROWED','OVERDUE')";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count >= MAX_BORROW_LIMIT) {
            cerr << "[Borrow] Borrow limit reached (" << MAX_BORROW_LIMIT << ")." << endl;
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // 4. Insert borrow record
    {
        time_t now = time(nullptr);
        time_t due = now + BORROW_DAYS * 24 * 3600;
        string borrowDate = timeToString(now);
        string dueDate = timeToString(due);

        string sql = "INSERT INTO borrow_records (book_id, user_id, borrow_date, due_date, status) "
                     "VALUES (" + to_string(bookID) + ", " + to_string(userID) +
                     ", '" + borrowDate + "', '" + dueDate + "', 'BORROWED')";
        if (!DatabaseManager::getInstance().execSQL(sql)) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    // 5. Update stock
    {
        string sql = "UPDATE books SET available_stock = available_stock - 1 "
                     "WHERE book_id = " + to_string(bookID);
        if (!DatabaseManager::getInstance().execSQL(sql)) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    cout << "[Borrow] Book ID=" << bookID << " borrowed by User ID=" << userID << " successfully." << endl;
    return true;
}

// ============================================================
// BorrowManager -- Core: return book
// ============================================================
bool Borrowmanager::returnBook(int userID, int bookID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Return] Database not connected." << endl;
        return false;
    }

    // Find the active borrow record for this user+book
    string findSql = "SELECT record_id, due_date FROM borrow_records "
                     "WHERE book_id = " + to_string(bookID) +
                     "  AND user_id = " + to_string(userID) +
                     "  AND status IN ('BORROWED','OVERDUE') "
                     "ORDER BY borrow_date DESC LIMIT 1";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, findSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[Return] SQL prepare error." << endl;
        return false;
    }
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        cerr << "[Return] No active borrow record found (user=" << userID
             << ", book=" << bookID << ")." << endl;
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

    // Determine if overdue
    bool isOverdue = (now > dueDate);

    // Update borrow record
    string updateRecord = "UPDATE borrow_records SET "
                          "return_date = '" + returnDateStr + "', "
                          "status = 'RETURNED' "
                          "WHERE record_id = " + to_string(recordID);
    if (!DatabaseManager::getInstance().execSQL(updateRecord)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    // Update book stock
    string updateStock = "UPDATE books SET available_stock = available_stock + 1 "
                         "WHERE book_id = " + to_string(bookID);
    if (!DatabaseManager::getInstance().execSQL(updateStock)) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    // If overdue, create a fine record
    if (isOverdue) {
        // Mark the record as OVERDUE (it was late)
        string markOverdue = "UPDATE borrow_records SET status = 'OVERDUE' "
                             "WHERE record_id = " + to_string(recordID);
        DatabaseManager::getInstance().execSQL(markOverdue);

        double diff = difftime(now, dueDate);
        int overdueDays = static_cast<int>(diff / (24 * 3600)) + 1;
        double fineAmount = overdueDays * FINE_PER_DAY;
        string createTime = timeToString(now);

        string insertFine = "INSERT INTO fine_records (record_id, user_id, amount, status, create_time) "
                            "VALUES (" + to_string(recordID) + ", " + to_string(userID) +
                            ", " + to_string(fineAmount) + ", 'UNPAID', '" + createTime + "')";
        if (!DatabaseManager::getInstance().execSQL(insertFine)) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }

        cout << "[Return] Book returned LATE. Overdue " << overdueDays
             << " day(s), fine=$" << fixed << setprecision(2) << fineAmount << endl;
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);

    if (isOverdue)
        cout << "[Return] Book ID=" << bookID << " returned (OVERDUE). A fine has been created." << endl;
    else
        cout << "[Return] Book ID=" << bookID << " returned on time. Thank you!" << endl;

    return true;
}

// ============================================================
// BorrowManager -- History (stubs)
// ============================================================
vector<BorrowRecord> Borrowmanager::getBorrowHistory(int userID) {
    // TODO: query from database
    return {};
}

vector<BorrowRecord> Borrowmanager::getAllBorrowRecords() {
    // TODO: admin query all records
    return {};
}

bool Borrowmanager::checkBorrowLimit(int userID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    string sql = "SELECT COUNT(*) FROM borrow_records "
                 "WHERE user_id = " + to_string(userID) +
                 "  AND status IN ('BORROWED','OVERDUE')";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;
    sqlite3_step(stmt);
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count < MAX_BORROW_LIMIT;
}

// ============================================================
// BorrowManager -- Core: overdue detection and reminders
// ============================================================
vector<BorrowRecord> Borrowmanager::getOverdueRecords() {
    vector<BorrowRecord> result;
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return result;

    string nowStr = timeToString(time(nullptr));
    string sql = "SELECT record_id, book_id, user_id, borrow_date, due_date, status "
                 "FROM borrow_records "
                 "WHERE status = 'BORROWED' AND due_date < '" + nowStr + "'";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int recordID = sqlite3_column_int(stmt, 0);
        int bookID = sqlite3_column_int(stmt, 1);
        int userID = sqlite3_column_int(stmt, 2);
        string dueDateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        // Look up user and book names
        string userSql = "SELECT username FROM users WHERE user_id = " + to_string(userID);
        string bookSql = "SELECT title FROM books WHERE book_id = " + to_string(bookID);

        string uname, btitle;
        sqlite3_stmt* uStmt = nullptr;
        if (sqlite3_prepare_v2(db, userSql.c_str(), -1, &uStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(uStmt) == SQLITE_ROW)
                uname = reinterpret_cast<const char*>(sqlite3_column_text(uStmt, 0));
            sqlite3_finalize(uStmt);
        }
        sqlite3_stmt* bStmt = nullptr;
        if (sqlite3_prepare_v2(db, bookSql.c_str(), -1, &bStmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(bStmt) == SQLITE_ROW)
                btitle = reinterpret_cast<const char*>(sqlite3_column_text(bStmt, 0));
            sqlite3_finalize(bStmt);
        }

        cout << "  [OVERDUE] Record #" << recordID
             << " | User: " << uname << " (ID:" << userID << ")"
             << " | Book: \"" << btitle << "\" (ID:" << bookID << ")"
             << " | Due: " << dueDateStr << endl;
    }
    sqlite3_finalize(stmt);
    return result;
}

void Borrowmanager::sendOverdueReminders() {
    cout << "\n========== Overdue Book Reminders ==========" << endl;
    cout << "Checking for overdue books..." << endl;

    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        cerr << "[Reminder] Database not connected." << endl;
        return;
    }

    string nowStr = timeToString(time(nullptr));
    string sql = "SELECT br.record_id, br.book_id, br.user_id, "
                 "       br.borrow_date, br.due_date, "
                 "       u.username, b.title "
                 "FROM borrow_records br "
                 "JOIN users u ON br.user_id = u.user_id "
                 "JOIN books b ON br.book_id = b.book_id "
                 "WHERE br.status = 'BORROWED' AND br.due_date < '" + nowStr + "'";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "[Reminder] SQL error." << endl;
        return;
    }

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
        int recordID  = sqlite3_column_int(stmt, 0);
        int bookID    = sqlite3_column_int(stmt, 1);
        int userID    = sqlite3_column_int(stmt, 2);
        string bDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string dDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string uname  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        string btitle = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        // Calculate overdue days and accrued fine
        time_t dueTime = stringToTime(dDate);
        time_t now = time(nullptr);
        int overdueDays = static_cast<int>(difftime(now, dueTime) / (24 * 3600)) + 1;
        double fine = overdueDays * FINE_PER_DAY;

        cout << "\n  --- Reminder #" << count << " ---" << endl;
        cout << "  To: " << uname << " (User ID: " << userID << ")" << endl;
        cout << "  Book: \"" << btitle << "\" (ID: " << bookID << ")" << endl;
        cout << "  Borrow Date: " << bDate << endl;
        cout << "  Due Date:    " << dDate << endl;
        cout << "  Overdue:     " << overdueDays << " day(s)" << endl;
        cout << "  Accrued Fine: $" << fixed << setprecision(2) << fine << endl;
        cout << "  Please return the book as soon as possible!" << endl;
    }
    sqlite3_finalize(stmt);

    if (count == 0) {
        cout << "  No overdue books found. Great job everyone!" << endl;
    }
    cout << "=============================================\n" << endl;
}

// ============================================================
// FineManager -- Core: create fine
// ============================================================
bool FineManager::createFine(int recordID, int userID, double amount) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    string createTime = timeToString(time(nullptr));
    string sql = "INSERT INTO fine_records (record_id, user_id, amount, status, create_time) "
                 "VALUES (" + to_string(recordID) + ", " + to_string(userID) +
                 ", " + to_string(amount) + ", 'UNPAID', '" + createTime + "')";

    bool ok = DatabaseManager::getInstance().execSQL(sql);
    if (ok) {
        cout << "[Fine] Fine created: $" << fixed << setprecision(2) << amount
             << " for record #" << recordID << endl;
    }
    return ok;
}

// ============================================================
// FineManager -- Core: calculate fine
// ============================================================
double FineManager::calculateFine(int recordID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return 0.0;

    string sql = "SELECT due_date, return_date FROM borrow_records "
                 "WHERE record_id = " + to_string(recordID);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return 0.0;

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 0.0;
    }

    string dueDateStr    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    string returnDateStr;
    if (sqlite3_column_type(stmt, 1) != SQLITE_NULL)
        returnDateStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    sqlite3_finalize(stmt);

    time_t dueDate = stringToTime(dueDateStr);
    time_t endDate;
    if (!returnDateStr.empty())
        endDate = stringToTime(returnDateStr);
    else
        endDate = time(nullptr);  // not yet returned, calculate to now

    double diff = difftime(endDate, dueDate);
    if (diff <= 0) return 0.0;

    int overdueDays = static_cast<int>(diff / (24 * 3600)) + 1;
    double fine = overdueDays * FINE_PER_DAY;

    cout << "[Fine] Calculated for record #" << recordID << ": "
         << overdueDays << " day(s) overdue, fine=$" << fixed << setprecision(2) << fine << endl;
    return fine;
}

// ============================================================
// FineManager -- Core: pay fine
// ============================================================
bool FineManager::payFine(int fineID) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return false;

    // Check fine info first
    string checkSql = "SELECT amount, status FROM fine_records WHERE fine_id = " + to_string(fineID);
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cerr << "[Pay] Fine record #" << fineID << " not found." << endl;
        sqlite3_finalize(stmt);
        return false;
    }

    double amount = sqlite3_column_double(stmt, 0);
    string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    sqlite3_finalize(stmt);

    if (status == "PAID") {
        cout << "[Pay] Fine #" << fineID << " is already paid." << endl;
        return true;
    }

    // Process payment
    string paySql = "UPDATE fine_records SET status = 'PAID' WHERE fine_id = " + to_string(fineID);
    bool ok = DatabaseManager::getInstance().execSQL(paySql);

    if (ok) {
        cout << "[Pay] Fine #" << fineID << " paid successfully. Amount: $"
             << fixed << setprecision(2) << amount << endl;
    } else {
        cerr << "[Pay] Failed to update fine #" << fineID << "." << endl;
    }
    return ok;
}

// ============================================================
// FineManager -- Query user fines
// ============================================================
vector<FineRecord> FineManager::getUserFines(int userID) {
    vector<FineRecord> result;
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) return result;

    string sql = "SELECT fine_id, record_id, user_id, amount, status, create_time "
                 "FROM fine_records WHERE user_id = " + to_string(userID) +
                 " ORDER BY create_time DESC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int fineID  = sqlite3_column_int(stmt, 0);
        int recID   = sqlite3_column_int(stmt, 1);
        double amt  = sqlite3_column_double(stmt, 3);
        string stat = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        string ctime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        cout << "  Fine #" << fineID << " | Record #" << recID
             << " | Amount: $" << fixed << setprecision(2) << amt
             << " | Status: " << stat
             << " | Created: " << ctime << endl;
    }
    sqlite3_finalize(stmt);
    return result;
}

// ============================================================
// FeedbackManager stubs
// ============================================================
bool FeedbackManager::submitFeedback(int userID, const string& title, const string& content) {
    // TODO: submit feedback
    return false;
}

vector<Feedback> FeedbackManager::getFeedbackList() {
    // TODO: get feedback list
    return {};
}

bool FeedbackManager::handleFeedback(int feedbackID, const string& reply) {
    // TODO: handle feedback
    return false;
}

// ============================================================
// BackupManager stubs
// ============================================================
bool BackupManager::backupData(const string& backupPath) {
    // TODO: backup data
    return false;
}

bool BackupManager::restoreData(const string& backupPath) {
    // TODO: restore data
    return false;
}

bool BackupManager::validateBackupFile(const string& filePath) {
    // TODO: validate backup file
    return false;
}

string BackupManager::generateBackupFileName() {
    return "backup_" + to_string(time(nullptr)) + ".db";
}

// ============================================================
// Reader -- delegates to Manager classes
// ============================================================
vector<Book> Reader::searchBook(const string& keyword) {
    // TODO: search books
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
// Admin -- delegates to Manager classes
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
