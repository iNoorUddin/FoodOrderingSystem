#include "../include/FileManager.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <map>
#include <string>

bool FileManager::saveReceipt(const Order& order, const std::string& customerName) {
    try {
        std::string filename = "receipts/receipt_order_" + std::to_string(order.getOrderId()) + ".txt";
        // Create receipts directory
        system("mkdir -p receipts");
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        file << "============================================\n";
        file << "         FOODIN - ONLINE FOOD ORDERING      \n";
        file << "============================================\n";
        file << "Order ID   : #" << order.getOrderId() << "\n";
        file << "Customer   : " << customerName << "\n";
        file << "Date       : " << order.getOrderTime() << "\n";
        file << "--------------------------------------------\n";
        file << std::left << std::setw(25) << "Item" << std::setw(5) << "Qty"
             << std::right << std::setw(10) << "Price\n";
        file << "--------------------------------------------\n";
        for (const auto& item : order.getItems()) {
            file << std::left << std::setw(25) << item.getMenuItem().getName()
                 << std::setw(5) << item.getQuantity()
                 << std::right << std::setw(10) << std::fixed << std::setprecision(2)
                 << item.getSubtotal() << " PKR\n";
            if (!item.getSpecialInstructions().empty())
                file << "  [Note: " << item.getSpecialInstructions() << "]\n";
        }
        file << "--------------------------------------------\n";
        file << std::right << std::setw(30) << "Delivery Fee: "
             << std::setw(10) << order.getDeliveryFee() << " PKR\n";
        if (order.getDiscount() > 0)
            file << std::setw(30) << "Discount: " << std::setw(10) << order.getDiscount() << " PKR\n";
        file << std::setw(30) << "TOTAL: " << std::setw(10) << order.getTotalAmount() << " PKR\n";
        file << "============================================\n";
        file << "Payment: " << order.getPaymentString() << "\n";
        file << "Status : " << order.getStatusString() << "\n";
        file << "============================================\n";
        file << "  Thank you for ordering with Foodin! Enjoy your meal! \n";
        file << "============================================\n";
        file.close();
        logActivity("Receipt saved for Order #" + std::to_string(order.getOrderId()), customerName);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving receipt: " << e.what() << "\n";
        return false;
    }
}

bool FileManager::exportMenuToCSV(const std::vector<MenuItem>& items, const std::string& filename) {
    try {
        system("mkdir -p exports");
        std::ofstream file("exports/" + filename);
        if (!file.is_open()) return false;
        file << "ID,Name,Description,Price,Category,Available,PrepTime\n";
        for (const auto& item : items) {
            file << item.getItemId() << ","
                 << "\"" << item.getName() << "\","
                 << "\"" << item.getDescription() << "\","
                 << item.getPrice() << ","
                 << item.getCategory() << ","
                 << (item.isAvailable() ? "Yes" : "No") << ","
                 << item.getPrepTime() << "\n";
        }
        file.close();
        return true;
    } catch (...) { return false; }
}

bool FileManager::exportOrdersToCSV(const std::vector<Order>& orders, const std::string& filename) {
    try {
        system("mkdir -p exports");
        std::ofstream file("exports/" + filename);
        if (!file.is_open()) return false;
        file << "OrderID,CustomerID,Status,Total,PaymentMethod,OrderTime\n";
        for (const auto& order : orders) {
            file << order.getOrderId() << "," << order.getCustomerId() << ","
                 << order.getStatusString() << "," << order.getTotalAmount() << ","
                 << order.getPaymentString() << "," << order.getOrderTime() << "\n";
        }
        file.close();
        return true;
    } catch (...) { return false; }
}

std::string FileManager::loadConfig(const std::string& key) {
    try {
        std::ifstream file("config.txt");
        if (!file.is_open()) return "";
        std::string line;
        while (std::getline(file, line)) {
            auto pos = line.find('=');
            if (pos != std::string::npos && line.substr(0, pos) == key) {
                return line.substr(pos + 1);
            }
        }
        return "";
    } catch (...) { return ""; }
}

bool FileManager::saveConfig(const std::string& key, const std::string& value) {
    try {
        // Read existing config
        std::map<std::string, std::string> config;
        std::ifstream inFile("config.txt");
        if (inFile.is_open()) {
            std::string line;
            while (std::getline(inFile, line)) {
                auto pos = line.find('=');
                if (pos != std::string::npos)
                    config[line.substr(0, pos)] = line.substr(pos + 1);
            }
            inFile.close();
        }
        config[key] = value;
        std::ofstream outFile("config.txt");
        for (const auto& [k, v] : config) outFile << k << "=" << v << "\n";
        return true;
    } catch (...) { return false; }
}

bool FileManager::logActivity(const std::string& activity, const std::string& user) {
    try {
        system("mkdir -p logs");
        std::ofstream file("logs/activity.log", std::ios::app);
        if (!file.is_open()) return false;
        time_t now = time(0);
        std::string timeStr = std::string(ctime(&now));
        if (!timeStr.empty() && timeStr.back() == '\n') timeStr.pop_back();
        file << "[" << timeStr << "] [" << user << "] " << activity << "\n";
        file.close();
        return true;
    } catch (...) { return false; }
}

std::vector<std::string> FileManager::readLog(int lastNLines) {
    std::vector<std::string> lines;
    try {
        std::ifstream file("logs/activity.log");
        if (!file.is_open()) return lines;
        std::string line;
        while (std::getline(file, line)) lines.push_back(line);
        if ((int)lines.size() > lastNLines)
            lines.erase(lines.begin(), lines.begin() + lines.size() - lastNLines);
    } catch (...) {}
    return lines;
}
