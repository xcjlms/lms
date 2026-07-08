#pragma once

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include "../LibrarySystem.h"

using namespace drogon;

class ApiController : public drogon::HttpController<ApiController> {
public:
    METHOD_LIST_BEGIN
        // Borrow / Return
        ADD_METHOD_TO(ApiController::borrowBook,   "/api/borrow",   Post);
        ADD_METHOD_TO(ApiController::returnBook,   "/api/return",   Post);

        // Overdue
        ADD_METHOD_TO(ApiController::getOverdue,   "/api/overdue",  Get);

        // Fines
        ADD_METHOD_TO(ApiController::calculateFine,"/api/fines/calculate", Post);
        ADD_METHOD_TO(ApiController::payFine,      "/api/fines/pay", Post);
        ADD_METHOD_TO(ApiController::getUserFines, "/api/fines",    Get);

        // Books
        ADD_METHOD_TO(ApiController::getBookList,  "/api/books",    Get);
        ADD_METHOD_TO(ApiController::addBook,      "/api/books",    Post);

        // Borrow history
        ADD_METHOD_TO(ApiController::getBorrowHistory, "/api/records", Get);
    METHOD_LIST_END

    // ----- Borrow / Return -----
    void borrowBook(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);
    void returnBook(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);

    // ----- Overdue -----
    void getOverdue(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback);

    // ----- Fines -----
    void calculateFine(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback);
    void payFine(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);
    void getUserFines(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback);

    // ----- Books -----
    void getBookList(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback);
    void addBook(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);

    // ----- History -----
    void getBorrowHistory(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& callback);

private:
    // Helper: build JSON success/failure response
    static HttpResponsePtr jsonResponse(bool success, const std::string& message);
    static HttpResponsePtr jsonResponse(bool success, const std::string& message, Json::Value& data);
    static HttpResponsePtr badRequest(const std::string& reason);
};
