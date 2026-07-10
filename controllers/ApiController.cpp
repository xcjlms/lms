#include "ApiController.h"
#include <iostream>
#include <json/json.h>
#include <cstdlib>

using namespace std;
static string join(const vector<string>& parts, const string& delim) {
    string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) result += delim;
        result += parts[i];
    }
    return result;
}
// ============================================================
// Helper: build JSON responses
// ============================================================
HttpResponsePtr ApiController::jsonResponse(bool success, const string& msg) {
    Json::Value json;
    json["success"] = success;
    json["message"] = msg;
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    return resp;
}

HttpResponsePtr ApiController::jsonResponse(bool success, const string& msg, Json::Value& data) {
    Json::Value json;
    json["success"] = success;
    json["message"] = msg;
    json["data"] = data;
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    return resp;
}

HttpResponsePtr ApiController::badRequest(const string& reason) {
    Json::Value json;
    json["success"] = false;
    json["message"] = reason;
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(k400BadRequest);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    return resp;
}

// ============================================================
// POST /api/login
// Body: { "username": "alice", "password": "xxx" }
// ============================================================
void ApiController::login(const HttpRequestPtr& req,
                           function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["username"].isString() || !(*json)["password"].isString()) {
        callback(badRequest("Requires username and password"));
        return;
    }

    string username = (*json)["username"].asString();
    string password = (*json)["password"].asString();

    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    // Escape username
    string safeUser;
    for (char c : username) { if (c == '\'') safeUser += "''"; else safeUser += c; }

    string sql = "SELECT user_id, username, role, status, password_hash "
                 "FROM users WHERE username = '" + safeUser + "'";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "Database error"));
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        callback(jsonResponse(false, "User not found"));
        return;
    }

    int    userId   = sqlite3_column_int(stmt, 0);
    string uname    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    string role     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    string status   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    string hash     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    sqlite3_finalize(stmt);

    if (status != "ACTIVE") {
        callback(jsonResponse(false, "Account is suspended"));
        return;
    }

    // Simple password check (plaintext for now — upgrade to hashed later)
    if (password != hash) {
        callback(jsonResponse(false, "Incorrect password"));
        return;
    }

    Json::Value data;
    data["user_id"]  = userId;
    data["username"] = uname;
    data["role"]     = role;
    callback(jsonResponse(true, "Login successful", data));
}

// ============================================================
// POST /api/register
// Body: { "username": "newuser", "password": "xxx", "role": "READER" }
// ============================================================
void ApiController::reg(const HttpRequestPtr& req,
                         function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["username"].isString() || !(*json)["password"].isString()) {
        callback(badRequest("Requires username and password"));
        return;
    }

    string username = (*json)["username"].asString();
    string password = (*json)["password"].asString();
    string role     = json->get("role", "READER").asString();

    if (role != "READER" && role != "MANAGER") {
        callback(badRequest("Role must be READER or MANAGER"));
        return;
    }

    // Escape single quotes
    auto esc = [](const string& s) -> string {
        string out; for (char c : s) { if (c == '\'') out += "''"; else out += c; }
        return out;
    };

    string sql = "INSERT INTO users (username, password_hash, role, status) VALUES ('" +
                 esc(username) + "', '" + esc(password) + "', '" + role + "', 'ACTIVE')";

    if (DatabaseManager::getInstance().execSQL(sql)) {
        Json::Value data;
        data["username"] = username;
        data["role"] = role;
        callback(jsonResponse(true, "User registered successfully", data));
    } else {
        callback(jsonResponse(false, "Registration failed — username may already exist"));
    }
}

// ============================================================
// POST /api/borrow
// Body: { "user_id": 2, "book_id": 1 }
// ============================================================
void ApiController::borrowBook(const HttpRequestPtr& req,
                                function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["user_id"].isInt() || !(*json)["book_id"].isInt()) {
        callback(badRequest("Requires JSON body with user_id and book_id"));
        return;
    }

    int userId = (*json)["user_id"].asInt();
    int bookId = (*json)["book_id"].asInt();

    Borrowmanager bm;
    bool ok = bm.borrowBook(userId, bookId);

    if (ok)
        callback(jsonResponse(true, "Book borrowed successfully"));
    else
        callback(jsonResponse(false, "Failed to borrow book (check server log for details)"));
}

