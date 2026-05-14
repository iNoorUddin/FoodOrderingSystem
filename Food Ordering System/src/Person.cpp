#include "../include/Person.h"
using namespace std;


Person::Person(int id, const string& name, const string& email, const string& phone)
    : id(id), name(name), email(email), phone(phone) {}

Person::~Person() {}

int Person::getId() const { return id; }
string Person::getName() const { return name; }
string Person::getEmail() const { return email; }
string Person::getPhone() const { return phone; }

void Person::setName(const string& n) { name = n; }
void Person::setEmail(const string& e) { email = e; }
void Person::setPhone(const string& p) { phone = p; }

ostream& operator<<(ostream& os, const Person& person) {
    os << "[" << person.getRole() << "] ID: " << person.id
       << ", Name: " << person.name << ", Email: " << person.email;
    return os;
}
