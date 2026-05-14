#undef byte
#include <mysql.h>
#include "../include/DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <vector>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() : connection(nullptr), port(3306) {}

DatabaseManager* DatabaseManager::getInstance() {
    if (!instance) instance = new DatabaseManager();
    return instance;
}

DatabaseManager::~DatabaseManager() { disconnect(); }

bool DatabaseManager::connect(const std::string& h, const std::string& u,
                               const std::string& p, const std::string& db, int prt) {
    host=h; user=u; password=p; dbName=db; port=prt;
    connection = mysql_init(nullptr);
    if (!connection) throw DatabaseException("Failed to initialize MySQL");
    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(),
                             nullptr, port, nullptr, 0)) {
        std::string err = mysql_error(connection);
        mysql_close(connection); connection = nullptr;
        throw DatabaseException("Connection failed: " + err);
    }
    std::string createDb = "CREATE DATABASE IF NOT EXISTS " + dbName;
    if (mysql_query(connection, createDb.c_str()))
        throw DatabaseException("Failed to create database");
    if (mysql_select_db(connection, dbName.c_str()))
        throw DatabaseException("Failed to select database");
    mysql_set_character_set(connection, "utf8");
    return initializeDatabase();
}

void DatabaseManager::disconnect() {
    if (connection) { mysql_close(connection); connection = nullptr; }
}

bool DatabaseManager::isConnected() const {
    return connection != nullptr && mysql_ping(connection) == 0;
}

MYSQL_RES* DatabaseManager::executeQuery(const std::string& query) {
    if (!connection) throw DatabaseException("Not connected");
    if (mysql_query(connection, query.c_str()))
        throw DatabaseException("Query failed: " + std::string(mysql_error(connection)));
    return mysql_store_result(connection);
}

bool DatabaseManager::executeUpdate(const std::string& query) {
    if (!connection) throw DatabaseException("Not connected");
    if (mysql_query(connection, query.c_str()))
        throw DatabaseException("Update failed: " + std::string(mysql_error(connection)));
    return true;
}

int DatabaseManager::getLastInsertId() {
    return (int)mysql_insert_id(connection);
}

std::string DatabaseManager::escapeString(const std::string& str) {
    if (!connection) return str;
    std::vector<char> buf(str.length() * 2 + 1);
    mysql_real_escape_string(connection, buf.data(), str.c_str(), str.length());
    return std::string(buf.data());
}

bool DatabaseManager::initializeDatabase() {
    std::vector<std::string> queries = {
        "CREATE TABLE IF NOT EXISTS customers (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(100) NOT NULL, email VARCHAR(100) UNIQUE NOT NULL, phone VARCHAR(20), address TEXT, password_hash VARCHAR(256) NOT NULL, loyalty_points DOUBLE DEFAULT 0, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",
        "CREATE TABLE IF NOT EXISTS admins (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(100) NOT NULL, email VARCHAR(100) UNIQUE NOT NULL, phone VARCHAR(20), role VARCHAR(50) DEFAULT 'Admin', password_hash VARCHAR(256) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",
        "CREATE TABLE IF NOT EXISTS menu_items (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(100) NOT NULL, description TEXT, price DOUBLE NOT NULL, category VARCHAR(50), image_path VARCHAR(255), available TINYINT(1) DEFAULT 1, prep_time_minutes INT DEFAULT 15, rating_sum DOUBLE DEFAULT 0, total_ratings INT DEFAULT 0, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",
        "CREATE TABLE IF NOT EXISTS orders (id INT AUTO_INCREMENT PRIMARY KEY, customer_id INT NOT NULL, status VARCHAR(30) DEFAULT 'Pending', payment_method VARCHAR(50) DEFAULT 'Cash on Delivery', delivery_address TEXT, total_amount DOUBLE DEFAULT 0, delivery_fee DOUBLE DEFAULT 50, discount DOUBLE DEFAULT 0, notes TEXT, estimated_delivery_time VARCHAR(100), order_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY (customer_id) REFERENCES customers(id))",
        "CREATE TABLE IF NOT EXISTS order_items (id INT AUTO_INCREMENT PRIMARY KEY, order_id INT NOT NULL, menu_item_id INT NOT NULL, quantity INT NOT NULL, unit_price DOUBLE NOT NULL, subtotal DOUBLE NOT NULL, special_instructions TEXT, FOREIGN KEY (order_id) REFERENCES orders(id), FOREIGN KEY (menu_item_id) REFERENCES menu_items(id))",
        "CREATE TABLE IF NOT EXISTS ratings (id INT AUTO_INCREMENT PRIMARY KEY, customer_id INT NOT NULL, menu_item_id INT NOT NULL, rating INT NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE KEY unique_rating (customer_id, menu_item_id))",
        "INSERT IGNORE INTO admins (name, email, phone, role, password_hash) VALUES ('Admin', 'admin@foodapp.com', '03001234567', 'Super Admin', '00000000000000000000000000000000000000000000000000000282c63bc483')",
        "INSERT IGNORE INTO menu_items (name, description, price, category, image_path, prep_time_minutes) VALUES ('Chicken Biryani','Aromatic basmati rice with tender chicken',350,'Rice Dishes',':/images/biryani.jpg',25),('Beef Burger','Juicy beef patty with fresh vegetables',280,'Burgers',':/images/burger.jpg',15),('Chicken Karahi','Traditional Pakistani karahi',420,'Curries',':/images/karahi.jpg',30),('Margherita Pizza','Classic pizza with mozzarella',450,'Pizza',':/images/pizza.jpg',20),('Seekh Kabab','Grilled minced meat kababs',220,'Starters',':/images/seekh.jpg',15),('French Fries','Crispy golden fries',120,'Sides',':/images/fries.jpg',10),('Mango Lassi','Chilled mango yogurt drink',90,'Drinks',':/images/lassi.jpg',5),('Naan','Freshly baked tandoor bread',30,'Bread',':/images/naan.jpg',10),('Mutton Nihari','Slow cooked mutton stew',480,'Curries',':/images/nihari.jpg',40),('Chocolate Brownie','Warm brownie with ice cream',180,'Desserts',':/images/brownie.jpg',10)"
    };
    for (const auto& q : queries) {
        try { executeUpdate(q); } catch (...) {}
    }
    return true;
}
