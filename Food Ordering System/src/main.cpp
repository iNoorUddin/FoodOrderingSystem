/**
 * ============================================================
 * FOOD EXPRESS - Online Food Ordering System
 * ============================================================
 * Project: CCA - Complex Computing Activity
 * System:  Online Food Ordering System
 * Tech:    C++ | Qt6 GUI | MySQL Database
 * OOP:     Encapsulation, Inheritance, Polymorphism,
 *          Composition, Abstract Classes, Friend Functions
 * ============================================================
 */

#include <QApplication>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QFont>
#include <QFontDatabase>
#include <iostream>

#include "../include/MainWindow.h"
#include "../include/DatabaseManager.h"

// ============================================================
// DB Configuration Dialog
// ============================================================
bool showDatabaseConfigDialog(QString& host, QString& user, QString& password, QString& dbName) {
    QDialog dialog;
    dialog.setWindowTitle("Database Configuration");
    dialog.setFixedSize(420, 350);
    dialog.setStyleSheet(R"(
        QDialog { background: #151515; color: #F0F0F0; }
        QLabel { color: #F0F0F0; font-size: 13px; }
        QLineEdit {
            background: #1E1E1E; border: 2px solid #333; border-radius: 8px;
            padding: 10px; color: #F0F0F0; font-size: 13px;
        }
        QLineEdit:focus { border-color: #FF6B35; }
        QPushButton {
            background: #FF6B35; color: white; border: none; border-radius: 8px;
            padding: 12px 24px; font-size: 14px; font-weight: bold;
        }
        QPushButton:hover { background: #FF8C5A; }
    )");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(14);

    QLabel* title = new QLabel("🍽️ Foodin - Database Setup");
    title->setFont(QFont("Segoe UI", 16, QFont::Bold));
    title->setStyleSheet("color: #FF6B35;");
    title->setAlignment(Qt::AlignCenter);

    QFormLayout* form = new QFormLayout();
    form->setSpacing(10);

    QLineEdit* hostEdit = new QLineEdit("localhost");
    QLineEdit* userEdit = new QLineEdit("root");
    QLineEdit* passEdit = new QLineEdit();
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText("Enter MySQL password");
    QLineEdit* dbEdit = new QLineEdit("food_ordering_db");

    form->addRow("Host:", hostEdit);
    form->addRow("Username:", userEdit);
    form->addRow("Password:", passEdit);
    form->addRow("Database:", dbEdit);

    QLabel* hint = new QLabel("Default admin: admin@foodapp.com / admin123");
    hint->setStyleSheet("color: #888; font-size: 11px;");
    hint->setAlignment(Qt::AlignCenter);

    QPushButton* connectBtn = new QPushButton("Connect & Launch");
    QPushButton* cancelBtn = new QPushButton("Exit");
    cancelBtn->setStyleSheet("background: #2A2A2A; color: #888;");

    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(connectBtn);

    layout->addWidget(title);
    layout->addLayout(form);
    layout->addWidget(hint);
    layout->addLayout(btnRow);

    bool accepted = false;
    QObject::connect(connectBtn, &QPushButton::clicked, [&](){
        host = hostEdit->text();
        user = userEdit->text();
        password = passEdit->text();
        dbName = dbEdit->text();
        accepted = true;
        dialog.accept();
    });
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
    return accepted;
}

// ============================================================
// MAIN ENTRY POINT
// ============================================================
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Foodin");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Foodin Team");

    // Set default font
    QFont defaultFont("Segoe UI", 10);
    app.setFont(defaultFont);

    // Database configuration
    QString host, user, password, dbName;
    if (!showDatabaseConfigDialog(host, user, password, dbName)) {
        return 0;
    }

    // Connect to database
    try {
        DatabaseManager* db = DatabaseManager::getInstance();
        db->connect(host.toStdString(), user.toStdString(),
                    password.toStdString(), dbName.toStdString());
    } catch (const DatabaseException& e) {
        QMessageBox::critical(nullptr, "Database Error",
            QString("Failed to connect to database:\n%1\n\nPlease check your MySQL settings.")
            .arg(e.what()));
        return 1;
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Error", QString(e.what()));
        return 1;
    }

    // Launch main window
    MainWindow window;
    window.show();

    return app.exec();
}
