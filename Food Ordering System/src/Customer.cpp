#include "../include/Customer.h"
#include <iostream>

Customer::Customer(int id, const std::string& name, const std::string& email,
                   const std::string& phone, const std::string& address,
                   const std::string& passwordHash)
    : Person(id, name, email, phone), address(address),
      passwordHash(passwordHash), loyaltyPoints(0.0) {}

Customer::~Customer() {}

void Customer::displayInfo() const {
    std::cout << "Customer ID: " << id << "\n"
              << "Name: " << name << "\n"
              << "Email: " << email << "\n"
              << "Phone: " << phone << "\n"
              << "Address: " << address << "\n"
              << "Loyalty Points: " << loyaltyPoints << "\n";
}



std::string Customer::getRole() const { return "Customer"; }
std::string Customer::getAddress() const { return address; }
std::string Customer::getPasswordHash() const { return passwordHash; }
double Customer::getLoyaltyPoints() const { return loyaltyPoints; }

void Customer::setAddress(const std::string& addr) { address = addr; }
void Customer::addLoyaltyPoints(double points) { loyaltyPoints += points; }
void Customer::addOrderToHistory(int orderId) { orderHistory.push_back(orderId); }
std::vector<int> Customer::getOrderHistory() const { return orderHistory; }
