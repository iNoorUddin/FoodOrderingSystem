#ifndef ADMIN_H
#define ADMIN_H

#include "Person.h"
#include <string>

// Admin inherits from Person (Inheritance)
class Admin : public Person {
private:
    std::string role;
    std::string passwordHash;

public:
    Admin(int id, const std::string& name, const std::string& email,
          const std::string& phone, const std::string& role,
          const std::string& passwordHash);
    ~Admin() override;

    void displayInfo() const override;
    std::string getRole() const override;
    std::string getPasswordHash() const;
    std::string getAdminRole() const;
};

#endif