// ============================================================
// POST /api/return
// Body: { "user_id": 2, "book_id": 1 }
// ============================================================
void ApiController::returnBook(const HttpRequestPtr& req,
                                function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["user_id"].isInt() || !(*json)["book_id"].isInt()) {
        callback(badRequest("Requires JSON body with user_id and book_id"));
        return;
    }

    int userId = (*json)["user_id"].asInt();
    int bookId = (*json)["book_id"].asInt();

    Borrowmanager bm;
    bool ok = bm.returnBook(userId, bookId);

    if (ok)
        callback(jsonResponse(true, "Book returned successfully"));
    else
        callback(jsonResponse(false, "Failed to return book (check server log for details)"));
}

// ============================================================
// GET /api/overdue
// ============================================================
void ApiController::getOverdue(const HttpRequestPtr& req,
                                function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
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
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value records(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value rec;
        rec["record_id"]      = sqlite3_column_int(stmt, 0);
        rec["book_id"]        = sqlite3_column_int(stmt, 1);
        rec["user_id"]        = sqlite3_column_int(stmt, 2);
        rec["borrow_date"]    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        rec["due_date"]       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec["username"]       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        rec["book_title"]     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        // Calculate overdue days and fine
        time_t dueTime = stringToTime(rec["due_date"].asString());
        time_t now = time(nullptr);
        int overdueDays = static_cast<int>(difftime(now, dueTime) / (24 * 3600)) + 1;
        rec["overdue_days"]   = overdueDays;
        rec["accrued_fine"]   = overdueDays * FINE_PER_DAY;

        records.append(rec);
    }
    sqlite3_finalize(stmt);

    Json::Value msg;
    msg["count"] = records.size();
    callback(jsonResponse(true, "OK", records));
}

// ============================================================
// POST /api/fines/calculate
// Body: { "record_id": 2 }
// ============================================================
void ApiController::calculateFine(const HttpRequestPtr& req,
                                   function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["record_id"].isInt()) {
        callback(badRequest("Requires JSON body with record_id"));
        return;
    }

    int recordId = (*json)["record_id"].asInt();
    FineManager fm;
    double fine = fm.calculateFine(recordId);

    Json::Value data;
    data["record_id"] = recordId;
    data["fine"] = fine;
    callback(jsonResponse(true, "Fine calculated", data));
}

// ============================================================
// POST /api/fines/pay
// Body: { "fine_id": 1 }
// ============================================================
void ApiController::payFine(const HttpRequestPtr& req,
                             function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["fine_id"].isInt()) {
        callback(badRequest("Requires JSON body with fine_id"));
        return;
    }

    int fineId = (*json)["fine_id"].asInt();
    FineManager fm;
    bool ok = fm.payFine(fineId);

    if (ok)
        callback(jsonResponse(true, "Fine paid successfully"));
    else
        callback(jsonResponse(false, "Failed to pay fine (check server log)"));
}

// ============================================================
// GET /api/fines?user_id=2
// ============================================================
void ApiController::getUserFines(const HttpRequestPtr& req,
                                  function<void(const HttpResponsePtr&)>&& callback) {
    auto params = req->getParameters();
    if (!params.count("user_id")) {
        callback(badRequest("Query parameter 'user_id' is required"));
        return;
    }

    int userId = stoi(params["user_id"]);
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string sql = "SELECT fine_id, record_id, user_id, amount, status, create_time "
                 "FROM fine_records WHERE user_id = " + to_string(userId) +
                 " ORDER BY create_time DESC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value fines(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value f;
        f["fine_id"]     = sqlite3_column_int(stmt, 0);
        f["record_id"]   = sqlite3_column_int(stmt, 1);
        f["user_id"]     = sqlite3_column_int(stmt, 2);
        f["amount"]      = sqlite3_column_double(stmt, 3);
        f["status"]      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        f["create_time"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        fines.append(f);
    }
    sqlite3_finalize(stmt);

    Json::Value data;
    data["fines"] = fines;
    data["count"] = fines.size();
    callback(jsonResponse(true, "OK", data));
}

