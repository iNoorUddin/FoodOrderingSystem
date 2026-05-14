#include "../include/Order.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

using namespace std;

Order::Order() : orderId(0), customerId(0), status(OrderStatus::PENDING),
                 paymentMethod(PaymentMethod::CASH_ON_DELIVERY),
                 totalAmount(0), deliveryFee(50), discount(0) {
    time_t now = time(0);
    orderTime = string(ctime(&now));
    if (!orderTime.empty() && orderTime.back() == '\n') orderTime.pop_back();
}



Order::Order(int orderId, int customerId, const std::string& address, PaymentMethod pm)
    : orderId(orderId), customerId(customerId), deliveryAddress(address),
      status(OrderStatus::PENDING), paymentMethod(pm),
      totalAmount(0), deliveryFee(50), discount(0) {
    time_t now = time(0);
    orderTime = string(ctime(&now));
    if (!orderTime.empty() && orderTime.back() == '\n') orderTime.pop_back();
}

Order::~Order() {}

void Order::calculateTotal() {
    totalAmount = 0;
    for (const auto& item : items) {
        totalAmount += item.getSubtotal();
    }
    totalAmount += deliveryFee;
    totalAmount -= discount;
}

void Order::addItem(const OrderItem& item) {
    // STL algorithm usage
    auto it = find_if(items.begin(), items.end(), [&](const OrderItem& oi){
        return oi.getMenuItem().getItemId() == item.getMenuItem().getItemId();
    });
    if (it != items.end()) {
        it->setQuantity(it->getQuantity() + item.getQuantity());
    } else {
        items.push_back(item);
    }
    calculateTotal();
}

void Order::removeItem(int itemId) {
    items.erase(remove_if(items.begin(), items.end(), [&](const OrderItem& oi){
        return oi.getMenuItem().getItemId() == itemId;
    }), items.end());
    calculateTotal();
}

void Order::clearItems() { items.clear(); totalAmount = 0; }

int Order::getOrderId() const { return orderId; }
int Order::getCustomerId() const { return customerId; }
OrderStatus Order::getStatus() const { return status; }
PaymentMethod Order::getPaymentMethod() const { return paymentMethod; }
string Order::getDeliveryAddress() const { return deliveryAddress; }
double Order::getTotalAmount() const { return totalAmount; }
double Order::getDeliveryFee() const { return deliveryFee; }
double Order::getDiscount() const { return discount; }
string Order::getOrderTime() const { return orderTime; }
string Order::getEstimatedDeliveryTime() const { return estimatedDeliveryTime; }
string Order::getNotes() const { return notes; }
vector<OrderItem> Order::getItems() const { return items; }

void Order::setStatus(OrderStatus s) { status = s; }
void Order::setDiscount(double d) { discount = d; calculateTotal(); }
void Order::setDeliveryFee(double f) { deliveryFee = f; calculateTotal(); }
void Order::setNotes(const std::string& n) { notes = n; }
void Order::setEstimatedDeliveryTime(const std::string& t) { estimatedDeliveryTime = t; }

std::string Order::statusToString(OrderStatus s) {
    switch(s) {
        case OrderStatus::PENDING: return "Pending";
        case OrderStatus::CONFIRMED: return "Confirmed";
        case OrderStatus::PREPARING: return "Preparing";
        case OrderStatus::READY: return "Ready for Delivery";
        case OrderStatus::DELIVERED: return "Delivered";
        case OrderStatus::CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

string Order::getStatusString() const { return statusToString(status); }

string Order::getPaymentString() const {
    switch(paymentMethod) {
        case PaymentMethod::CASH_ON_DELIVERY: return "Cash on Delivery";
        case PaymentMethod::CREDIT_CARD: return "Credit Card";
        case PaymentMethod::DEBIT_CARD: return "Debit Card";
        case PaymentMethod::MOBILE_WALLET: return "Mobile Wallet";
        default: return "Unknown";
    }
}

void Order::printReceipt() const {
    cout << std::fixed << std::setprecision(2);
    cout << "\n========== RECEIPT ==========\n";
    cout << "Order #" << orderId << "\n";
    cout << "Date: " << orderTime << "\n";
    cout << "-----------------------------\n";
    for (const auto& item : items) {
        cout << item.getMenuItem().getName() << " x" << item.getQuantity()
                  << "  PKR " << item.getSubtotal() << "\n";
        if (!item.getSpecialInstructions().empty())
            cout << "  Note: " << item.getSpecialInstructions() << "\n";
    }
    cout << "-----------------------------\n";
    cout << "Delivery Fee: PKR " << deliveryFee << "\n";
    if (discount > 0) cout << "Discount: PKR " << discount << "\n";
    cout << "TOTAL: PKR " << totalAmount << "\n";
    cout << "Payment: " << getPaymentString() << "\n";
    cout << "Status: " << getStatusString() << "\n";
    cout << "=============================\n";
}
