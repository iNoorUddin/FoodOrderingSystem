#ifndef ORDERITEM_H
#define ORDERITEM_H

#include "MenuItem.h"

// OrderItem - Composition: Order has OrderItems
class OrderItem {
private:
    MenuItem menuItem;
    int quantity;
    std::string specialInstructions;
    double subtotal;

public:
    OrderItem(const MenuItem& item, int qty, const std::string& instructions = "");
    ~OrderItem();

    MenuItem getMenuItem() const;
    int getQuantity() const;
    std::string getSpecialInstructions() const;
    double getSubtotal() const;

    void setQuantity(int qty);
    void setSpecialInstructions(const std::string& instructions);
};

#endif
