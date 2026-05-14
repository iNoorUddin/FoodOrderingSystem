#include "../include/Admin.h"
#include <iostream>

Admin::Admin(int id, const std::string& name, const std::string& email,
             const std::string& phone, const std::string& role,
             const std::string& passwordHash)
    : Person(id, name, email, phone), role(role), passwordHash(passwordHash) {}

Admin::~Admin() {}

void Admin::displayInfo() const {
    std::cout << "Admin ID: " << id << "\n"
              << "Name: " << name << "\n"
              << "Email: " << email << "\n"
              << "Role: " << role << "\n";
}

std::string Admin::getRole() const { return "Admin"; }
std::string Admin::getPasswordHash() const { return passwordHash; }
std::string Admin::getAdminRole() const { return role; }
