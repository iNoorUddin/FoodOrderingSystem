#ifndef FOODSERVICE_H
#define FOODSERVICE_H

#include "MenuItem.h"
#include "Order.h"
#include "Customer.h"
#include "Admin.h"
#include "DatabaseManager.h"
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <stdexcept>

// Custom exceptions (Exception Handling)
class OrderException : public std::runtime_error {
public:
    explicit OrderException(const std::string& msg) : std::runtime_error(msg) {}
};

class AuthException : public std::runtime_error {
public:
    explicit AuthException(const std::string& msg) : std::runtime_error(msg) {}
};

// FoodService - main business logic class
// Demonstrates Friend class relationship with Order
class FoodService {
private:
    DatabaseManager* db;
    std::map<int, MenuItem> menuCache;  // STL map for caching
    std::vector<std::string> categories; // STL vector

    // Private helpers
    std::string hashPassword(const std::string& password);
    bool validateEmail(const std::string& email);
    bool validatePhone(const std::string& phone);

public:
    FoodService();
    ~FoodService();

    // Authentication
    Customer* loginCustomer(const std::string& email, const std::string& password);
    bool registerCustomer(const std::string& name, const std::string& email,
                          const std::string& phone, const std::string& address,
                          const std::string& password);
    Admin* loginAdmin(const std::string& email, const std::string& password);

    // Menu Management
    std::vector<MenuItem> getAllMenuItems();
    std::vector<MenuItem> getMenuByCategory(const std::string& category);
    std::vector<MenuItem> searchMenu(const std::string& keyword);
    MenuItem getMenuItemById(int itemId);
    bool addMenuItem(const MenuItem& item);
    bool updateMenuItem(const MenuItem& item);
    bool deleteMenuItem(int itemId);
    bool toggleMenuItemAvailability(int itemId);
    std::vector<std::string> getCategories();

    // Order Management
    int placeOrder(Order& order);
    bool updateOrderStatus(int orderId, OrderStatus status);
    Order getOrderById(int orderId);
    std::vector<Order> getOrdersByCustomer(int customerId);
    std::vector<Order> getPendingOrders();
    std::vector<Order> getAllOrders();
    bool cancelOrder(int orderId, int customerId);

    // Analytics (Admin)
    double getTotalRevenue();
    int getTotalOrders();
    std::vector<std::pair<std::string, int>> getTopSellingItems(int limit = 5);
    std::map<std::string, double> getRevenueByCategory();

    // Rating
    bool rateMenuItem(int customerId, int itemId, int rating);
};

#endif
