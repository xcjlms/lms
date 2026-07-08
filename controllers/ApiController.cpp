#include "ApiController.h"
#include <iostream>
#include <json/json.h>

using namespace std;

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
              "total_stock, available_stock, location, status FROM books";
    } else {
        // Simple search: match title or author
        sql = "SELECT book_id, isbn, title, author, publisher, "
              "total_stock, available_stock, location, status FROM books "
              "WHERE title LIKE '%" + keyword + "%' OR author LIKE '%" + keyword + "%'";
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
// Body: { "isbn": "...", "title": "...", "author": "...", ... }
// ============================================================
void ApiController::addBook(const HttpRequestPtr& req,
                             function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["title"].isString() || !(*json)["author"].isString()) {
        callback(badRequest("Requires JSON body with title and author at minimum"));
        return;
    }

    sqlite3* db = DatabaseManager::getInstance().getConnection();
    if (!db) {
        callback(jsonResponse(false, "Database not connected"));
        return;
    }

    string isbn    = json->get("isbn", "").asString();
    string title   = (*json)["title"].asString();
    string author  = (*json)["author"].asString();
    string pub     = json->get("publisher", "").asString();
    string cat     = json->get("category_id", "").asString();
    string loc     = json->get("location", "").asString();
    int stock      = json->get("total_stock", 1).asInt();

    // Escape single quotes for SQL
    auto esc = [](const string& s) -> string {
        string out;
        for (char c : s) { if (c == '\'') out += "''"; else out += c; }
        return out;
    };

    string sql = "INSERT INTO books (isbn, title, author, publisher, category_id, "
                 "total_stock, available_stock, location, status) VALUES ('" +
                 esc(isbn) + "', '" + esc(title) + "', '" + esc(author) + "', '" +
                 esc(pub) + "', '" + esc(cat) + "', " + to_string(stock) + ", " +
                 to_string(stock) + ", '" + esc(loc) + "', 'NORMAL')";

    bool ok = DatabaseManager::getInstance().execSQL(sql);

    Json::Value data;
    data["title"] = title;
    data["author"] = author;
    if (ok)
        callback(jsonResponse(true, "Book added successfully", data));
    else
        callback(jsonResponse(false, "Failed to add book"));
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
