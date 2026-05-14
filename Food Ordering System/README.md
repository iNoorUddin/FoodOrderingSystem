# 🍽️ FoodExpress — Online Food Ordering System

**Subject:** Object-Oriented Programming  
**Project:** Complex Computing Activity (CCA)  
**Group Members:**
- Member 1 — [Your Name] — [Roll No]
- Member 2 — [Name] — [Roll No]
- Member 3 — [Name] — [Roll No]

---

## About The Project

FoodExpress is a fully functional online food ordering system built using C++ and the Qt6 GUI framework. It allows customers to browse a menu, add items to a cart, place orders, and track their order status — just like a real food delivery app. Admins can manage the menu, update order statuses, and view analytics.

We chose this project because food delivery apps are used by millions of people daily, and it gave us a great opportunity to apply all OOP concepts in a real-world scenario.

---

## Features

**For Customers:**
- Register and login securely
- Browse menu with food images and categories
- Search items by name
- Add items to cart with quantity control
- Place orders with multiple payment options
- View order history and track status
- Cancel pending orders
- Earn loyalty points on every order

**For Admins:**
- Manage menu items (add, edit, delete, toggle availability)
- Upload food photos for each item
- Update order statuses in real-time
- View analytics: revenue, total orders, top-selling items
- Export menu and orders to CSV files

---

## Technologies Used

| Technology | Purpose |
|-----------|---------|
| C++ 17 | Core programming language |
| Qt 6 | GUI framework |
| MySQL | Database backend |
| OpenSSL | Password hashing (SHA-256) |
| STL | Vectors, maps, algorithms |
| File I/O | Receipts, logs, CSV exports |

---

## OOP Concepts Implemented

| Concept | Where Used |
|---------|-----------|
| **Encapsulation** | All classes use private members with public getters/setters |
| **Inheritance** | `Customer` and `Admin` inherit from abstract `Person` |
| **Polymorphism** | `displayInfo()` and `getRole()` are virtual and overridden |
| **Abstract Class** | `Person` has pure virtual functions |
| **Composition** | `Order` contains `OrderItem` objects; `OrderItem` contains `MenuItem` |
| **Friend Function** | `operator<<` overloaded as friend in `Person`, `MenuItem` |
| **Constructors/Destructors** | All classes have proper constructors and virtual destructors |

---

## Additional Requirements

| Feature | Implementation |
|---------|---------------|
| **File Handling** | Receipt saving, CSV export, activity logging |
| **Exception Handling** | Custom `DatabaseException`, `OrderException`, `AuthException` |
| **STL Usage** | `std::vector`, `std::map`, `std::find_if`, `std::remove_if` |
| **GUI** | Qt6 with dark theme, image cards, animated notifications |
| **Database** | MySQL with full CRUD, foreign keys, triggers |

---

## How to Run

See `docs/ExecutionInstructions.md` for complete setup steps.

**Quick start:**
1. Install Qt6, MySQL, OpenSSL
2. Run `database/setup.sql` in MySQL
3. Open project in Qt Creator and press Run

**Default admin login:** `admin@foodapp.com` / `admin123`

---

## Project Structure

```
FoodOrderingSystem/
├── include/          Header files for all classes
├── src/              Implementation files + main.cpp
├── resources/        Qt resources (images, icons)
│   └── images/       Food item photos
├── database/         MySQL schema and seed data
└── docs/             Execution guide
```

---

## Contribution Statement

| Member | Contribution |
|--------|-------------|
| [Member 1] | OOP class design, Person/Customer/Admin hierarchy, DatabaseManager |
| [Member 2] | Order management, FoodService business logic, FileManager |
| [Member 3] | Qt GUI (MainWindow), styling, testing, documentation |

*All members contributed equally to the overall design and testing.*

---

## Screenshots

*(Add screenshots of your running application here)*

---

> **Note:** This project was developed for academic purposes as part of the CCA assignment.