// ============================================================
// GET /api/books?keyword=xxx (optional search)
// ============================================================
void ApiController::getBookList(const HttpRequestPtr& req,
                                 function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    auto params = req->getParameters();
    string keyword = params.count("keyword") ? params["keyword"] : "";

    string sql;
    if (keyword.empty()) {
    sql = "SELECT book_id, isbn, title, author, publisher, "
          "total_stock, available_stock, location, status FROM books "
          "WHERE status != 'DELETED'";
} else {
    sql = "SELECT book_id, isbn, title, author, publisher, "
          "total_stock, available_stock, location, status FROM books "
          "WHERE status != 'DELETED' AND "
          "(title LIKE '%" + keyword + "%' OR author LIKE '%" + keyword + "%')";
}
sql += " ORDER BY book_id";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value books(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value b;
        b["book_id"]         = sqlite3_column_int(stmt, 0);
        b["isbn"]            = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        b["title"]           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        b["author"]          = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        b["publisher"]       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        b["total_stock"]     = sqlite3_column_int(stmt, 5);
        b["available_stock"] = sqlite3_column_int(stmt, 6);
        b["location"]        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        b["status"]          = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        books.append(b);
    }
    sqlite3_finalize(stmt);

    Json::Value data;
    data["books"] = books;
    data["count"] = books.size();
    callback(jsonResponse(true, "OK", data));
}

// ============================================================
// POST /api/books
// Body:
// {
//   "isbn": "978-xxx",
//   "title": "Book Title",
//   "author": "Author",
//   "publisher": "Publisher",
//   "category_id": "CS",
//   "total_stock": 5,
//   "location": "A-01"
// }
// ============================================================
void ApiController::addBook(const HttpRequestPtr& req,
                            function<void(const HttpResponsePtr&)>&& callback) {
    Json::Value emptyData;
    auto json = req->getJsonObject();

    if (!json) {
        callback(badRequest("Requires JSON body"));
        return;
    }

    if (!(*json)["isbn"].isString() ||
        !(*json)["title"].isString() ||
        !(*json)["author"].isString() ||
        !(*json)["total_stock"].isInt()) {
        callback(badRequest("Requires isbn, title, author and total_stock"));
        return;
    }

    string isbn = (*json)["isbn"].asString();
    string title = (*json)["title"].asString();
    string author = (*json)["author"].asString();
    string publisher = (*json).isMember("publisher") ? (*json)["publisher"].asString() : "";
    string categoryID = (*json).isMember("category_id") ? (*json)["category_id"].asString() : "";
    string location = (*json).isMember("location") ? (*json)["location"].asString() : "";
    int totalStock = (*json)["total_stock"].asInt();

    InventoryManager manager;
    bool ok = manager.addBook(isbn, title, author, publisher, categoryID, totalStock, location);

    if (ok) {
        callback(jsonResponse(true, "Book added successfully"));
    } else {
        callback(jsonResponse(false, "Failed to add book"));
    }
}

