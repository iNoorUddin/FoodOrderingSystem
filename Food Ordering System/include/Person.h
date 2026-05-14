#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <iostream>

using namespace std;


class Person {
protected:
    string name;
    string email;
    string phone;
    int id;

public:
    Person(int id, const string& name, const string& email, const string& phone);
    virtual ~Person();


    virtual void displayInfo() const = 0;
    virtual string getRole() const = 0;

    
    int getId() const;
    string getName() const;
    string getEmail() const;
    string getPhone() const;

    void setName(const string& name);
    void setEmail(const string& email);
    void setPhone(const string& phone);

    friend ostream& operator<<(ostream& os, const Person& person);
};

#endif
