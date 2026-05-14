#include "../include/FoodService.h"
#include <sstream>
#include <algorithm>
#include <functional>
#include <iomanip>


FoodService::FoodService() {
    db = DatabaseManager::getInstance();
}

FoodService::~FoodService() {}

std::string FoodService::hashPassword(const std::string& password) {
    std::size_t hash = 0;
    for (char c : password) {
        hash = hash * 31 + c;
    }
    std::ostringstream oss;
    oss << std::hex << hash;
    std::string result = oss.str();
    while (result.length() < 64) result = "0" + result;
    return result;
}

bool FoodService::validateEmail(const std::string& email) {
    return email.find('@') != std::string::npos && email.find('.') != std::string::npos;
}

bool FoodService::validatePhone(const std::string& phone) {
    return phone.length() >= 10;
}

Customer* FoodService::loginCustomer(const std::string& email, const std::string& password) {
    std::string hashed = hashPassword(password);
    std::string escapedEmail = db->escapeString(email);
    std::string query = "SELECT id, name, email, phone, address, loyalty_points FROM customers "
                        "WHERE email='" + escapedEmail + "' AND password_hash='" + hashed + "'";
    MYSQL_RES* res = db->executeQuery(query);
    if (!res) throw AuthException("Login failed");
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        throw AuthException("Invalid email or password");
    }
    Customer* c = new Customer(
        atoi(row[0]), row[1], row[2], row[3] ? row[3] : "",
        row[4] ? row[4] : "", hashed
    );
    if (row[5]) c->addLoyaltyPoints(atof(row[5]));
    mysql_free_result(res);
    return c;
}

bool FoodService::registerCustomer(const std::string& name, const std::string& email,
                                    const std::string& phone, const std::string& address,
                                    const std::string& password) {
    if (!validateEmail(email)) throw AuthException("Invalid email format");
    if (!validatePhone(phone)) throw AuthException("Invalid phone number");
    if (password.length() < 6) throw AuthException("Password must be at least 6 characters");

    std::string hashed = hashPassword(password);
    std::string query = "INSERT INTO customers (name, email, phone, address, password_hash) VALUES ('"
        + db->escapeString(name) + "','" + db->escapeString(email) + "','"
        + db->escapeString(phone) + "','" + db->escapeString(address) + "','" + hashed + "')";
    try {
        db->executeUpdate(query);
        return true;
    } catch (const DatabaseException& e) {
        throw AuthException("Registration failed: Email may already exist");
    }
}

Admin* FoodService::loginAdmin(const std::string& email, const std::string& password) {
    std::string hashed = hashPassword(password);
    std::string query = "SELECT id, name, email, phone, role FROM admins "
                        "WHERE email='" + db->escapeString(email) + "' AND password_hash='" + hashed + "'";
    MYSQL_RES* res = db->executeQuery(query);
    if (!res) throw AuthException("Admin login failed");
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        throw AuthException("Invalid admin credentials");
    }
    Admin* a = new Admin(atoi(row[0]), row[1], row[2], row[3] ? row[3] : "", row[4], hashed);
    mysql_free_result(res);
    return a;
}

std::vector<MenuItem> FoodService::getAllMenuItems() {
    std::vector<MenuItem> items;
    MYSQL_RES* res = db->executeQuery(
        "SELECT id, name, description, price, category, image_path, available, prep_time_minutes, rating_sum, total_ratings "
        "FROM menu_items ORDER BY category, name"
    );
    if (!res) return items;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        MenuItem item(atoi(row[0]), row[1], row[2] ? row[2] : "",
                      atof(row[3]), row[4] ? row[4] : "", row[5] ? row[5] : "",
                      atoi(row[6]), atoi(row[7]));
        // Set ratings
        int totalR = atoi(row[9]);
        double sumR = atof(row[8]);
        for (int i = 0; i < totalR; i++) item.addRating((int)(sumR / (totalR > 0 ? totalR : 1)));
        items.push_back(item);
        menuCache[item.getItemId()] = item;
    }
    mysql_free_result(res);
    return items;
}

