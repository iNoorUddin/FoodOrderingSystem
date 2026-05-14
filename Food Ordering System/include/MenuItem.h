#ifndef MENUITEM_H
#define MENUITEM_H

#include <string>
#include <iostream>

// MenuItem class - Encapsulation of food item data
class MenuItem {
private:
    int itemId;
    std::string name;
    std::string description;
    double price;
    std::string category;
    std::string imagePath;
    bool available;
    int prepTimeMinutes;
    double rating;
    int totalRatings;

public:
    MenuItem();
    MenuItem(int itemId, const std::string& name, const std::string& description,
             double price, const std::string& category, const std::string& imagePath,
             bool available = true, int prepTime = 15);
    ~MenuItem();

    // Getters
    int getItemId() const;
    std::string getName() const;
    std::string getDescription() const;
    double getPrice() const;
    std::string getCategory() const;
    std::string getImagePath() const;
    bool isAvailable() const;
    int getPrepTime() const;
    double getRating() const;
    int getTotalRatings() const;

    // Setters
    void setName(const std::string& name);
    void setDescription(const std::string& description);
    void setPrice(double price);
    void setCategory(const std::string& category);
    void setImagePath(const std::string& path);
    void setAvailable(bool available);
    void setPrepTime(int minutes);
    void addRating(int rating);

    void display() const;
    friend std::ostream& operator<<(std::ostream& os, const MenuItem& item);
};

#endif