// ============================================================
// GET /api/records?user_id=2
// ============================================================
void ApiController::getBorrowHistory(const HttpRequestPtr& req,
                                      function<void(const HttpResponsePtr&)>&& callback) {
    auto params = req->getParameters();
    if (!params.count("user_id")) {
        callback(badRequest("Query parameter 'user_id' is required"));
        return;
    }

    int userId = stoi(params["user_id"]);
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string sql = "SELECT br.record_id, br.book_id, br.user_id, "
                 "       br.borrow_date, br.due_date, br.return_date, br.status, "
                 "       b.title, b.isbn "
                 "FROM borrow_records br "
                 "JOIN books b ON br.book_id = b.book_id "
                 "WHERE br.user_id = " + to_string(userId) +
                 " ORDER BY br.borrow_date DESC";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }
    Json::Value records(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value rec;
        rec["record_id"]   = sqlite3_column_int(stmt, 0);
        rec["book_id"]     = sqlite3_column_int(stmt, 1);
        rec["user_id"]     = sqlite3_column_int(stmt, 2);
        rec["borrow_date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        rec["due_date"]    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL)
            rec["return_date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        else
            rec["return_date"] = Json::nullValue;
        rec["status"]      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        rec["book_title"]  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        rec["isbn"]        = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        records.append(rec);
    }

    sqlite3_finalize(stmt);

    Json::Value data;
    data["records"] = records;
    data["count"] = records.size();
    callback(jsonResponse(true, "OK", data));
}

// ============================================================
// PUT /api/books/{book_id}
// Body:
// {
//   "title": "New Title",
//   "author": "New Author",
//   "publisher": "New Publisher",
//   "category_id": "CS",
//   "location": "A-02"
// }
// ============================================================
void ApiController::editBook(const HttpRequestPtr& req,
                             function<void(const HttpResponsePtr&)>&& callback,
                             int bookID) {
    auto json = req->getJsonObject();

    if (!json) {
        callback(badRequest("Requires JSON body"));
        return;
    }

    if (!(*json)["title"].isString() ||
        !(*json)["author"].isString()) {
        callback(badRequest("Requires title and author"));
        return;
    }

    string title = (*json)["title"].asString();
    string author = (*json)["author"].asString();
    string publisher = (*json).isMember("publisher") ? (*json)["publisher"].asString() : "";
    string categoryID = (*json).isMember("category_id") ? (*json)["category_id"].asString() : "";
    string location = (*json).isMember("location") ? (*json)["location"].asString() : "";

    InventoryManager manager;
    bool ok = manager.editBook(bookID, title, author, publisher, categoryID, location);

    if (ok) {
        callback(jsonResponse(true, "Book edited successfully"));
    } else {
        callback(jsonResponse(false, "Failed to edit book"));
    }
}

// ============================================================
// DELETE /api/books/{book_id}
// ============================================================
void ApiController::deleteBook(const HttpRequestPtr& req,
                               function<void(const HttpResponsePtr&)>&& callback,
                               int bookID) {
    InventoryManager manager;
    bool ok = manager.deleteBook(bookID);

    if (ok) {
        callback(jsonResponse(true, "Book deleted successfully"));
    } else {
        callback(jsonResponse(false, "Failed to delete book. It may have active borrow records."));
    }
}

// ============================================================
// POST /api/books/{book_id}/stock/increase
// Body: { "quantity": 3 }
// ============================================================
void ApiController::increaseStock(const HttpRequestPtr& req,
                                  function<void(const HttpResponsePtr&)>&& callback,
                                  int bookID) {
    auto json = req->getJsonObject();

    if (!json || !(*json)["quantity"].isInt()) {
        callback(badRequest("Requires JSON body with quantity"));
        return;
    }

    int quantity = (*json)["quantity"].asInt();

    InventoryManager manager;
    bool ok = manager.increaseStock(bookID, quantity);

    if (ok) {
        callback(jsonResponse(true, "Stock increased successfully"));
    } else {
        callback(jsonResponse(false, "Failed to increase stock"));
    }
}

// ============================================================
// POST /api/books/{book_id}/stock/decrease
// Body: { "quantity": 2 }
// ============================================================
void ApiController::decreaseStock(const HttpRequestPtr& req,
                                  function<void(const HttpResponsePtr&)>&& callback,
                                  int bookID) {
    auto json = req->getJsonObject();

    if (!json || !(*json)["quantity"].isInt()) {
        callback(badRequest("Requires JSON body with quantity"));
        return;
    }

    int quantity = (*json)["quantity"].asInt();

    InventoryManager manager;
    bool ok = manager.decreaseStock(bookID, quantity);

    if (ok) {
        callback(jsonResponse(true, "Stock decreased successfully"));
    } else {
        callback(jsonResponse(false, "Failed to decrease stock"));
    }
}

// ============================================================
// GET /api/inventory
// ============================================================
void ApiController::getInventory(const HttpRequestPtr& req,
                                 function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string sql =
        "SELECT book_id, isbn, title, author, publisher, category_id, "
        "total_stock, available_stock, location, status "
        "FROM books WHERE status != 'DELETED' "
        "ORDER BY book_id ASC";

    sqlite3_stmt* stmt = NULL;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value books(Json::arrayValue);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value book;

        book["book_id"] = sqlite3_column_int(stmt, 0);
        book["isbn"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        book["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        book["author"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        book["publisher"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        book["category_id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        book["total_stock"] = sqlite3_column_int(stmt, 6);
        book["available_stock"] = sqlite3_column_int(stmt, 7);
        book["borrowed_count"] = sqlite3_column_int(stmt, 6) - sqlite3_column_int(stmt, 7);
        book["location"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        book["status"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        books.append(book);
    }

    sqlite3_finalize(stmt);

    Json::Value data;
    data["books"] = books;
    data["count"] = books.size();

    callback(jsonResponse(true, "OK", data));
}

// ============================================================
// GET /api/inventory/low?threshold=2
// ============================================================
void ApiController::getLowStockBooks(const HttpRequestPtr& req,
                                     function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();

    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    auto params = req->getParameters();

    int threshold = 5;

    if (params.count("threshold")) {
        threshold = atoi(params["threshold"].c_str());
    }

    string sql =
        "SELECT book_id, isbn, title, author, publisher, category_id, "
        "total_stock, available_stock, location, status "
        "FROM books "
        "WHERE status != 'DELETED' AND available_stock <= " + to_string(threshold) +
        " ORDER BY available_stock ASC";

    sqlite3_stmt* stmt = NULL;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value books(Json::arrayValue);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value book;

        book["book_id"] = sqlite3_column_int(stmt, 0);
        book["isbn"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        book["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        book["author"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        book["publisher"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        book["category_id"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        book["total_stock"] = sqlite3_column_int(stmt, 6);
        book["available_stock"] = sqlite3_column_int(stmt, 7);
        book["borrowed_count"] = sqlite3_column_int(stmt, 6) - sqlite3_column_int(stmt, 7);
        book["location"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        book["status"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        books.append(book);
    }

    sqlite3_finalize(stmt);

    Json::Value data;
    data["threshold"] = threshold;
    data["books"] = books;
    data["count"] = books.size();

    callback(jsonResponse(true, "OK", data));
}

// ============================================================
// POST /api/backup
// Body optional: { "backup_path": "library_backup.db" }
// ============================================================
void ApiController::backupData(const HttpRequestPtr& req,
                               function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();

    string backupPath = "";

    if (json && (*json).isMember("backup_path") && (*json)["backup_path"].isString()) {
        backupPath = (*json)["backup_path"].asString();
    }

    BackupManager manager;

    if (backupPath.empty()) {
        backupPath = manager.generateBackupFileName();
    }

    bool ok = manager.backupData(backupPath);

    Json::Value data;
    data["backup_path"] = backupPath;

    if (ok) {
        callback(jsonResponse(true, "Backup created successfully", data));
    } else {
        callback(jsonResponse(false, "Failed to create backup", data));
    }
}

// ============================================================
// POST /api/restore
// Body: { "backup_path": "library_backup_20260709_085200.db" }
// ============================================================
void ApiController::restoreData(const HttpRequestPtr& req,
                                function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();

    if (!json || !(*json)["backup_path"].isString()) {
        callback(badRequest("Requires JSON body with backup_path"));
        return;
    }

    string backupPath = (*json)["backup_path"].asString();

    BackupManager manager;
    bool ok = manager.restoreData(backupPath);

    if (ok) {
        callback(jsonResponse(true, "Database restored successfully"));
    } else {
        callback(jsonResponse(false, "Failed to restore database"));
    }
}
// ========== 用户管理 ==========
void ApiController::getUsers(const HttpRequestPtr& req,
                             function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string sql = "SELECT user_id, username, role, status FROM users ORDER BY user_id";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value users(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value u;
        u["user_id"]  = sqlite3_column_int(stmt, 0);
        u["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u["role"]     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u["status"]   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        users.append(u);
    }
    sqlite3_finalize(stmt);

    Json::Value data;
    data["users"] = users;
    data["count"] = users.size();
    callback(jsonResponse(true, "OK", data));
}

void ApiController::updateUser(const HttpRequestPtr& req,
                               function<void(const HttpResponsePtr&)>&& callback,
                               int userID) {
    auto json = req->getJsonObject();
    if (!json) {
        callback(badRequest("Requires JSON body"));
        return;
    }

    string role = json->get("role", "").asString();
    string status = json->get("status", "").asString();

    if (role.empty() && status.empty()) {
        callback(badRequest("At least one field (role/status) required"));
        return;
    }

    vector<string> setParts;
    if (!role.empty()) {
        if (role != "READER" && role != "MANAGER") {
            callback(badRequest("Role must be READER or MANAGER"));
            return;
        }
        setParts.push_back("role = '" + role + "'");
    }
    if (!status.empty()) {
        if (status != "ACTIVE" && status != "SUSPENDED") {
            callback(badRequest("Status must be ACTIVE or SUSPENDED"));
            return;
        }
        setParts.push_back("status = '" + status + "'");
    }

    string sql = "UPDATE users SET " + join(setParts, ", ") + " WHERE user_id = " + to_string(userID);
    bool ok = DatabaseManager::getInstance().execSQL(sql);
    callback(ok ? jsonResponse(true, "User updated") : jsonResponse(false, "Update failed"));
}

// ========== 反馈 ==========
void ApiController::submitFeedback(const HttpRequestPtr& req,
                                   function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["title"].isString() || !(*json)["content"].isString()) {
        callback(badRequest("Requires title and content"));
        return;
    }
    // 从请求中获取用户ID（通常从session/token获取，此处简化：从body获取或默认）
    int userID = json->get("user_id", 0).asInt();
    if (userID <= 0) {
        callback(badRequest("user_id is required"));
        return;
    }
    string title = (*json)["title"].asString();
    string content = (*json)["content"].asString();

    FeedbackManager fm;
    bool ok = fm.submitFeedback(userID, title, content);
    callback(ok ? jsonResponse(true, "Feedback submitted") : jsonResponse(false, "Submit failed"));
}

void ApiController::getFeedbackList(const HttpRequestPtr& req,
                                    function<void(const HttpResponsePtr&)>&& callback) {
    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string sql = "SELECT feedback_id, user_id, title, content, submitted_at, status, reply FROM feedback ORDER BY submitted_at DESC";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        callback(jsonResponse(false, "SQL error"));
        return;
    }

    Json::Value items(Json::arrayValue);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Json::Value f;
        f["feedback_id"] = sqlite3_column_int(stmt, 0);
        f["user_id"]     = sqlite3_column_int(stmt, 1);
        f["title"]       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        f["content"]     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        f["submitted_at"]= reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        f["status"]      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        f["reply"]       = (sqlite3_column_type(stmt, 6) != SQLITE_NULL) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        items.append(f);
    }
    sqlite3_finalize(stmt);

    Json::Value data;
    data["list"] = items;
    data["count"] = items.size();
    callback(jsonResponse(true, "OK", data));
}

void ApiController::replyFeedback(const HttpRequestPtr& req,
                                  function<void(const HttpResponsePtr&)>&& callback,
                                  int feedbackID) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["reply"].isString()) {
        callback(badRequest("Requires reply text"));
        return;
    }
    string reply = (*json)["reply"].asString();
    FeedbackManager fm;
    bool ok = fm.handleFeedback(feedbackID, reply);
    callback(ok ? jsonResponse(true, "Reply sent") : jsonResponse(false, "Reply failed"));
}
