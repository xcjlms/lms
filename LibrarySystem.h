#ifndef LIBRARY_SYSTEM_H
#define LIBRARY_SYSTEM_H
#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
using namespace std;

// Date/time utility functions (defined in LibrarySystem.cpp)
std::string timeToString(time_t t);
time_t stringToTime(const std::string& s);

#define FINE_PER_DAY 0.5
#define MAX_BORROW_LIMIT 5
#define BORROW_DAYS 30

// ==================== DatabaseManager ====================
class DatabaseManager {
public:
    static DatabaseManager& getInstance();
    bool open(const string& dbPath);
    void close();
    sqlite3* getConnection();
    bool execSQL(const string& sql);
    bool initTables();
private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    sqlite3* db;
};

// ==================== User ====================
class User {
    friend class UserManager;
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
    // ADDED: constructor for easy creation
    Book(int id, const string& isbn, const string& title, const string& author,
         const string& publisher, const string& category, int total, int avail,
         const string& loc, Status s)
        : bookID(id), isbn(isbn), title(title), author(author), publisher(publisher),
          categoryID(category), totalStock(total), availableStock(avail),
          location(loc), status(s) {}

    bool isAvailable() const;
    int getBorrowedCount() const;
    void updateInfo(const string& title, const string& author,
                    const string& publisher, const string& location);

    // Getters (all)
    int getBookID() const { return bookID; }
    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    string getCategoryID() const { return categoryID; }
    int getTotalStock() const { return totalStock; }
    int getAvailableStock() const { return availableStock; }
    string getLocation() const { return location; }
    Book::Status getBookStatus() const { return status; }

    // ADDED: setters for internal use
    void setBookID(int id) { bookID = id; }
    void setISBN(const string& s) { isbn = s; }
    void setTitle(const string& s) { title = s; }
    void setAuthor(const string& s) { author = s; }
    void setPublisher(const string& s) { publisher = s; }
    void setCategoryID(const string& s) { categoryID = s; }
    void setTotalStock(int n) { totalStock = n; }
    void setAvailableStock(int n) { availableStock = n; }
    void setLocation(const string& s) { location = s; }
    void setBookStatus(Status s) { status = s; }

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
    // ADDED: constructor
    BorrowRecord(int id, int book, int user, time_t bdate, time_t ddate,
                 time_t rdate, Status s)
        : recordID(id), bookID(book), userID(user), borrowDate(bdate),
          dueDate(ddate), returnDate(rdate), status(s) {}

    bool isOverdue() const;
    int calculateOverdueDays() const;
    void markReturned();

    // Getters
    int getRecordID() const { return recordID; }
    int getBookID() const { return bookID; }
    int getUserID() const { return userID; }
    time_t getBorrowDate() const { return borrowDate; }
    time_t getDueDate() const { return dueDate; }
    time_t getReturnDate() const { return returnDate; }
    BorrowRecord::Status getBorrowStatus() const { return status; }

    // ADDED: setters for construction
    void setRecordID(int id) { recordID = id; }
    void setBookID(int id) { bookID = id; }
    void setUserID(int id) { userID = id; }
    void setBorrowDate(time_t t) { borrowDate = t; }
    void setDueDate(time_t t) { dueDate = t; }
    void setReturnDate(time_t t) { returnDate = t; }
    void setBorrowStatus(Status s) { status = s; }

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
    // ADDED: constructor
    FineRecord(int fid, int rid, int uid, double amt, Status s, time_t ct)
        : fineID(fid), recordID(rid), userID(uid), amount(amt), status(s), createTime(ct) {}

    int getFineID() const { return fineID; }
    int getRecordID() const { return recordID; }
    int getUserID() const { return userID; }
    double getAmount() const { return amount; }
    FineRecord::Status getFineStatus() const { return status; }
    time_t getCreateTime() const { return createTime; }

    // ADDED: setters
    void setFineID(int id) { fineID = id; }
    void setRecordID(int id) { recordID = id; }
    void setUserID(int id) { userID = id; }
    void setAmount(double a) { amount = a; }
    void setFineStatus(Status s) { status = s; }
    void setCreateTime(time_t t) { createTime = t; }

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
    // ADDED: constructor
    Feedback(int fid, int uid, const string& title, const string& content,
             time_t at, Status s, const string& reply)
        : feedbackID(fid), userID(uid), title(title), content(content),
          submittedAt(at), status(s), reply(reply) {}

    // Getters
    int getFeedbackID() const { return feedbackID; }
    int getUserID() const { return userID; }
    string getTitle() const { return title; }
    string getContent() const { return content; }
    time_t getSubmittedAt() const { return submittedAt; }
    Status getFeedbackStatus() const { return status; }
    string getReply() const { return reply; }

    // ADDED: setters
    void setFeedbackID(int id) { feedbackID = id; }
    void setUserID(int id) { userID = id; }
    void setTitle(const string& s) { title = s; }
    void setContent(const string& s) { content = s; }
    void setSubmittedAt(time_t t) { submittedAt = t; }
    void setFeedbackStatus(Status s) { status = s; }
    void setReply(const string& s) { reply = s; }

private:
    int feedbackID;
    int userID;
    string title;
    string content;
    time_t submittedAt;
    Status status;
    string reply;
};

// ==================== Admin / Reader forward declarations ====================
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
    // Original overload (uses Book object)
    bool addBook(const Book& book);
    // ADDED: convenience overload with all fields
    bool addBook(const string& isbn, const string& title, const string& author,
                 const string& publisher, const string& categoryID,
                 int totalStock, const string& location);

    bool editBook(int bookID, const Book& newInfo);
    // ADDED: convenience overload for edit
    bool editBook(int bookID, const string& title, const string& author,
                  const string& publisher, const string& categoryID,
                  const string& location);

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
    bool borrowBook(int userID, int bookID, int durationDays = BORROW_DAYS);
    bool returnBook(int userID, int bookID);
    vector<BorrowRecord> getBorrowHistory(int userID);
    vector<BorrowRecord> getAllBorrowRecords();
    bool checkBorrowLimit(int userID);
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
