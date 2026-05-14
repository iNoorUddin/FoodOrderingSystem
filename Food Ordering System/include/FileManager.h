#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "Order.h"
#include "MenuItem.h"


class FileManager {
public:
    static bool saveReceipt(const Order& order, const std::string& customerName);
    static bool exportMenuToCSV(const std::vector<MenuItem>& items, const std::string& filename);
    static bool exportOrdersToCSV(const std::vector<Order>& orders, const std::string& filename);
    static std::string loadConfig(const std::string& key);
    static bool saveConfig(const std::string& key, const std::string& value);
    static bool logActivity(const std::string& activity, const std::string& user);
    static std::vector<std::string> readLog(int lastNLines = 50);
};

#endif
