#ifndef ORDER_H
#define ORDER_H

#include "OrderItem.h"
#include <vector>
#include <string>
#include <ctime>

enum class OrderStatus {
    PENDING,
    CONFIRMED,
    PREPARING,
    READY,
    DELIVERED,
    CANCELLED
};

enum class PaymentMethod {
    CASH_ON_DELIVERY,
    CREDIT_CARD,
    DEBIT_CARD,
    MOBILE_WALLET
};

// Order class - demonstrates Composition (Order has many OrderItems)
class Order {
private:
    int orderId;
    int customerId;
    std::vector<OrderItem> items;  // Composition
    OrderStatus status;
    PaymentMethod paymentMethod;
    std::string deliveryAddress;
    double totalAmount;
    double deliveryFee;
    double discount;
    std::string orderTime;
    std::string estimatedDeliveryTime;
    std::string notes;

    // Helper
    void calculateTotal();

public:
    Order();
    Order(int orderId, int customerId, const std::string& address, PaymentMethod pm);
    ~Order();

    void addItem(const OrderItem& item);
    void removeItem(int itemId);
    void clearItems();

    int getOrderId() const;
    int getCustomerId() const;
    OrderStatus getStatus() const;
    PaymentMethod getPaymentMethod() const;
    std::string getDeliveryAddress() const;
    double getTotalAmount() const;
    double getDeliveryFee() const;
    double getDiscount() const;
    std::string getOrderTime() const;
    std::string getEstimatedDeliveryTime() const;
    std::string getNotes() const;
    std::vector<OrderItem> getItems() const;

    void setStatus(OrderStatus status);
    void setDiscount(double discount);
    void setDeliveryFee(double fee);
    void setNotes(const std::string& notes);
    void setEstimatedDeliveryTime(const std::string& time);

    std::string getStatusString() const;
    std::string getPaymentString() const;
    void printReceipt() const;

    static std::string statusToString(OrderStatus s);
};

#endif
