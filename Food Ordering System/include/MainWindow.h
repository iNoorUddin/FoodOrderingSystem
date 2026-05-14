#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QTimer>
#include <QPixmap>
#include <QFrame>
#include <QProgressBar>
#include <QGraphicsDropShadowEffect>
#include <QFileInfo>
#include <vector>
#include "../include/FoodService.h"
#include "../include/FileManager.h"
#include "../include/Customer.h"
#include "../include/Admin.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    FoodService* foodService;
    Customer* currentCustomer;
    Admin* currentAdmin;
    std::vector<OrderItem> cartItems;
    int editingItemId;

    // Stacks
    QStackedWidget* mainStack;
    QStackedWidget* customerStack;
    QStackedWidget* adminStack;

    // Pages
    QWidget* loginPage;
    QWidget* registerPage;
    QWidget* customerDashboard;
    QWidget* adminDashboard;
    QWidget* homePage;
    QWidget* menuPage;
    QWidget* cartPage;
    QWidget* ordersPage;
    QWidget* profilePage;
    QWidget* adminMenuPage;
    QWidget* adminOrdersPage;
    QWidget* adminAnalyticsPage;

    // Shared widgets
    QWidget* menuItemsContainer;
    QScrollArea* menuScrollArea;
    QComboBox* categoryFilter;
    QLineEdit* searchBar;
    QWidget* cartContainer;
    QLabel* cartTotalLabel;
    QLabel* cartBadge;
    QTableWidget* ordersTable;
    QTableWidget* adminOrdersTable;
    QTableWidget* adminMenuTable;
    QLabel* totalRevenueLabel;
    QLabel* totalOrdersLabel;
    QWidget* topItemsContainer;
    QWidget* popularContainer;
    QLabel* welcomeLabel;
    QLabel* loyaltyPointsLabel;
    QComboBox* paymentCombo;
    QTextEdit* notesEdit;
    QLabel* notificationLabel;
    QTimer* notificationTimer;

    // Admin form
    QLineEdit* itemNameEdit;
    QLineEdit* itemDescEdit;
    QLineEdit* itemPriceEdit;
    QLineEdit* itemCategoryEdit;
    QLineEdit* itemPrepEdit;
    QLineEdit* itemImageEdit;

    // Build methods
    void buildLoginPage();
    void buildRegisterPage();
    void buildCustomerDashboard();
    void buildAdminDashboard();
    void buildHomePage();
    void buildMenuPage();
    void buildCartPage();
    void buildOrdersPage();
    void buildProfilePage();
    void buildAdminMenuPage();
    void buildAdminOrdersPage();
    void buildAdminAnalyticsPage();
    void refreshAdminMenuTable();
    void refreshHomePage();

    // Sidebar helper
    QWidget* buildSidebar(const QString& brand, const QStringList& items,
                          QStackedWidget* stack, bool isAdmin, QVBoxLayout*& slOut);

    // UI helpers
    QWidget* createMenuItemCard(const MenuItem& item, bool compact = false);
    QFrame* createSeparator();
    QLabel* createTitleLabel(const QString& text);
    QPushButton* createStyledButton(const QString& text, const QString& color = "#f97316");
    QPushButton* createSolidButton(const QString& text, const QString& color = "#f97316");
    void addShadow(QWidget* w, QColor c = QColor(249,115,22,40));
    void showNotif(const QString& msg, bool ok = true);

    // Refresh
    void refreshMenu();
    void refreshCart();
    void refreshOrders();
    void refreshAdminOrders();
    void refreshAnalytics();
    void updateCartBadge();

private slots:
    void onLogin();
    void onRegister();
    void onLogout();
    void onSearch();
    void onCategoryFilter(const QString&);
    void onAddToCart(int itemId);
    void onRemoveFromCart(int itemId);
    void onPlaceOrder();
    void onCancelOrder(int orderId);
    void onAdminUpdateStatus(int orderId, const QString& status);
    void onAddMenuItem();
    void onUpdateMenuItem();
    void onDeleteMenuItem(int itemId);
    void onToggleAvail(int itemId);
    void onBrowseImage();
    void onExportMenu();
    void onExportOrders();
    void hideNotif();
};

#endif
