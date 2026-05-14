#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Person.h"
#include <vector>
#include <string>

// Customer inherits from Person (Inheritance)
class Customer : public Person {
private:
    std::string address;
    std::string passwordHash;
    double loyaltyPoints;
    std::vector<int> orderHistory; // stores order IDs

public:
    Customer(int id, const std::string& name, const std::string& email,
             const std::string& phone, const std::string& address,
             const std::string& passwordHash);
    ~Customer() override;

    // Override pure virtual functions
    void displayInfo() const override;
    std::string getRole() const override;

    std::string getAddress() const;
    std::string getPasswordHash() const;
    double getLoyaltyPoints() const;

    void setAddress(const std::string& address);
    void addLoyaltyPoints(double points);
    void addOrderToHistory(int orderId);
    std::vector<int> getOrderHistory() const;
};

#endif