std::vector<MenuItem> FoodService::getMenuByCategory(const std::string& category) {
    std::vector<MenuItem> all = getAllMenuItems();
    std::vector<MenuItem> filtered;
    std::copy_if(all.begin(), all.end(), std::back_inserter(filtered), [&](const MenuItem& m){
        return m.getCategory() == category && m.isAvailable();
    });
    return filtered;
}

std::vector<MenuItem> FoodService::searchMenu(const std::string& keyword) {
    std::string esc = db->escapeString(keyword);
    std::string query = "SELECT id, name, description, price, category, image_path, available, prep_time_minutes, rating_sum, total_ratings "
                        "FROM menu_items WHERE (name LIKE '%" + esc + "%' OR description LIKE '%" + esc + "%') AND available=1";
    std::vector<MenuItem> items;
    MYSQL_RES* res = db->executeQuery(query);
    if (!res) return items;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        items.emplace_back(atoi(row[0]), row[1], row[2]?row[2]:"", atof(row[3]),
                           row[4]?row[4]:"", row[5]?row[5]:"", atoi(row[6]), atoi(row[7]));
    }
    mysql_free_result(res);
    return items;
}

MenuItem FoodService::getMenuItemById(int itemId) {
    if (menuCache.count(itemId)) return menuCache[itemId];
    std::string query = "SELECT id, name, description, price, category, image_path, available, prep_time_minutes FROM menu_items WHERE id=" + std::to_string(itemId);
    MYSQL_RES* res = db->executeQuery(query);
    if (!res) throw OrderException("Menu item not found");
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); throw OrderException("Menu item not found"); }
    MenuItem item(atoi(row[0]), row[1], row[2]?row[2]:"", atof(row[3]), row[4]?row[4]:"", row[5]?row[5]:"", atoi(row[6]), atoi(row[7]));
    mysql_free_result(res);
    return item;
}

bool FoodService::addMenuItem(const MenuItem& item) {
    std::string query = "INSERT INTO menu_items (name, description, price, category, image_path, available, prep_time_minutes) VALUES ('"
        + db->escapeString(item.getName()) + "','" + db->escapeString(item.getDescription()) + "',"
        + std::to_string(item.getPrice()) + ",'" + db->escapeString(item.getCategory()) + "','"
        + db->escapeString(item.getImagePath()) + "'," + std::to_string(item.isAvailable()) + ","
        + std::to_string(item.getPrepTime()) + ")";
    db->executeUpdate(query);
    return true;
}

bool FoodService::updateMenuItem(const MenuItem& item) {
    std::string query = "UPDATE menu_items SET name='" + db->escapeString(item.getName())
        + "', description='" + db->escapeString(item.getDescription())
        + "', price=" + std::to_string(item.getPrice())
        + ", category='" + db->escapeString(item.getCategory())
        + "', image_path='" + db->escapeString(item.getImagePath())
        + "', available=" + std::to_string(item.isAvailable())
        + ", prep_time_minutes=" + std::to_string(item.getPrepTime())
        + " WHERE id=" + std::to_string(item.getItemId());
    db->executeUpdate(query);
    menuCache.erase(item.getItemId());
    return true;
}

bool FoodService::deleteMenuItem(int itemId) {
    db->executeUpdate("DELETE FROM menu_items WHERE id=" + std::to_string(itemId));
    menuCache.erase(itemId);
    return true;
}

bool FoodService::toggleMenuItemAvailability(int itemId) {
    db->executeUpdate("UPDATE menu_items SET available = NOT available WHERE id=" + std::to_string(itemId));
    menuCache.erase(itemId);
    return true;
}

std::vector<std::string> FoodService::getCategories() {
    std::vector<std::string> cats;
    MYSQL_RES* res = db->executeQuery("SELECT DISTINCT category FROM menu_items WHERE available=1 ORDER BY category");
    if (!res) return cats;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) cats.push_back(row[0]);
    mysql_free_result(res);
    return cats;
}

