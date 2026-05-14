#include "../include/OrderItem.h"

OrderItem::OrderItem(const MenuItem& item, int qty, const std::string& instructions)
    : menuItem(item), quantity(qty), specialInstructions(instructions) {
    subtotal = item.getPrice() * qty;
}

OrderItem::~OrderItem() {}

MenuItem OrderItem::getMenuItem() const { return menuItem; }
int OrderItem::getQuantity() const { return quantity; }
std::string OrderItem::getSpecialInstructions() const { return specialInstructions; }
double OrderItem::getSubtotal() const { return subtotal; }

void OrderItem::setQuantity(int qty) {
    quantity = qty;
    subtotal = menuItem.getPrice() * qty;
}
void OrderItem::setSpecialInstructions(const std::string& i) { specialInstructions = i; }
