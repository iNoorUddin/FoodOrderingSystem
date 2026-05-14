#include "../include/MenuItem.h"
#include <iostream>
#include <iomanip>

MenuItem::MenuItem() : itemId(0), price(0), available(true), prepTimeMinutes(15), rating(0), totalRatings(0) {}

MenuItem::MenuItem(int itemId, const std::string& name, const std::string& description,
                   double price, const std::string& category, const std::string& imagePath,
                   bool available, int prepTime)
    : itemId(itemId), name(name), description(description), price(price),
      category(category), imagePath(imagePath), available(available),
      prepTimeMinutes(prepTime), rating(0), totalRatings(0) {}

MenuItem::~MenuItem() {}

int MenuItem::getItemId() const { return itemId; }
std::string MenuItem::getName() const { return name; }
std::string MenuItem::getDescription() const { return description; }
double MenuItem::getPrice() const { return price; }
std::string MenuItem::getCategory() const { return category; }
std::string MenuItem::getImagePath() const { return imagePath; }
bool MenuItem::isAvailable() const { return available; }
int MenuItem::getPrepTime() const { return prepTimeMinutes; }
double MenuItem::getRating() const { return totalRatings > 0 ? rating / totalRatings : 0; }
int MenuItem::getTotalRatings() const { return totalRatings; }

void MenuItem::setName(const std::string& n) { name = n; }
void MenuItem::setDescription(const std::string& d) { description = d; }
void MenuItem::setPrice(double p) { price = p; }
void MenuItem::setCategory(const std::string& c) { category = c; }
void MenuItem::setImagePath(const std::string& path) { imagePath = path; }
void MenuItem::setAvailable(bool a) { available = a; }
void MenuItem::setPrepTime(int m) { prepTimeMinutes = m; }
void MenuItem::addRating(int r) { rating += r; totalRatings++; }

void MenuItem::display() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[" << itemId << "] " << name << " - PKR " << price << "\n"
              << "    Category: " << category << "\n"
              << "    " << description << "\n"
              << "    Prep Time: " << prepTimeMinutes << " mins | "
              << "Rating: " << getRating() << "/5\n"
              << "    Status: " << (available ? "Available" : "Unavailable") << "\n";
}

std::ostream& operator<<(std::ostream& os, const MenuItem& item) {
    os << item.name << " (PKR " << item.price << ")";
    return os;
}
