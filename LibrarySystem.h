#ifndef LIBRARY_SYSTEM_H
#define LIBRARY_SYSTEM_H

#include <vector>
#include <ctime>
#include <string>
#include <sqlite3.h>
using namespace std;

// Date/time utility functions (defined in LibrarySystem.c)
std::string timeToString(time_t t);
time_t stringToTime(const std::string& s);

#define FINE_PER_DAY 0.5      // 逾期每日罚款金额（元）
#define MAX_BORROW_LIMIT 5    // 最大借阅数量
#define BORROW_DAYS 30        // 标准借阅天数

// ==================== DatabaseManager ====================
class DatabaseManager {
public:
    static DatabaseManager& getInstance();
    bool open(const string& dbPath);
    void close();
    sqlite3* getConnection();
    bool execSQL(const string& sql);
    bool initTables();  // 创建数据库表
private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    sqlite3* db;
};

// ==================== User ====================
class User {
public:
    enum Role { READER, MANAGER };
    enum Status { ACTIVE, SUSPENDED };
    User() : userID(0), role(READER), status(ACTIVE) {}
    bool login(const string& username, const string& password);
    void logout() {}
    bool changePassword(const string& oldPwd, const string& newPwd);
    int getUserID() const { return userID; }
    string getUsername() const { return username; }
    Role getRole() const { return role; }
    Status getStatus() const { return status; }
protected:
    int userID;
    string username;
    string passwordHash;
    Role role;
    Status status;
};

// ==================== Book ====================
class Book {
public:
    enum Status { NORMAL, BORROWED, DELETED };
    Book() : bookID(0), totalStock(0), availableStock(0), status(NORMAL) {}
    bool isAvailable() const;
    int getBorrowedCount() const;
    void updateInfo(const string& title, const string& author,
                    const string& publisher, const string& location);
    // getters
    int getBookID() const { return bookID; }
    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getAvailableStock() const { return availableStock; }
    Book::Status getBookStatus() const { return status; }
private:
    int bookID;
    string isbn;
    string title;
    string author;
    string publisher;
    string categoryID;
    int totalStock;
    int availableStock;
    string location;
    Status status;
};

// ==================== BorrowRecord ====================
class BorrowRecord {
public:
    enum Status { BORROWED, RETURNED, OVERDUE };
    BorrowRecord() : recordID(0), bookID(0), userID(0),
                     borrowDate(0), dueDate(0), returnDate(0),
                     status(BORROWED) {}
    bool isOverdue() const;
    int calculateOverdueDays() const;
    void markReturned();
    // getters
    int getRecordID() const { return recordID; }
    int getBookID() const { return bookID; }
    int getUserID() const { return userID; }
    time_t getBorrowDate() const { return borrowDate; }
    time_t getDueDate() const { return dueDate; }
    time_t getReturnDate() const { return returnDate; }
    BorrowRecord::Status getBorrowStatus() const { return status; }
private:
    int recordID;
    int bookID;
    int userID;
    time_t borrowDate;
    time_t dueDate;
    time_t returnDate;
    Status status;
};

// ==================== FineRecord ====================
class FineRecord {
public:
    enum Status { UNPAID, PAID };
    FineRecord() : fineID(0), recordID(0), userID(0),
                   amount(0.0), status(UNPAID), createTime(0) {}
    // getters
    int getFineID() const { return fineID; }
    int getRecordID() const { return recordID; }
    int getUserID() const { return userID; }
    double getAmount() const { return amount; }
    FineRecord::Status getFineStatus() const { return status; }
private:
    int fineID;
    int recordID;
    int userID;
    double amount;
    Status status;
    time_t createTime;
};

// ==================== Feedback ====================
class Feedback {
public:
    enum Status { PENDING, ADDRESSED };
    Feedback() : feedbackID(0), userID(0), status(PENDING), submittedAt(0) {}
private:
    int feedbackID;
    int userID;
    string title;
    string content;
    time_t submittedAt;
    Status status;
    string reply;
};

// ==================== Admin / Reader ====================
class Admin;
class Reader;

// ==================== UserManager ====================
class UserManager {
public:
    bool registerUser(const string& username, const string& password, User::Role role);
    User* login(const string& username, const string& password);
    bool checkPermission(User* user, const string& action);
    bool changePassword(User* user, const string& oldPwd, const string& newPwd);
};

// ==================== InventoryManager ====================
class InventoryManager {
public:
    bool addBook(const Book& book);
    bool editBook(int bookID, const Book& newInfo);
    bool deleteBook(int bookID);
    bool increaseStock(int bookID, int quantity);
    bool decreaseStock(int bookID, int quantity);
    bool validateStock(int bookID);
    vector<Book> getInventoryList();
    vector<Book> getLowStockBooks(int threshold = 5);
};

// ==================== BorrowManager ====================
class Borrowmanager {
public:
    bool borrowBook(int userID, int bookID);
    bool returnBook(int userID, int bookID);
    vector<BorrowRecord> getBorrowHistory(int userID);
    vector<BorrowRecord> getAllBorrowRecords();
    bool checkBorrowLimit(int userID);
    // 逾期相关
    vector<BorrowRecord> getOverdueRecords();
    void sendOverdueReminders();
};

// ==================== FineManager ====================
class FineManager {
public:
    bool createFine(int recordID, int userID, double amount);
    double calculateFine(int recordID);
    bool payFine(int fineID);
    vector<FineRecord> getUserFines(int userID);
};

// ==================== FeedbackManager ====================
class FeedbackManager {
public:
    bool submitFeedback(int userID, const string& title, const string& content);
    vector<Feedback> getFeedbackList();
    bool handleFeedback(int feedbackID, const string& reply);
};

// ==================== BackupManager ====================
class BackupManager {
public:
    bool backupData(const string& backupPath);
    bool restoreData(const string& backupPath);
    bool validateBackupFile(const string& filePath);
    string generateBackupFileName();
};

// ==================== Reader ====================
class Reader : public User {
public:
    Reader() {}
    vector<Book> searchBook(const string& keyword);
    bool borrowBook(int bookID);
    bool returnBook(int bookID);
    vector<BorrowRecord> viewBorrowHistory();
    bool payFine(int fineID);
    bool submitFeedback(const string& title, const string& content);
};

// ==================== Admin ====================
class Admin : public User {
public:
    Admin() {}
    bool addBook(const Book& book);
    bool editBook(int bookID, const Book& newInfo);
    bool deleteBook(int bookID);
    bool adjustStock(int bookID, int quantity);
    bool backupData(const string& backupPath);
    bool restoreData(const string& backupPath);
    bool handleFeedback(int feedbackID, const string& reply);
};

#endif // LIBRARY_SYSTEM_H