int FoodService::placeOrder(Order& order) {
    if (order.getItems().empty()) throw OrderException("Cannot place empty order");

    std::string query = "INSERT INTO orders (customer_id, status, payment_method, delivery_address, total_amount, delivery_fee, discount, notes) VALUES ("
        + std::to_string(order.getCustomerId()) + ",'Pending','"
        + db->escapeString(order.getPaymentString()) + "','"
        + db->escapeString(order.getDeliveryAddress()) + "',"
        + std::to_string(order.getTotalAmount()) + ","
        + std::to_string(order.getDeliveryFee()) + ","
        + std::to_string(order.getDiscount()) + ",'"
        + db->escapeString(order.getNotes()) + "')";
    db->executeUpdate(query);
    int orderId = db->getLastInsertId();

    for (const auto& item : order.getItems()) {
        std::string itemQuery = "INSERT INTO order_items (order_id, menu_item_id, quantity, unit_price, subtotal, special_instructions) VALUES ("
            + std::to_string(orderId) + "," + std::to_string(item.getMenuItem().getItemId())
            + "," + std::to_string(item.getQuantity()) + ","
            + std::to_string(item.getMenuItem().getPrice()) + ","
            + std::to_string(item.getSubtotal()) + ",'"
            + db->escapeString(item.getSpecialInstructions()) + "')";
        db->executeUpdate(itemQuery);
    }

    // Award loyalty points (1 point per 10 PKR spent)
    double points = order.getTotalAmount() / 10.0;
    db->executeUpdate("UPDATE customers SET loyalty_points = loyalty_points + " + std::to_string(points)
                      + " WHERE id=" + std::to_string(order.getCustomerId()));
    return orderId;
}

bool FoodService::updateOrderStatus(int orderId, OrderStatus status) {
    db->executeUpdate("UPDATE orders SET status='" + Order::statusToString(status)
                      + "' WHERE id=" + std::to_string(orderId));
    return true;
}

std::vector<Order> FoodService::getOrdersByCustomer(int customerId) {
    std::vector<Order> orders;
    MYSQL_RES* res = db->executeQuery(
        "SELECT id, customer_id, status, payment_method, delivery_address, total_amount, delivery_fee, discount, notes, order_time "
        "FROM orders WHERE customer_id=" + std::to_string(customerId) + " ORDER BY order_time DESC"
    );
    if (!res) return orders;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Order o(atoi(row[0]), atoi(row[1]), row[4]?row[4]:"", PaymentMethod::CASH_ON_DELIVERY);
        // Map status string
        std::string stat = row[2] ? row[2] : "Pending";
        if (stat == "Confirmed") o.setStatus(OrderStatus::CONFIRMED);
        else if (stat == "Preparing") o.setStatus(OrderStatus::PREPARING);
        else if (stat == "Ready for Delivery") o.setStatus(OrderStatus::READY);
        else if (stat == "Delivered") o.setStatus(OrderStatus::DELIVERED);
        else if (stat == "Cancelled") o.setStatus(OrderStatus::CANCELLED);
        orders.push_back(o);
    }
    mysql_free_result(res);
    return orders;
}

std::vector<Order> FoodService::getPendingOrders() {
    std::vector<Order> orders;
    MYSQL_RES* res = db->executeQuery(
        "SELECT id, customer_id, status, payment_method, delivery_address, total_amount FROM orders "
        "WHERE status IN ('Pending','Confirmed','Preparing') ORDER BY order_time ASC"
    );
    if (!res) return orders;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Order o(atoi(row[0]), atoi(row[1]), row[4]?row[4]:"", PaymentMethod::CASH_ON_DELIVERY);
        orders.push_back(o);
    }
    mysql_free_result(res);
    return orders;
}

std::vector<Order> FoodService::getAllOrders() {
    std::vector<Order> orders;
    MYSQL_RES* res = db->executeQuery(
        "SELECT id, customer_id, status, payment_method, delivery_address, total_amount, order_time "
        "FROM orders ORDER BY order_time DESC LIMIT 100"
    );
    if (!res) return orders;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Order o(atoi(row[0]), atoi(row[1]), row[4]?row[4]:"", PaymentMethod::CASH_ON_DELIVERY);
        orders.push_back(o);
    }
    mysql_free_result(res);
    return orders;
}

bool FoodService::cancelOrder(int orderId, int customerId) {
    MYSQL_RES* res = db->executeQuery(
        "SELECT status FROM orders WHERE id=" + std::to_string(orderId)
        + " AND customer_id=" + std::to_string(customerId)
    );
    if (!res) throw OrderException("Order not found");
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) { mysql_free_result(res); throw OrderException("Order not found or unauthorized"); }
    std::string status = row[0];
    mysql_free_result(res);
    if (status == "Preparing" || status == "Ready for Delivery" || status == "Delivered")
        throw OrderException("Cannot cancel order in status: " + status);
    db->executeUpdate("UPDATE orders SET status='Cancelled' WHERE id=" + std::to_string(orderId));
    return true;
}

double FoodService::getTotalRevenue() {
    MYSQL_RES* res = db->executeQuery("SELECT SUM(total_amount) FROM orders WHERE status='Delivered'");
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    double rev = (row && row[0]) ? atof(row[0]) : 0;
    mysql_free_result(res);
    return rev;
}

int FoodService::getTotalOrders() {
    MYSQL_RES* res = db->executeQuery("SELECT COUNT(*) FROM orders");
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int cnt = (row && row[0]) ? atoi(row[0]) : 0;
    mysql_free_result(res);
    return cnt;
}

std::vector<std::pair<std::string, int>> FoodService::getTopSellingItems(int limit) {
    std::vector<std::pair<std::string, int>> result;
    MYSQL_RES* res = db->executeQuery(
        "SELECT m.name, SUM(oi.quantity) as total_sold FROM order_items oi "
        "JOIN menu_items m ON oi.menu_item_id = m.id "
        "GROUP BY m.name ORDER BY total_sold DESC LIMIT " + std::to_string(limit)
    );
    if (!res) return result;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        result.emplace_back(row[0], row[1] ? atoi(row[1]) : 0);
    }
    mysql_free_result(res);
    return result;
}

bool FoodService::rateMenuItem(int customerId, int itemId, int rating) {
    if (rating < 1 || rating > 5) throw OrderException("Rating must be between 1 and 5");
    std::string query = "INSERT INTO ratings (customer_id, menu_item_id, rating) VALUES ("
        + std::to_string(customerId) + "," + std::to_string(itemId) + "," + std::to_string(rating)
        + ") ON DUPLICATE KEY UPDATE rating=" + std::to_string(rating);
    db->executeUpdate(query);
    db->executeUpdate(
        "UPDATE menu_items SET rating_sum = (SELECT SUM(rating) FROM ratings WHERE menu_item_id=" + std::to_string(itemId)
        + "), total_ratings = (SELECT COUNT(*) FROM ratings WHERE menu_item_id=" + std::to_string(itemId)
        + ") WHERE id=" + std::to_string(itemId)
    );
    menuCache.erase(itemId);
    return true;
}

Order FoodService::getOrderById(int orderId) {
    std::string q = "SELECT id, customer_id, delivery_address, delivery_fee, discount, notes, order_time FROM orders WHERE id=" + std::to_string(orderId);
    MYSQL_RES* res = db->executeQuery(q);
    if(!res) throw OrderException("Order not found");
    MYSQL_ROW row = mysql_fetch_row(res);
    if(!row){ mysql_free_result(res); throw OrderException("Order not found"); }
    Order o(atoi(row[0]), atoi(row[1]), row[2]?row[2]:"", PaymentMethod::CASH_ON_DELIVERY);
    if(row[3]) o.setDeliveryFee(atof(row[3]));
    if(row[4]) o.setDiscount(atof(row[4]));
    if(row[5]) o.setNotes(row[5]);
    mysql_free_result(res);
    // Load items
    MYSQL_RES* ir = db->executeQuery(
        "SELECT oi.quantity, oi.special_instructions, m.id, m.name, m.description, "
        "m.price, m.category, m.image_path, m.available, m.prep_time_minutes "
        "FROM order_items oi JOIN menu_items m ON oi.menu_item_id=m.id "
        "WHERE oi.order_id=" + std::to_string(orderId));
    if(ir){
        MYSQL_ROW ir2;
        while((ir2=mysql_fetch_row(ir))){
            MenuItem m(atoi(ir2[2]),ir2[3],ir2[4]?ir2[4]:"",atof(ir2[5]),
                       ir2[6]?ir2[6]:"",ir2[7]?ir2[7]:"",atoi(ir2[8]),atoi(ir2[9]));
            o.addItem(OrderItem(m, atoi(ir2[0]), ir2[1]?ir2[1]:""));
        }
        mysql_free_result(ir);
    }
    return o;
}
