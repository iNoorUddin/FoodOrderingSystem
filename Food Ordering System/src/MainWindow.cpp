#include "../include/MainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QScrollBar>
#include <QDateTime>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <algorithm>

// ─── THEME ───────────────────────────────────────────────────────
#define BG_MAIN   "#111111"
#define BG_CARD   "#161616"
#define BG_PANEL  "#0e0e0e"
#define BG_HOVER  "#1c1c1c"
#define ORANGE    "#f97316"
#define ORANGE_D  "#ea650c"
#define TEXT_MAIN "#f0f0f0"
#define TEXT_DIM  "#666666"
#define BORDER    "#1e1e1e"
#define BORDER2   "#262626"

// ─── CONSTRUCTOR ─────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , foodService(new FoodService())
    , currentCustomer(nullptr), currentAdmin(nullptr)
    , editingItemId(-1)
{
    setWindowTitle("Foodin – Online Food Ordering");
    resize(1320, 840);

    setStyleSheet(QString(R"(
        QMainWindow,QWidget{background:%1;color:%2;font-family:'Segoe UI',Arial,sans-serif;}
        QScrollBar:vertical{background:#1a1a1a;width:5px;border-radius:3px;}
        QScrollBar::handle:vertical{background:#f97316;border-radius:3px;min-height:20px;}
        QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}
        QScrollBar:horizontal{background:#1a1a1a;height:5px;border-radius:3px;}
        QScrollBar::handle:horizontal{background:#f97316;border-radius:3px;}
        QLineEdit{
            background:#1a1a1a;border:1px solid #262626;border-radius:9px;
            padding:10px 14px;color:%2;font-size:13px;
        }
        QLineEdit:hover{border-color:#333;}
        QLineEdit:focus{border-color:%3;background:#1c1c1c;
            box-shadow:0 0 0 3px rgba(249,115,22,0.12);}
        QComboBox{background:#1a1a1a;border:1px solid #262626;border-radius:9px;
            padding:8px 13px;color:%2;font-size:13px;}
        QComboBox:focus{border-color:%3;}
        QComboBox::drop-down{border:none;}
        QComboBox QAbstractItemView{background:#1a1a1a;color:%2;border:1px solid #2a2a2a;
            selection-background-color:rgba(249,115,22,0.15);border-radius:8px;}
        QTableWidget{background:#161616;border:1px solid #222;border-radius:10px;
            gridline-color:rgba(255,255,255,0.04);color:%2;}
        QTableWidget::item{padding:8px;}
        QTableWidget::item:selected{background:rgba(249,115,22,0.12);color:%3;}
        QTableWidget::item:hover{background:#1c1c1c;}
        QHeaderView::section{background:#141414;color:%3;border:none;
            border-bottom:1px solid rgba(249,115,22,0.2);padding:10px;
            font-weight:700;font-size:11px;letter-spacing:1px;text-transform:uppercase;}
        QTextEdit{background:#1a1a1a;border:1px solid #262626;border-radius:9px;
            padding:8px;color:%2;}
        QTextEdit:focus{border-color:%3;}
        QProgressBar{background:rgba(255,255,255,0.05);border-radius:4px;border:none;}
        QProgressBar::chunk{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
            stop:0 %3,stop:1 %4);border-radius:4px;}
        QToolTip{background:#1a1a1a;color:%2;border:1px solid #2a2a2a;
            border-radius:6px;padding:5px;}
    )").arg(BG_MAIN).arg(TEXT_MAIN).arg(ORANGE).arg(ORANGE_D));

    mainStack = new QStackedWidget(this);
    setCentralWidget(mainStack);

    buildLoginPage();        // 0
    buildRegisterPage();     // 1
    buildCustomerDashboard();// 2
    buildAdminDashboard();   // 3

    notificationLabel = new QLabel(this);
    notificationLabel->setFixedHeight(42);
    notificationLabel->setAlignment(Qt::AlignCenter);
    notificationLabel->setFont(QFont("Segoe UI",11,QFont::Bold));
    notificationLabel->hide();
    notificationTimer = new QTimer(this);
    notificationTimer->setSingleShot(true);
    connect(notificationTimer,&QTimer::timeout,this,&MainWindow::hideNotif);
}

MainWindow::~MainWindow(){
    delete foodService; delete currentCustomer; delete currentAdmin;
}

// ─── HELPERS ─────────────────────────────────────────────────────
QPushButton* MainWindow::createStyledButton(const QString& text,const QString& color){
    QString c=color.isEmpty()?ORANGE:color;
    auto* b=new QPushButton(text);
    b->setStyleSheet(QString(R"(
        QPushButton{background:transparent;color:%1;border:1px solid %1;
            border-radius:9px;padding:10px 20px;font-size:13px;font-weight:600;}
        QPushButton:hover{background:%1;color:#fff;
            box-shadow:0 5px 16px rgba(249,115,22,0.3);}
        QPushButton:pressed{background:%2;}
        QPushButton:disabled{color:#333;border-color:#2a2a2a;}
    )").arg(c).arg(ORANGE_D));
    b->setCursor(Qt::PointingHandCursor); return b;
}

QPushButton* MainWindow::createSolidButton(const QString& text,const QString& color){
    QString c=color.isEmpty()?ORANGE:color;
    auto* b=new QPushButton(text);
    b->setStyleSheet(QString(R"(
        QPushButton{background:%1;color:#fff;border:none;
            border-radius:9px;padding:11px 22px;font-size:13px;font-weight:700;}
        QPushButton:hover{background:%2;transform:translateY(-1px);
            box-shadow:0 6px 20px rgba(249,115,22,0.35);}
        QPushButton:pressed{background:%2;}
        QPushButton:disabled{background:#2a2a2a;color:#555;}
    )").arg(c).arg(ORANGE_D));
    b->setCursor(Qt::PointingHandCursor); return b;
}

QLabel* MainWindow::createTitleLabel(const QString& text){
    auto* l=new QLabel(text);
    l->setFont(QFont("Segoe UI",20,QFont::Bold));
    l->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));
    return l;
}

void MainWindow::addShadow(QWidget* w,QColor c){
    auto* s=new QGraphicsDropShadowEffect;
    s->setBlurRadius(20); s->setColor(c); s->setOffset(0,2);
    w->setGraphicsEffect(s);
}

QFrame* MainWindow::createSeparator(){
    auto* f=new QFrame; f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color:#1e1e1e;"); return f;
}

void MainWindow::showNotif(const QString& msg,bool ok){
    notificationLabel->setText(msg);
    notificationLabel->setStyleSheet(ok
        ? "background:#1a1a1a;border:1px solid rgba(34,197,94,0.4);color:#22c55e;"
          "border-radius:9px;font-weight:600;padding:0 16px;"
          "box-shadow:0 4px 16px rgba(34,197,94,0.12);"
        : "background:#1a1a1a;border:1px solid rgba(239,68,68,0.4);color:#ef4444;"
          "border-radius:9px;font-weight:600;padding:0 16px;");
    int w=qMin(560,width()-40);
    notificationLabel->setGeometry((width()-w)/2,8,w,42);
    notificationLabel->show(); notificationLabel->raise();
    notificationTimer->start(3200);
}
void MainWindow::hideNotif(){ notificationLabel->hide(); }
void MainWindow::updateCartBadge(){
    if(!cartBadge) return;
    int n=(int)cartItems.size();
    cartBadge->setText(QString::number(n));
    cartBadge->setVisible(n>0);
}

// ─── LOGIN PAGE ──────────────────────────────────────────────────
void MainWindow::buildLoginPage(){
    loginPage=new QWidget;
    loginPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* ml=new QHBoxLayout(loginPage);
    ml->setContentsMargins(0,0,0,0); ml->setSpacing(0);

    // Left image panel
    auto* lp=new QWidget; lp->setFixedWidth(520);
    lp->setStyleSheet(R"(background:#0a0a0a;border-right:1px solid #1e1e1e;)");
    auto* ll=new QVBoxLayout(lp);
    ll->setContentsMargins(0,0,0,0); ll->setSpacing(0);

    // Big food image
    auto* imgLabel=new QLabel; imgLabel->setFixedSize(520,400);
    imgLabel->setAlignment(Qt::AlignCenter); imgLabel->setScaledContents(true);
    QPixmap px(":/images/banner.jpg");
    if(px.isNull()) px.load("resources/images/banner.jpg");
    if(!px.isNull()){
        imgLabel->setPixmap(px.scaled(520,400,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
    } else {
        imgLabel->setText("🍽️");
        imgLabel->setFont(QFont("Segoe UI Emoji",80));
        imgLabel->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1a0800,stop:1 #0a0a0a);");
    }

    // Brand overlay on image
    auto* brandOverlay=new QWidget; brandOverlay->setFixedHeight(120);
    brandOverlay->setStyleSheet(R"(
        background:qlineargradient(x1:0,y1:0,x2:0,y2:1,
            stop:0 rgba(10,10,10,0.9),stop:1 rgba(10,10,10,0));
    )");
    auto* bol=new QVBoxLayout(brandOverlay);
    bol->setContentsMargins(32,20,32,10);
    auto* brandRow=new QHBoxLayout;
    auto* brandIcon=new QLabel("🍽️");
    brandIcon->setFont(QFont("Segoe UI Emoji",28));
    brandIcon->setStyleSheet("background:transparent;");
    auto* brandName=new QLabel("Foodin");
    brandName->setFont(QFont("Segoe UI",26,QFont::Bold));
    brandName->setStyleSheet(QString("color:%1;background:transparent;").arg(ORANGE));
    brandRow->addWidget(brandIcon); brandRow->addWidget(brandName); brandRow->addStretch();
    bol->addLayout(brandRow);

    // Stack image + overlay using absolute positioning trick
    auto* imgContainer=new QWidget; imgContainer->setFixedHeight(400);
    auto* imgLayout=new QVBoxLayout(imgContainer);
    imgLayout->setContentsMargins(0,0,0,0); imgLayout->setSpacing(0);
    imgLayout->addWidget(imgLabel);
    brandOverlay->setParent(imgContainer);
    brandOverlay->setGeometry(0,0,520,120);
    brandOverlay->raise();

    // Bottom info
    auto* infoWidget=new QWidget;
    infoWidget->setStyleSheet("background:#0e0e0e;border-top:1px solid #1a1a1a;");
    auto* infoLayout=new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(28,18,28,18);
    QStringList stats={"50+\nMenu Items","30 Min\nDelivery","4.8★\nRating","100%\nFresh"};
    for(const auto& s:stats){
        auto* w=new QWidget;
        auto* wl=new QVBoxLayout(w); wl->setContentsMargins(0,0,0,0); wl->setSpacing(2);
        QStringList parts=s.split("\n");
        auto* num=new QLabel(parts[0]);
        num->setFont(QFont("Segoe UI",16,QFont::Bold));
        num->setStyleSheet(QString("color:%1;background:transparent;").arg(ORANGE));
        num->setAlignment(Qt::AlignCenter);
        auto* lbl=new QLabel(parts[1]);
        lbl->setStyleSheet(QString("color:%1;font-size:10px;background:transparent;").arg(TEXT_DIM));
        lbl->setAlignment(Qt::AlignCenter);
        wl->addWidget(num); wl->addWidget(lbl);
        infoLayout->addWidget(w,1);
        if(stats.indexOf(s)<3){
            auto* sep=new QFrame; sep->setFrameShape(QFrame::VLine);
            sep->setStyleSheet("color:#222;"); infoLayout->addWidget(sep);
        }
    }

    ll->addWidget(imgContainer);
    ll->addWidget(infoWidget,1);

    // Right form
    auto* rp=new QWidget;
    rp->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* rl=new QVBoxLayout(rp);
    rl->setAlignment(Qt::AlignCenter);
    rl->setContentsMargins(70,50,70,50); rl->setSpacing(15);

    auto* hello=new QLabel("WELCOME BACK");
    hello->setStyleSheet(QString("color:%1;font-size:10px;letter-spacing:4px;font-weight:700;").arg(ORANGE));

    auto* loginTitle=new QLabel("Sign In");
    loginTitle->setFont(QFont("Segoe UI",28,QFont::Bold));
    loginTitle->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));

    auto* subTxt=new QLabel("Enter your credentials to continue");
    subTxt->setStyleSheet(QString("color:%1;font-size:12px;").arg(TEXT_DIM));

    // Role toggle
    auto* rRow=new QHBoxLayout;
    auto* custBtn=new QPushButton("👤  Customer");
    auto* admBtn=new QPushButton("⚙️  Admin");
    custBtn->setObjectName("custRoleBtn"); admBtn->setObjectName("admRoleBtn");
    custBtn->setCheckable(true); admBtn->setCheckable(true); custBtn->setChecked(true);
    QString rs=QString(R"(
        QPushButton{background:#1a1a1a;color:%1;border:1px solid #262626;
            border-radius:9px;padding:10px 16px;font-size:13px;}
        QPushButton:checked{background:%2;color:#fff;border-color:%2;font-weight:700;}
        QPushButton:hover:!checked{background:#1e1e1e;border-color:#333;color:%3;}
    )").arg(TEXT_DIM).arg(ORANGE).arg(TEXT_MAIN);
    custBtn->setStyleSheet(rs); admBtn->setStyleSheet(rs);
    custBtn->setCursor(Qt::PointingHandCursor); admBtn->setCursor(Qt::PointingHandCursor);
    connect(custBtn,&QPushButton::clicked,[=](){custBtn->setChecked(true);admBtn->setChecked(false);});
    connect(admBtn,&QPushButton::clicked,[=](){admBtn->setChecked(true);custBtn->setChecked(false);});
    rRow->addWidget(custBtn); rRow->addWidget(admBtn);

    auto* emailE=new QLineEdit; emailE->setPlaceholderText("Email address"); emailE->setFixedHeight(50); emailE->setObjectName("lEmail");
    auto* passE=new QLineEdit; passE->setPlaceholderText("Password"); passE->setFixedHeight(50); passE->setEchoMode(QLineEdit::Password); passE->setObjectName("lPass");

    auto* loginBtn=createSolidButton("Sign In");
    loginBtn->setFixedHeight(52);

    auto* regLink=new QPushButton("Don't have an account?  Register →");
    regLink->setStyleSheet(QString("background:transparent;color:%1;border:none;font-size:13px;").arg(ORANGE));
    regLink->setCursor(Qt::PointingHandCursor);

    connect(loginBtn,&QPushButton::clicked,this,&MainWindow::onLogin);
    connect(passE,&QLineEdit::returnPressed,this,&MainWindow::onLogin);
    connect(regLink,&QPushButton::clicked,[this]{mainStack->setCurrentIndex(1);});

    rl->addWidget(hello); rl->addWidget(loginTitle); rl->addWidget(subTxt);
    rl->addSpacing(8); rl->addLayout(rRow);
    rl->addWidget(emailE); rl->addWidget(passE);
    rl->addSpacing(4); rl->addWidget(loginBtn);
    rl->addWidget(regLink,0,Qt::AlignCenter);

    ml->addWidget(lp); ml->addWidget(rp,1);
    mainStack->addWidget(loginPage);
}

// ─── REGISTER PAGE ───────────────────────────────────────────────
void MainWindow::buildRegisterPage(){
    registerPage=new QWidget;
    registerPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* ml=new QVBoxLayout(registerPage); ml->setAlignment(Qt::AlignCenter);

    auto* card=new QWidget; card->setFixedWidth(540);
    card->setStyleSheet(QString(R"(
        background:%1;border-radius:16px;
        border:1px solid #222;
    )").arg(BG_CARD));
    addShadow(card,QColor(249,115,22,30));
    auto* cl=new QVBoxLayout(card); cl->setContentsMargins(46,40,46,40); cl->setSpacing(13);

    auto* rSub=new QLabel("CREATE ACCOUNT");
    rSub->setStyleSheet(QString("color:%1;font-size:10px;letter-spacing:4px;font-weight:700;").arg(ORANGE));

    auto mkF=[](const QString& ph,bool pass=false)->QLineEdit*{
        auto* e=new QLineEdit; e->setPlaceholderText(ph); e->setFixedHeight(48);
        if(pass) e->setEchoMode(QLineEdit::Password); return e;
    };
    auto* rName=mkF("Full Name"); rName->setObjectName("rName");
    auto* rEmail=mkF("Email Address"); rEmail->setObjectName("rEmail");
    auto* rPhone=mkF("Phone Number"); rPhone->setObjectName("rPhone");
    auto* rAddr=mkF("Delivery Address"); rAddr->setObjectName("rAddr");
    auto* rPass=mkF("Password",true); rPass->setObjectName("rPass");
    auto* rConf=mkF("Confirm Password",true); rConf->setObjectName("rConf");

    auto* regBtn=createSolidButton("Create Account"); regBtn->setFixedHeight(52);
    auto* back=new QPushButton("Already registered?  Sign In →");
    back->setStyleSheet(QString("background:transparent;color:%1;border:none;font-size:13px;").arg(ORANGE));
    back->setCursor(Qt::PointingHandCursor);

    connect(regBtn,&QPushButton::clicked,this,&MainWindow::onRegister);
    connect(back,&QPushButton::clicked,[this]{mainStack->setCurrentIndex(0);});

    cl->addWidget(rSub); cl->addWidget(createTitleLabel("Join Foodin"));
    cl->addSpacing(6);
    cl->addWidget(rName); cl->addWidget(rEmail); cl->addWidget(rPhone);
    cl->addWidget(rAddr); cl->addWidget(rPass); cl->addWidget(rConf);
    cl->addSpacing(4); cl->addWidget(regBtn);
    cl->addWidget(back,0,Qt::AlignCenter);
    ml->addWidget(card,0,Qt::AlignCenter);
    mainStack->addWidget(registerPage);
}

// ─── SIDEBAR BUILDER ─────────────────────────────────────────────
QWidget* MainWindow::buildSidebar(const QString& brand,const QStringList& items,
    QStackedWidget* stack,bool isAdmin,QVBoxLayout*& slOut){
    auto* sb=new QWidget; sb->setFixedWidth(200);
    sb->setStyleSheet(QString("background:%1;border-right:1px solid %2;").arg(BG_PANEL).arg(BORDER));
    auto* sl=new QVBoxLayout(sb); sl->setContentsMargins(0,0,0,0); sl->setSpacing(0);

    // Brand
    auto* bh=new QWidget; bh->setFixedHeight(66);
    bh->setStyleSheet("background:#0a0a0a;border-bottom:1px solid #1a1a1a;");
    auto* bhl=new QHBoxLayout(bh); bhl->setContentsMargins(15,0,15,0);
    auto* bIcon=new QWidget; bIcon->setFixedSize(30,30);
    bIcon->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #f97316,stop:1 #ea580c);border-radius:7px;");
    auto* bIconLayout=new QVBoxLayout(bIcon); bIconLayout->setContentsMargins(0,0,0,0);
    auto* bIconLabel=new QLabel("🍽"); bIconLabel->setFont(QFont("Segoe UI Emoji",14));
    bIconLabel->setAlignment(Qt::AlignCenter); bIconLabel->setStyleSheet("background:transparent;");
    bIconLayout->addWidget(bIconLabel);
    auto* bNameW=new QWidget; bNameW->setStyleSheet("background:transparent;");
    auto* bNameL=new QVBoxLayout(bNameW); bNameL->setContentsMargins(0,0,0,0); bNameL->setSpacing(0);
    auto* bNameLabel=new QLabel(brand); bNameLabel->setFont(QFont("Segoe UI",14,QFont::Bold));
    bNameLabel->setStyleSheet(QString("color:%1;background:transparent;").arg(TEXT_MAIN));
    auto* bTagLabel=new QLabel("FOOD ORDERING");
    bTagLabel->setStyleSheet(QString("color:%1;font-size:8px;letter-spacing:1px;background:transparent;").arg(TEXT_DIM));
    bNameL->addWidget(bNameLabel); bNameL->addWidget(bTagLabel);
    bhl->addWidget(bIcon); bhl->addSpacing(8); bhl->addWidget(bNameW); bhl->addStretch();
    sl->addWidget(bh); sl->addSpacing(6);

    // Nav items
    QString ns=QString(R"(
        QPushButton{background:transparent;color:%1;border:none;
            border-left:3px solid transparent;text-align:left;
            padding:12px 16px;font-size:13px;}
        QPushButton:hover{background:#161616;color:%2;
            border-left:3px solid rgba(249,115,22,0.3);}
        QPushButton:checked{background:rgba(249,115,22,0.1);color:%3;
            border-left:3px solid %3;font-weight:600;}
    )").arg(TEXT_DIM).arg(TEXT_MAIN).arg(ORANGE);

    for(int i=0;i<items.size();i++){
        auto* nb=new QPushButton(items[i]);
        nb->setStyleSheet(ns); nb->setCheckable(true);
        nb->setCursor(Qt::PointingHandCursor); nb->setFixedHeight(46);
        if(i==0) nb->setChecked(true);
        connect(nb,&QPushButton::clicked,[=]{
            stack->setCurrentIndex(i);
            for(auto* b:sb->findChildren<QPushButton*>()) b->setChecked(false);
            nb->setChecked(true);
            if(!isAdmin){if(i==2)refreshCart();if(i==3)refreshOrders();}
            else{if(i==1)refreshAdminOrders();if(i==2)refreshAnalytics();}
        });
        sl->addWidget(nb);
    }
    sl->addStretch(); slOut=sl; return sb;
}

// ─── CUSTOMER DASHBOARD ──────────────────────────────────────────
void MainWindow::buildCustomerDashboard(){
    customerDashboard=new QWidget;
    auto* ml=new QHBoxLayout(customerDashboard);
    ml->setContentsMargins(0,0,0,0); ml->setSpacing(0);

    customerStack=new QStackedWidget;
    buildHomePage();
    buildMenuPage();
    buildCartPage();
    buildOrdersPage();
    buildProfilePage();

    QVBoxLayout* sl;
    auto* sb=buildSidebar("Foodin",
        {"🏠  Home","🍕  Menu","🛒  Cart","📋  Orders","👤  Profile"},
        customerStack,false,sl);

    cartBadge=new QLabel("0");
    cartBadge->setStyleSheet(
        "background:#f97316;color:#fff;border-radius:8px;"
        "font-size:9px;font-weight:700;padding:1px 5px;");
    cartBadge->hide();

    welcomeLabel=new QLabel("Welcome!");
    welcomeLabel->setStyleSheet(QString(
        "color:%1;font-size:11px;padding:5px 16px;background:transparent;").arg(TEXT_DIM));
    sl->addWidget(welcomeLabel);

    loyaltyPointsLabel=new QLabel("⭐ 0 pts");
    loyaltyPointsLabel->setStyleSheet(
        "color:#f59e0b;font-size:11px;padding:3px 16px;background:transparent;");
    sl->addWidget(loyaltyPointsLabel);

    sl->addWidget(createSeparator());
    auto* lo=new QPushButton("⏻   Sign Out");
    lo->setStyleSheet(R"(
        QPushButton{background:transparent;color:rgba(239,68,68,0.7);border:none;
            border-top:1px solid #1a1a1a;padding:14px 16px;font-size:13px;text-align:left;}
        QPushButton:hover{color:#ef4444;background:#161616;}
    )");
    connect(lo,&QPushButton::clicked,this,&MainWindow::onLogout);
    sl->addWidget(lo);

    ml->addWidget(sb); ml->addWidget(customerStack,1);
    mainStack->addWidget(customerDashboard);
}

// ─── HOME PAGE ───────────────────────────────────────────────────
void MainWindow::buildHomePage(){
    homePage=new QWidget;
    homePage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(homePage);
    l->setContentsMargins(0,0,0,0); l->setSpacing(0);

    // ── HERO BANNER ──
    auto* hero=new QWidget; hero->setFixedHeight(260);
    hero->setStyleSheet("background:#0a0500;");
    auto* heroLayout=new QStackedLayout(hero);
    heroLayout->setStackingMode(QStackedLayout::StackAll);

    // Background image
    auto* heroBg=new QLabel;
    heroBg->setScaledContents(true);
    QPixmap bannerPx;
    // Try multiple paths
    QStringList bannerPaths={
        "resources/images/banner.jpg",
        ":/images/banner.jpg",
        "resources/images/burger.jpg",
        ":/images/burger.jpg"
    };
    for(const auto& path:bannerPaths){
        bannerPx.load(path);
        if(!bannerPx.isNull()) break;
    }
    if(false){
    // image disabled
    } else {
        heroBg->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1a0500,stop:1 #0a0a0a);");
    }

    // Dark gradient overlay
    auto* overlay=new QWidget;
    overlay->setStyleSheet(R"(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 rgba(5,2,0,0.97),"
        "stop:0.5 rgba(5,2,0,0.80),"
        "stop:1 rgba(5,2,0,0.50));");
    )");

    // Content on top
    auto* content=new QWidget;
    content->setStyleSheet("background:transparent;");
    auto* contentLayout=new QVBoxLayout(content);
    contentLayout->setContentsMargins(40,30,40,22);
    contentLayout->setSpacing(8);

    auto* tagLine=new QLabel("🔥  Fresh & Hot — Order Now");
    tagLine->setStyleSheet(QString(R"(
        background:rgba(249,115,22,0.15);
        border:1px solid rgba(249,115,22,0.35);
        color:%1;font-size:10px;font-weight:700;letter-spacing:2px;
        padding:5px 12px;border-radius:20px;
    )").arg(ORANGE));
    tagLine->setFixedWidth(220);

    auto* heroTitle=new QLabel;
    heroTitle->setText("<span style='color:#ffffff;font-size:28px;font-weight:800;'>Hungry? Order<br>"
                       "<span style='color:#f97316;'>Delicious Food</span><br>Right Now!</span>");
    heroTitle->setTextFormat(Qt::RichText);
    heroTitle->setStyleSheet("background:transparent;");

    auto* heroSub=new QLabel("Fresh meals delivered to your door in 30–45 minutes");
    heroSub->setStyleSheet("color:rgba(255,255,255,0.55);font-size:12px;background:transparent;");

    auto* heroBtns=new QHBoxLayout;
    auto* orderBtn=createSolidButton("🍕  Order Now");
    orderBtn->setFixedHeight(44);
    auto* menuBtn=new QPushButton("View Menu  →");
    menuBtn->setStyleSheet(R"(
        QPushButton{background:rgba(255,255,255,0.1);color:#fff;
            border:1px solid rgba(255,255,255,0.2);border-radius:9px;
            padding:10px 16px;font-size:13px;font-weight:600;}
        QPushButton:hover{background:rgba(255,255,255,0.18);border-color:rgba(255,255,255,0.4);}
    )");
    menuBtn->setFixedHeight(44);
    menuBtn->setCursor(Qt::PointingHandCursor);
    connect(orderBtn,&QPushButton::clicked,[this]{customerStack->setCurrentIndex(1);});
    connect(menuBtn,&QPushButton::clicked,[this]{customerStack->setCurrentIndex(1);});
    heroBtns->addWidget(orderBtn); heroBtns->addWidget(menuBtn); heroBtns->addStretch();

    // Stats row bottom right
    auto* statsRow=new QHBoxLayout;
    statsRow->addStretch();
    QStringList svals={"50+\nItems","30 Min\nDelivery","4.8★\nRating"};
    for(const auto& sv:svals){
        QStringList p=sv.split("\n");
        auto* sw=new QWidget; sw->setStyleSheet("background:transparent;");
        auto* swl=new QVBoxLayout(sw); swl->setContentsMargins(0,0,0,0); swl->setSpacing(0);
        auto* snum=new QLabel(p[0]);
        snum->setStyleSheet(QString("color:%1;font-size:16px;font-weight:800;background:transparent;").arg(ORANGE));
        snum->setAlignment(Qt::AlignCenter);
        auto* slbl=new QLabel(p[1]);
        slbl->setStyleSheet("color:rgba(255,255,255,0.4);font-size:9px;letter-spacing:1px;background:transparent;");
        slbl->setAlignment(Qt::AlignCenter);
        swl->addWidget(snum); swl->addWidget(slbl);
        statsRow->addWidget(sw); statsRow->addSpacing(14);
    }

    contentLayout->addWidget(tagLine); contentLayout->addSpacing(4);
    contentLayout->addWidget(heroTitle); contentLayout->addWidget(heroSub);
    contentLayout->addSpacing(8); contentLayout->addLayout(heroBtns);
    contentLayout->addStretch(); contentLayout->addLayout(statsRow);

    heroLayout->addWidget(heroBg);
    heroLayout->addWidget(overlay);
    heroLayout->addWidget(content);
    heroLayout->setCurrentIndex(2);
    // ── CATEGORIES ──
    auto* catsWrap=new QWidget; catsWrap->setFixedHeight(100);
    catsWrap->setStyleSheet(QString("background:%1;border-bottom:1px solid %2;").arg(BG_MAIN).arg(BORDER));
    auto* catsLayout=new QVBoxLayout(catsWrap);
    catsLayout->setContentsMargins(24,10,24,8);

    auto* catsTitle=new QLabel("BROWSE CATEGORIES");
    catsTitle->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:2px;font-weight:700;").arg(TEXT_DIM));
    catsLayout->addWidget(catsTitle);

    auto* catsScroll=new QScrollArea; catsScroll->setFixedHeight(62);
    catsScroll->setWidgetResizable(true); catsScroll->setFrameShape(QFrame::NoFrame);
    catsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    catsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    catsScroll->setStyleSheet("background:transparent;");

    auto* catsContainer=new QWidget; catsContainer->setStyleSheet("background:transparent;");
    auto* catsHLayout=new QHBoxLayout(catsContainer);
    catsHLayout->setContentsMargins(0,0,0,0); catsHLayout->setSpacing(8);

    struct CatItem{ QString emoji; QString label; };
    QList<CatItem> catItems={
        {"🍔","Burgers"},{"🍕","Pizza"},{"🍚","Rice"},{"🍛","Curries"},
        {"🥙","Starters"},{"🍰","Desserts"},{"🥤","Drinks"},{"🫓","Bread"},{"🍟","Sides"}
    };
    for(const auto& ci:catItems){
        auto* cw=new QWidget; cw->setFixedSize(58,56);
        cw->setStyleSheet("background:transparent;");
        cw->setCursor(Qt::PointingHandCursor);
        auto* cwl=new QVBoxLayout(cw); cwl->setContentsMargins(0,0,0,0); cwl->setSpacing(2);

        auto* iconBox=new QLabel(ci.emoji);
        iconBox->setFixedSize(42,42); iconBox->setAlignment(Qt::AlignCenter);
        iconBox->setFont(QFont("Segoe UI Emoji",18));
        iconBox->setStyleSheet(R"(
            background:#1c1c1c;border:1px solid #262626;border-radius:11px;
        )");
        iconBox->setStyleSheet(QString(R"(
            background:%1;border:1px solid %2;border-radius:11px;
        )").arg(BG_CARD).arg(BORDER2));

        auto* catLbl=new QLabel(ci.label);
        catLbl->setAlignment(Qt::AlignCenter);
        catLbl->setStyleSheet(QString("color:%1;font-size:9px;font-weight:600;background:transparent;").arg(TEXT_DIM));
        catLbl->setFixedWidth(58);

        QString cat=ci.label;
        connect(cw,&QWidget::destroyed,[]{});
        // Use event filter approach via QPushButton invisible
        auto* catBtn=new QPushButton(cw);
        catBtn->setGeometry(0,0,58,56);
        catBtn->setStyleSheet("background:transparent;border:none;");
        connect(catBtn,&QPushButton::clicked,[this,cat]{
            customerStack->setCurrentIndex(1);
            QTimer::singleShot(100,[this,cat]{
                if(categoryFilter) categoryFilter->setCurrentText(cat);
            });
        });

        cwl->addWidget(iconBox,0,Qt::AlignCenter);
        cwl->addWidget(catLbl);
        catsHLayout->addWidget(cw);
    }
    catsHLayout->addStretch();
    catsScroll->setWidget(catsContainer);
    catsLayout->addWidget(catsScroll);

    // ── POPULAR ITEMS ──
    auto* popSection=new QWidget;
    popSection->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* popLayout=new QVBoxLayout(popSection);
    popLayout->setContentsMargins(24,16,24,16); popLayout->setSpacing(12);

    auto* popHdr=new QHBoxLayout;
    auto* popTitle=new QLabel("🔥  Popular This Week");
    popTitle->setFont(QFont("Segoe UI",14,QFont::Bold));
    popTitle->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));
    auto* seeAll=new QPushButton("See all  →");
    seeAll->setStyleSheet(QString(
        "QPushButton{background:transparent;color:%1;border:none;font-size:12px;}"
        "QPushButton:hover{color:%2;}").arg(ORANGE).arg(ORANGE_D));
    seeAll->setCursor(Qt::PointingHandCursor);
    connect(seeAll,&QPushButton::clicked,[this]{customerStack->setCurrentIndex(1);});
    popHdr->addWidget(popTitle); popHdr->addStretch(); popHdr->addWidget(seeAll);

    // Popular grid (3 cols) — loaded after init
    popularContainer=new QWidget; popularContainer->setStyleSheet("background:transparent;");
    auto* popGrid=new QGridLayout(popularContainer);
    popGrid->setSpacing(12); popGrid->setContentsMargins(0,0,0,0);

    auto* popScroll=new QScrollArea;
    popScroll->setWidgetResizable(true); popScroll->setFrameShape(QFrame::NoFrame);
    popScroll->setStyleSheet("background:transparent;");
    popScroll->setWidget(popularContainer);

    popLayout->addLayout(popHdr);
    popLayout->addWidget(popScroll,1);

    l->addWidget(hero);
    l->addWidget(catsWrap);
    l->addWidget(popSection,1);

    customerStack->addWidget(homePage);
    QTimer::singleShot(300,this,[this]{ refreshHomePage(); });
}

void MainWindow::refreshHomePage(){
    try{
        auto items=foodService->getAllMenuItems();
        // Show top 6 by rating
        std::sort(items.begin(),items.end(),[](const MenuItem& a,const MenuItem& b){
            return a.getRating()>b.getRating();
        });
        if(items.size()>6) items.resize(6);

        auto* grid=qobject_cast<QGridLayout*>(popularContainer->layout());
        if(!grid) return;
        while(grid->count()){
            auto* it=grid->takeAt(0);
            if(it->widget()) it->widget()->deleteLater();
            delete it;
        }

        int cols=3;
        for(int i=0;i<(int)items.size();i++)
            grid->addWidget(createMenuItemCard(items[i],true),i/cols,i%cols);
    } catch(...){}
}

// ─── MENU PAGE ───────────────────────────────────────────────────
void MainWindow::buildMenuPage(){
    menuPage=new QWidget;
    menuPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(menuPage);
    l->setContentsMargins(22,22,22,22); l->setSpacing(14);

    auto* hdr=new QHBoxLayout;
    auto* titleW=new QWidget; titleW->setStyleSheet("background:transparent;");
    auto* tl=new QVBoxLayout(titleW); tl->setContentsMargins(0,0,0,0); tl->setSpacing(2);
    auto* sub=new QLabel("EXPLORE"); sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    auto* title=new QLabel("Our Menu"); title->setFont(QFont("Segoe UI",20,QFont::Bold));
    title->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));
    tl->addWidget(sub); tl->addWidget(title);

    hdr->addWidget(titleW); hdr->addStretch();
    searchBar=new QLineEdit; searchBar->setPlaceholderText("Search dishes...");
    searchBar->setFixedHeight(42); searchBar->setFixedWidth(250);
    categoryFilter=new QComboBox; categoryFilter->setFixedHeight(42); categoryFilter->setFixedWidth(190);
    categoryFilter->addItem("All Categories");
    connect(searchBar,&QLineEdit::textChanged,this,&MainWindow::onSearch);
    connect(categoryFilter,&QComboBox::currentTextChanged,this,&MainWindow::onCategoryFilter);
    hdr->addWidget(searchBar); hdr->addWidget(categoryFilter);
    l->addLayout(hdr); l->addWidget(createSeparator());

    menuScrollArea=new QScrollArea; menuScrollArea->setWidgetResizable(true);
    menuScrollArea->setFrameShape(QFrame::NoFrame); menuScrollArea->setStyleSheet("background:transparent;");
    menuItemsContainer=new QWidget; menuItemsContainer->setStyleSheet("background:transparent;");
    new QGridLayout(menuItemsContainer);
    menuScrollArea->setWidget(menuItemsContainer);
    l->addWidget(menuScrollArea,1);
    customerStack->addWidget(menuPage);
    QTimer::singleShot(300,this,[this]{refreshMenu();});
}

// ─── MENU ITEM CARD ──────────────────────────────────────────────
QWidget* MainWindow::createMenuItemCard(const MenuItem& item,bool compact){
    int cardW=compact?240:265;
    int cardH=compact?295:330;
    int imgH=compact?110:135;

    auto* card=new QWidget; card->setFixedSize(cardW,cardH);
    card->setStyleSheet(QString(R"(
        QWidget{background:%1;border-radius:13px;border:1px solid %2;}
        QWidget:hover{border:1px solid rgba(249,115,22,0.55);background:%3;}
    )").arg(BG_CARD).arg(BORDER).arg(BG_HOVER));

    auto* vl=new QVBoxLayout(card); vl->setContentsMargins(0,0,0,12); vl->setSpacing(0);

    // Image
    auto* imgLabel=new QLabel; imgLabel->setFixedSize(cardW,imgH);
    imgLabel->setAlignment(Qt::AlignCenter);
    imgLabel->setStyleSheet(QString("border-radius:13px 13px 0 0;background:#1c1c1c;"));

    QPixmap px;
    bool loaded=false;
    if(!item.getImagePath().empty()){
        px.load(QString::fromStdString(item.getImagePath()));
        if(!px.isNull()) loaded=true;
    }
    if(!loaded){
        QFileInfo fi(QString::fromStdString(item.getImagePath()));
        px.load(":/images/"+fi.fileName());
        if(!px.isNull()) loaded=true;
    }
    if(loaded){
        imgLabel->setPixmap(px.scaled(cardW,imgH,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
        imgLabel->setScaledContents(false);
    } else {
        QString em="🍽️";
        std::string n=item.getName();
        if(n.find("Burger")!=std::string::npos) em="🍔";
        else if(n.find("Pizza")!=std::string::npos) em="🍕";
        else if(n.find("Biryani")!=std::string::npos) em="🍚";
        else if(n.find("Karahi")!=std::string::npos||n.find("Nihari")!=std::string::npos) em="🍛";
        else if(n.find("Lassi")!=std::string::npos) em="🥛";
        else if(n.find("Naan")!=std::string::npos) em="🫓";
        else if(n.find("Brownie")!=std::string::npos) em="🍫";
        else if(n.find("Fries")!=std::string::npos) em="🍟";
        else if(n.find("Kabab")!=std::string::npos) em="🥙";
        imgLabel->setText(em);
        imgLabel->setFont(QFont("Segoe UI Emoji",compact?32:40));
        imgLabel->setStyleSheet(
            "border-radius:13px 13px 0 0;"
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #1c1c1c,stop:1 #222);");
    }

    // Category badge
    auto* badge=new QLabel(QString::fromStdString(item.getCategory()),card);
    badge->setStyleSheet(R"(
        background:rgba(249,115,22,0.12);color:#f97316;
        border:1px solid rgba(249,115,22,0.25);
        border-radius:5px;padding:2px 7px;font-size:9px;font-weight:700;letter-spacing:1px;
    )");
    badge->adjustSize(); badge->move(10,imgH-26);

    // Info
    auto* info=new QWidget; info->setStyleSheet("background:transparent;");
    auto* il=new QVBoxLayout(info); il->setContentsMargins(12,9,12,0); il->setSpacing(3);

    auto* nm=new QLabel(QString::fromStdString(item.getName()));
    nm->setFont(QFont("Segoe UI",compact?11:12,QFont::Bold));
    nm->setStyleSheet(QString("color:%1;background:transparent;").arg(TEXT_MAIN));
    nm->setWordWrap(true);

    if(!compact){
        auto* ds=new QLabel(QString::fromStdString(item.getDescription()));
        ds->setStyleSheet(QString("color:%1;font-size:10px;background:transparent;").arg(TEXT_DIM));
        ds->setWordWrap(true); ds->setMaximumHeight(28);
        il->addWidget(ds);
    }

    double rat=item.getRating();
    QString stars="";
    for(int i=1;i<=5;i++) stars+=(i<=(int)rat?"★":"☆");
    auto* starL=new QLabel(stars+QString(" (%1)").arg(item.getTotalRatings()));
    starL->setStyleSheet("color:#f59e0b;font-size:10px;background:transparent;");

    auto* pr=new QHBoxLayout;
    auto* prL=new QLabel(QString("PKR %1").arg(item.getPrice(),0,'f',0));
    prL->setFont(QFont("Segoe UI",compact?12:13,QFont::Bold));
    prL->setStyleSheet(QString("color:%1;background:transparent;").arg(ORANGE));
    auto* tL=new QLabel(QString("⏱ %1m").arg(item.getPrepTime()));
    tL->setStyleSheet(QString("color:%1;font-size:10px;background:transparent;").arg(TEXT_DIM));
    pr->addWidget(prL); pr->addStretch(); pr->addWidget(tL);

    int id=item.getItemId();
    auto* addBtn=item.isAvailable()?createSolidButton("Add to Cart"):new QPushButton("Unavailable");
    if(!item.isAvailable()){
        addBtn->setStyleSheet(QString("QPushButton{background:#1e1e1e;color:%1;border:1px solid #2a2a2a;border-radius:8px;padding:8px;}").arg(TEXT_DIM));
        addBtn->setEnabled(false);
    }
    addBtn->setFixedHeight(34);
    connect(addBtn,&QPushButton::clicked,[this,id]{onAddToCart(id);});

    il->addWidget(nm); il->addWidget(starL); il->addLayout(pr); il->addWidget(addBtn);
    vl->addWidget(imgLabel); vl->addWidget(info,1);
    return card;
}

// ─── CART PAGE ───────────────────────────────────────────────────
void MainWindow::buildCartPage(){
    cartPage=new QWidget;
    cartPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(cartPage);
    l->setContentsMargins(22,22,22,22); l->setSpacing(14);

    auto* sub=new QLabel("YOUR ORDER"); sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    l->addWidget(sub); l->addWidget(createTitleLabel("Cart")); l->addWidget(createSeparator());

    auto* hl=new QHBoxLayout;
    auto* sc=new QScrollArea; sc->setWidgetResizable(true); sc->setFrameShape(QFrame::NoFrame);
    sc->setStyleSheet("background:transparent;");
    cartContainer=new QWidget; cartContainer->setStyleSheet("background:transparent;");
    new QVBoxLayout(cartContainer); sc->setWidget(cartContainer);

    auto* sp=new QWidget; sp->setFixedWidth(300);
    sp->setStyleSheet(QString("background:%1;border-radius:14px;border:1px solid #222;").arg(BG_CARD));
    addShadow(sp,QColor(249,115,22,20));
    auto* sl=new QVBoxLayout(sp); sl->setContentsMargins(20,20,20,20); sl->setSpacing(11);

    auto* st=new QLabel("ORDER SUMMARY");
    st->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    cartTotalLabel=new QLabel("Total: PKR 0");
    cartTotalLabel->setFont(QFont("Segoe UI",17,QFont::Bold));
    cartTotalLabel->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));
    auto* df=new QLabel("🚚  Delivery Fee: PKR 50");
    df->setStyleSheet(QString("color:%1;font-size:12px;").arg(TEXT_DIM));

    auto* pmL=new QLabel("PAYMENT");
    pmL->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:2px;").arg(TEXT_DIM));
    paymentCombo=new QComboBox;
    paymentCombo->addItems({"Cash on Delivery","Credit Card","Debit Card","Mobile Wallet"});
    paymentCombo->setFixedHeight(42);

    auto* ntL=new QLabel("SPECIAL NOTES");
    ntL->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:2px;").arg(TEXT_DIM));
    notesEdit=new QTextEdit; notesEdit->setPlaceholderText("Any requests?"); notesEdit->setFixedHeight(68);

    auto* plBtn=createSolidButton("Place Order"); plBtn->setFixedHeight(50);
    addShadow(plBtn,QColor(249,115,22,50));
    connect(plBtn,&QPushButton::clicked,this,&MainWindow::onPlaceOrder);

    auto* clBtn=createStyledButton("Clear Cart","#ef4444"); clBtn->setFixedHeight(38);
    clBtn->setStyleSheet(R"(
        QPushButton{background:transparent;color:#ef4444;border:1px solid rgba(239,68,68,0.35);border-radius:9px;padding:9px;}
        QPushButton:hover{background:rgba(239,68,68,0.1);border-color:#ef4444;}
    )");
    connect(clBtn,&QPushButton::clicked,[this]{cartItems.clear();refreshCart();updateCartBadge();});

    sl->addWidget(st); sl->addWidget(createSeparator());
    sl->addWidget(cartTotalLabel); sl->addWidget(df); sl->addWidget(createSeparator());
    sl->addWidget(pmL); sl->addWidget(paymentCombo);
    sl->addWidget(ntL); sl->addWidget(notesEdit);
    sl->addStretch(); sl->addWidget(plBtn); sl->addWidget(clBtn);

    hl->addWidget(sc,1); hl->addWidget(sp);
    l->addLayout(hl,1);
    customerStack->addWidget(cartPage);
}

// ─── ORDERS PAGE ─────────────────────────────────────────────────
void MainWindow::buildOrdersPage(){
    ordersPage=new QWidget;
    ordersPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(ordersPage);
    l->setContentsMargins(22,22,22,22); l->setSpacing(14);

    auto* hr=new QHBoxLayout;
    auto* sub=new QLabel("TRACK YOUR"); sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    auto* tw=new QWidget; tw->setStyleSheet("background:transparent;");
    auto* twl=new QVBoxLayout(tw); twl->setContentsMargins(0,0,0,0); twl->setSpacing(2);
    twl->addWidget(sub); twl->addWidget(createTitleLabel("Orders"));
    hr->addWidget(tw); hr->addStretch();
    auto* rb=createStyledButton("↻  Refresh"); rb->setFixedHeight(38);
    connect(rb,&QPushButton::clicked,this,&MainWindow::refreshOrders);
    hr->addWidget(rb); l->addLayout(hr); l->addWidget(createSeparator());

    ordersTable=new QTableWidget(0,6);
    ordersTable->setHorizontalHeaderLabels({"Order #","Date","Status","Total","Payment","Action"});
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ordersTable->verticalHeader()->setVisible(false);
    ordersTable->setColumnWidth(0,80); ordersTable->setColumnWidth(1,170);
    ordersTable->setColumnWidth(2,120); ordersTable->setColumnWidth(3,100);
    ordersTable->setColumnWidth(4,140);
    l->addWidget(ordersTable,1);
    customerStack->addWidget(ordersPage);
}

// ─── PROFILE PAGE ────────────────────────────────────────────────
void MainWindow::buildProfilePage(){
    profilePage=new QWidget;
    profilePage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(profilePage);
    l->setAlignment(Qt::AlignCenter); l->setContentsMargins(60,40,60,40); l->setSpacing(18);

    auto* sub=new QLabel("YOUR"); sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    l->addWidget(sub);
    auto* t=createTitleLabel("Profile"); t->setAlignment(Qt::AlignCenter);
    l->addWidget(t); l->addWidget(createSeparator());

    auto* card=new QWidget; card->setFixedWidth(480);
    card->setStyleSheet(QString("background:%1;border-radius:16px;border:1px solid #222;").arg(BG_CARD));
    addShadow(card,QColor(249,115,22,20));
    auto* cl=new QVBoxLayout(card); cl->setContentsMargins(36,36,36,36); cl->setSpacing(14);

    auto* av=new QLabel("👤");
    av->setFont(QFont("Segoe UI Emoji",52)); av->setAlignment(Qt::AlignCenter);
    av->setStyleSheet("background:transparent;"); cl->addWidget(av);

    auto row=[&](const QString& lbl,const QString& obj){
        auto* w=new QWidget; w->setStyleSheet("background:transparent;");
        auto* rl=new QHBoxLayout(w); rl->setContentsMargins(0,0,0,0);
        auto* ll2=new QLabel(lbl);
        ll2->setStyleSheet(QString("color:%1;font-size:10px;letter-spacing:1px;").arg(TEXT_DIM));
        ll2->setFixedWidth(110);
        auto* vl2=new QLabel("—");
        vl2->setStyleSheet(QString("color:%1;font-size:13px;").arg(TEXT_MAIN));
        vl2->setObjectName(obj);
        rl->addWidget(ll2); rl->addWidget(vl2,1); cl->addWidget(w);
    };
    row("NAME","pName"); row("EMAIL","pEmail");
    row("PHONE","pPhone"); row("ADDRESS","pAddress"); row("LOYALTY PTS","pPoints");
    l->addWidget(card,0,Qt::AlignCenter); l->addStretch();
    customerStack->addWidget(profilePage);
}

// ─── ADMIN DASHBOARD ─────────────────────────────────────────────
void MainWindow::buildAdminDashboard(){
    adminDashboard=new QWidget;
    auto* ml=new QHBoxLayout(adminDashboard);
    ml->setContentsMargins(0,0,0,0); ml->setSpacing(0);
    adminStack=new QStackedWidget;
    buildAdminMenuPage(); buildAdminOrdersPage(); buildAdminAnalyticsPage();

    QVBoxLayout* sl;
    auto* sb=buildSidebar("Admin",
        {"🍽️  Menu Mgmt","📦  Orders","📊  Analytics"},
        adminStack,true,sl);

    auto* lo=new QPushButton("⏻   Sign Out");
    lo->setStyleSheet(R"(
        QPushButton{background:transparent;color:rgba(239,68,68,0.7);border:none;
            border-top:1px solid #1a1a1a;padding:14px 16px;font-size:13px;text-align:left;}
        QPushButton:hover{color:#ef4444;background:#161616;}
    )");
    connect(lo,&QPushButton::clicked,this,&MainWindow::onLogout);
    sl->addWidget(lo);
    ml->addWidget(sb); ml->addWidget(adminStack,1);
    mainStack->addWidget(adminDashboard);
}

// ─── ADMIN MENU PAGE ─────────────────────────────────────────────
void MainWindow::buildAdminMenuPage(){
    adminMenuPage=new QWidget;
    adminMenuPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* ml=new QHBoxLayout(adminMenuPage);
    ml->setContentsMargins(22,22,22,22); ml->setSpacing(18);

    auto* fp=new QWidget; fp->setFixedWidth(295);
    fp->setStyleSheet(QString("background:%1;border-radius:14px;border:1px solid #222;").arg(BG_CARD));
    auto* fl=new QVBoxLayout(fp); fl->setContentsMargins(18,18,18,18); fl->setSpacing(10);

    auto* fSub=new QLabel("MANAGE"); fSub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    auto* fTitle=createTitleLabel("Menu Items"); fTitle->setStyleSheet(QString("color:%1;font-size:16px;font-weight:bold;").arg(TEXT_MAIN));

    auto mkI=[](const QString& ph)->QLineEdit*{
        auto* e=new QLineEdit; e->setPlaceholderText(ph); e->setFixedHeight(42); return e;
    };
    itemNameEdit=mkI("Item Name"); itemDescEdit=mkI("Description");
    itemPriceEdit=mkI("Price (PKR)"); itemCategoryEdit=mkI("Category");
    itemPrepEdit=mkI("Prep Time (mins)");

    auto* imgRow=new QHBoxLayout;
    itemImageEdit=mkI("Image path");
    auto* brBtn=createStyledButton("📁"); brBtn->setFixedSize(42,42);
    connect(brBtn,&QPushButton::clicked,this,&MainWindow::onBrowseImage);
    imgRow->addWidget(itemImageEdit,1); imgRow->addWidget(brBtn);

    auto* addBtn=createSolidButton("➕  Add Item"); addBtn->setFixedHeight(44);
    connect(addBtn,&QPushButton::clicked,this,&MainWindow::onAddMenuItem);
    auto* updBtn=createStyledButton("✏️  Update"); updBtn->setFixedHeight(44);
    connect(updBtn,&QPushButton::clicked,this,&MainWindow::onUpdateMenuItem);
    auto* clrBtn=new QPushButton("Clear");
    clrBtn->setStyleSheet(QString("QPushButton{background:transparent;color:%1;border:1px solid #2a2a2a;border-radius:9px;padding:8px;}QPushButton:hover{background:#1a1a1a;color:%2;}").arg(TEXT_DIM).arg(TEXT_MAIN));
    clrBtn->setCursor(Qt::PointingHandCursor);
    connect(clrBtn,&QPushButton::clicked,[this]{
        itemNameEdit->clear();itemDescEdit->clear();itemPriceEdit->clear();
        itemCategoryEdit->clear();itemPrepEdit->clear();itemImageEdit->clear();editingItemId=-1;
    });

    fl->addWidget(fSub); fl->addWidget(fTitle); fl->addWidget(createSeparator());
    fl->addWidget(itemNameEdit); fl->addWidget(itemDescEdit); fl->addWidget(itemPriceEdit);
    fl->addWidget(itemCategoryEdit); fl->addWidget(itemPrepEdit); fl->addLayout(imgRow);
    fl->addStretch(); fl->addWidget(addBtn); fl->addWidget(updBtn); fl->addWidget(clrBtn);

    auto* tp=new QWidget;
    auto* tl=new QVBoxLayout(tp); tl->setContentsMargins(0,0,0,0); tl->setSpacing(12);
    auto* th=new QHBoxLayout;
    th->addWidget(createTitleLabel("All Items")); th->addStretch();
    auto* refBtn=createStyledButton("↻"); refBtn->setFixedHeight(36); refBtn->setFixedWidth(50);
    connect(refBtn,&QPushButton::clicked,this,&MainWindow::refreshAdminMenuTable);
    auto* expBtn=createStyledButton("↓ CSV"); expBtn->setFixedHeight(36);
    connect(expBtn,&QPushButton::clicked,this,&MainWindow::onExportMenu);
    th->addWidget(refBtn); th->addWidget(expBtn); tl->addLayout(th);

    adminMenuTable=new QTableWidget(0,7);
    adminMenuTable->setHorizontalHeaderLabels({"ID","Name","Price","Category","Avail","Edit","Del"});
    adminMenuTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    adminMenuTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    adminMenuTable->verticalHeader()->setVisible(false);
    adminMenuTable->setColumnWidth(0,44); adminMenuTable->setColumnWidth(1,175);
    adminMenuTable->setColumnWidth(2,90); adminMenuTable->setColumnWidth(3,120);
    adminMenuTable->setColumnWidth(4,64); adminMenuTable->setColumnWidth(5,54);
    adminMenuTable->setColumnWidth(6,54);
    tl->addWidget(adminMenuTable,1);
    ml->addWidget(fp); ml->addWidget(tp,1);
    adminStack->addWidget(adminMenuPage);
    QTimer::singleShot(400,this,[this]{refreshAdminMenuTable();});
}

void MainWindow::refreshAdminMenuTable(){
    try{
        auto items=foodService->getAllMenuItems();
        adminMenuTable->setRowCount(0);
        for(const auto& item:items){
            int r=adminMenuTable->rowCount(); adminMenuTable->insertRow(r);
            adminMenuTable->setItem(r,0,new QTableWidgetItem(QString::number(item.getItemId())));
            adminMenuTable->setItem(r,1,new QTableWidgetItem(QString::fromStdString(item.getName())));
            adminMenuTable->setItem(r,2,new QTableWidgetItem(QString("PKR %1").arg(item.getPrice(),0,'f',0)));
            adminMenuTable->setItem(r,3,new QTableWidgetItem(QString::fromStdString(item.getCategory())));
            int id=item.getItemId();
            auto* togBtn=new QPushButton(item.isAvailable()?"✅":"❌");
            togBtn->setStyleSheet("background:transparent;border:none;font-size:14px;");
            connect(togBtn,&QPushButton::clicked,[this,id]{onToggleAvail(id);});
            adminMenuTable->setCellWidget(r,4,togBtn);
            MenuItem it=item;
            auto* editBtn=new QPushButton("✏");
            editBtn->setStyleSheet(QString("background:rgba(249,115,22,0.1);color:%1;border:1px solid rgba(249,115,22,0.25);border-radius:6px;padding:4px 8px;").arg(ORANGE));
            connect(editBtn,&QPushButton::clicked,[this,it]{
                editingItemId=it.getItemId();
                itemNameEdit->setText(QString::fromStdString(it.getName()));
                itemDescEdit->setText(QString::fromStdString(it.getDescription()));
                itemPriceEdit->setText(QString::number(it.getPrice()));
                itemCategoryEdit->setText(QString::fromStdString(it.getCategory()));
                itemPrepEdit->setText(QString::number(it.getPrepTime()));
                itemImageEdit->setText(QString::fromStdString(it.getImagePath()));
            });
            adminMenuTable->setCellWidget(r,5,editBtn);
            auto* delBtn=new QPushButton("🗑");
            delBtn->setStyleSheet("background:rgba(239,68,68,0.1);color:#ef4444;border:1px solid rgba(239,68,68,0.25);border-radius:6px;padding:4px 8px;");
            connect(delBtn,&QPushButton::clicked,[this,id]{onDeleteMenuItem(id);});
            adminMenuTable->setCellWidget(r,6,delBtn);
            adminMenuTable->setRowHeight(r,42);
        }
    } catch(const std::exception& e){showNotif("Error: "+QString(e.what()),false);}
}

// ─── ADMIN ORDERS PAGE ───────────────────────────────────────────
void MainWindow::buildAdminOrdersPage(){
    adminOrdersPage=new QWidget;
    adminOrdersPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(adminOrdersPage);
    l->setContentsMargins(22,22,22,22); l->setSpacing(14);
    auto* hr=new QHBoxLayout;
    auto* sub=new QLabel("MANAGE"); sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    auto* tw=new QWidget; tw->setStyleSheet("background:transparent;");
    auto* twl=new QVBoxLayout(tw); twl->setContentsMargins(0,0,0,0); twl->setSpacing(2);
    twl->addWidget(sub); twl->addWidget(createTitleLabel("Orders"));
    hr->addWidget(tw); hr->addStretch();
    auto* rb=createStyledButton("↻ Refresh"); rb->setFixedHeight(36);
    connect(rb,&QPushButton::clicked,this,&MainWindow::refreshAdminOrders);
    auto* eb=createStyledButton("↓ Export"); eb->setFixedHeight(36);
    connect(eb,&QPushButton::clicked,this,&MainWindow::onExportOrders);
    hr->addWidget(rb); hr->addWidget(eb);
    l->addLayout(hr); l->addWidget(createSeparator());
    adminOrdersTable=new QTableWidget(0,7);
    adminOrdersTable->setHorizontalHeaderLabels({"#","Cust","Total","Payment","Status","Update","Time"});
    adminOrdersTable->horizontalHeader()->setStretchLastSection(true);
    adminOrdersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    adminOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    adminOrdersTable->verticalHeader()->setVisible(false);
    adminOrdersTable->setColumnWidth(0,55); adminOrdersTable->setColumnWidth(1,70);
    adminOrdersTable->setColumnWidth(2,100); adminOrdersTable->setColumnWidth(3,130);
    adminOrdersTable->setColumnWidth(4,110); adminOrdersTable->setColumnWidth(5,155);
    l->addWidget(adminOrdersTable,1);
    adminStack->addWidget(adminOrdersPage);
}

// ─── ADMIN ANALYTICS ─────────────────────────────────────────────
void MainWindow::buildAdminAnalyticsPage(){
    adminAnalyticsPage=new QWidget;
    adminAnalyticsPage->setStyleSheet(QString("background:%1;").arg(BG_MAIN));
    auto* l=new QVBoxLayout(adminAnalyticsPage);
    l->setContentsMargins(22,22,22,22); l->setSpacing(18);
    auto* sub=new QLabel("BUSINESS"); sub->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    l->addWidget(sub); l->addWidget(createTitleLabel("Analytics")); l->addWidget(createSeparator());
    auto* sr=new QHBoxLayout;
    auto mkCard=[&](const QString& em,const QString& lbl,QLabel*& out)->QWidget*{
        auto* c=new QWidget;
        c->setStyleSheet(QString("background:%1;border-radius:14px;border:1px solid #222;").arg(BG_CARD));
        addShadow(c,QColor(249,115,22,15));
        auto* cl=new QVBoxLayout(c); cl->setContentsMargins(22,22,22,22);
        auto* e=new QLabel(em); e->setFont(QFont("Segoe UI Emoji",26)); e->setAlignment(Qt::AlignCenter); e->setStyleSheet("background:transparent;");
        out=new QLabel("—"); out->setFont(QFont("Segoe UI",20,QFont::Bold));
        out->setStyleSheet(QString("color:%1;background:transparent;").arg(ORANGE)); out->setAlignment(Qt::AlignCenter);
        auto* lb=new QLabel(lbl); lb->setStyleSheet(QString("color:%1;font-size:10px;letter-spacing:1px;background:transparent;").arg(TEXT_DIM)); lb->setAlignment(Qt::AlignCenter);
        cl->addWidget(e); cl->addWidget(out); cl->addWidget(lb); return c;
    };
    sr->addWidget(mkCard("💰","TOTAL REVENUE",totalRevenueLabel));
    sr->addWidget(mkCard("📦","TOTAL ORDERS",totalOrdersLabel));
    l->addLayout(sr);
    auto* topT=new QLabel("TOP SELLERS"); topT->setStyleSheet(QString("color:%1;font-size:9px;letter-spacing:3px;font-weight:700;").arg(ORANGE));
    l->addWidget(topT);
    auto* sc=new QScrollArea; sc->setWidgetResizable(true); sc->setFrameShape(QFrame::NoFrame);
    topItemsContainer=new QWidget; topItemsContainer->setStyleSheet("background:transparent;");
    new QVBoxLayout(topItemsContainer); sc->setWidget(topItemsContainer);
    l->addWidget(sc,1);
    auto* rb=createSolidButton("↻  Refresh");
    connect(rb,&QPushButton::clicked,this,&MainWindow::refreshAnalytics);
    l->addWidget(rb,0,Qt::AlignRight);
    adminStack->addWidget(adminAnalyticsPage);
}

// ─── REFRESH METHODS ─────────────────────────────────────────────
void MainWindow::refreshMenu(){
    try{
        std::vector<MenuItem> items;
        QString srch=searchBar->text().trimmed();
        QString cat=categoryFilter->currentText();
        if(!srch.isEmpty()) items=foodService->searchMenu(srch.toStdString());
        else if(cat.isEmpty()||cat=="All Categories") items=foodService->getAllMenuItems();
        else items=foodService->getMenuByCategory(cat.toStdString());

        categoryFilter->blockSignals(true);
        QString cur=categoryFilter->currentText();
        categoryFilter->clear(); categoryFilter->addItem("All Categories");
        for(const auto& c:foodService->getCategories()) categoryFilter->addItem(QString::fromStdString(c));
        int idx=categoryFilter->findText(cur);
        if(idx>=0) categoryFilter->setCurrentIndex(idx);
        categoryFilter->blockSignals(false);

        auto* grid=qobject_cast<QGridLayout*>(menuItemsContainer->layout());
        if(!grid){grid=new QGridLayout(menuItemsContainer);grid->setSpacing(16);}
        while(grid->count()){auto* it=grid->takeAt(0);if(it->widget())it->widget()->deleteLater();delete it;}

        if(items.empty()){
            auto* emp=new QLabel("No items found"); emp->setStyleSheet(QString("color:%1;font-size:16px;").arg(TEXT_DIM));
            emp->setAlignment(Qt::AlignCenter); grid->addWidget(emp,0,0); return;
        }
        int cols=4;
        for(int i=0;i<(int)items.size();i++)
            grid->addWidget(createMenuItemCard(items[i],false),i/cols,i%cols);
        grid->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding),
            ((int)items.size()+cols-1)/cols,0,1,cols);
    } catch(const std::exception& e){showNotif("Error: "+QString(e.what()),false);}
}

void MainWindow::refreshCart(){
    auto* l=qobject_cast<QVBoxLayout*>(cartContainer->layout());
    if(!l){l=new QVBoxLayout(cartContainer);l->setSpacing(10);}
    while(l->count()){auto* it=l->takeAt(0);if(it->widget())it->widget()->deleteLater();delete it;}
    if(cartItems.empty()){
        auto* emp=new QLabel("🛒  Your cart is empty\n\nBrowse the menu to add items!");
        emp->setStyleSheet(QString("color:%1;font-size:15px;").arg(TEXT_DIM)); emp->setAlignment(Qt::AlignCenter);
        l->addWidget(emp); cartTotalLabel->setText("Total: PKR 50"); return;
    }
    double sub=0;
    for(const auto& oi:cartItems){
        auto* row=new QWidget;
        row->setStyleSheet(QString(R"(
            background:%1;border-radius:11px;border:1px solid #222;
        )").arg(BG_CARD)); row->setFixedHeight(76);
        auto* rl=new QHBoxLayout(row); rl->setContentsMargins(14,10,14,10);
        auto* img=new QLabel; img->setFixedSize(52,52);
        img->setStyleSheet("border-radius:8px;background:#1c1c1c;border:1px solid #2a2a2a;");
        img->setAlignment(Qt::AlignCenter);
        QPixmap px;
        if(!oi.getMenuItem().getImagePath().empty()) px.load(QString::fromStdString(oi.getMenuItem().getImagePath()));
        if(!px.isNull()) img->setPixmap(px.scaled(52,52,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation));
        else{img->setText("🍽️");img->setFont(QFont("Segoe UI Emoji",22));}
        auto* infoW=new QVBoxLayout;
        auto* nm=new QLabel(QString::fromStdString(oi.getMenuItem().getName()));
        nm->setFont(QFont("Segoe UI",12,QFont::Bold)); nm->setStyleSheet(QString("color:%1;").arg(TEXT_MAIN));
        auto* pr=new QLabel(QString("PKR %1  ×  %2").arg(oi.getMenuItem().getPrice(),0,'f',0).arg(oi.getQuantity()));
        pr->setStyleSheet(QString("color:%1;font-size:11px;").arg(TEXT_DIM));
        infoW->addWidget(nm); infoW->addWidget(pr);
        auto* stL=new QLabel(QString("PKR %1").arg(oi.getSubtotal(),0,'f',0));
        stL->setFont(QFont("Segoe UI",13,QFont::Bold)); stL->setStyleSheet(QString("color:%1;").arg(ORANGE));
        int id=oi.getMenuItem().getItemId();
        auto* rmBtn=new QPushButton("✕");
        rmBtn->setStyleSheet("QPushButton{background:rgba(239,68,68,0.1);color:#ef4444;border:1px solid rgba(239,68,68,0.25);border-radius:6px;padding:5px 9px;font-weight:bold;}QPushButton:hover{background:#ef4444;color:#fff;}");
        connect(rmBtn,&QPushButton::clicked,[this,id]{onRemoveFromCart(id);});
        rl->addWidget(img); rl->addLayout(infoW,1); rl->addWidget(stL); rl->addWidget(rmBtn);
        l->addWidget(row); sub+=oi.getSubtotal();
    }
    l->addStretch();
    cartTotalLabel->setText(QString("Total: PKR %1").arg(sub+50,0,'f',0));
    updateCartBadge();
}

void MainWindow::refreshOrders(){
    if(!currentCustomer) return;
    try{
        auto orders=foodService->getOrdersByCustomer(currentCustomer->getId());
        ordersTable->setRowCount(0);
        for(const auto& o:orders){
            int r=ordersTable->rowCount(); ordersTable->insertRow(r);
            ordersTable->setItem(r,0,new QTableWidgetItem("#"+QString::number(o.getOrderId())));
            ordersTable->setItem(r,1,new QTableWidgetItem(QString::fromStdString(o.getOrderTime())));
            QString st=QString::fromStdString(o.getStatusString());
            auto* si=new QTableWidgetItem(st);
            if(st=="Delivered") si->setForeground(QColor("#22c55e"));
            else if(st=="Cancelled") si->setForeground(QColor("#ef4444"));
            else if(st=="Preparing") si->setForeground(QColor("#f59e0b"));
            else si->setForeground(QColor(ORANGE));
            ordersTable->setItem(r,2,si);
            ordersTable->setItem(r,3,new QTableWidgetItem(QString("PKR %1").arg(o.getTotalAmount(),0,'f',0)));
            ordersTable->setItem(r,4,new QTableWidgetItem(QString::fromStdString(o.getPaymentString())));
            if(o.getStatus()==OrderStatus::PENDING||o.getStatus()==OrderStatus::CONFIRMED){
                int oid=o.getOrderId();
                auto* cb=new QPushButton("Cancel");
                cb->setStyleSheet("background:rgba(239,68,68,0.1);color:#ef4444;border:1px solid rgba(239,68,68,0.25);border-radius:6px;padding:4px 10px;");
                connect(cb,&QPushButton::clicked,[this,oid]{onCancelOrder(oid);});
                ordersTable->setCellWidget(r,5,cb);
            } else ordersTable->setItem(r,5,new QTableWidgetItem("—"));
            ordersTable->setRowHeight(r,44);
        }
    } catch(const std::exception& e){showNotif("Error: "+QString(e.what()),false);}
}

void MainWindow::refreshAdminOrders(){
    try{
        auto orders=foodService->getAllOrders();
        adminOrdersTable->setRowCount(0);
        for(const auto& o:orders){
            int r=adminOrdersTable->rowCount(); adminOrdersTable->insertRow(r);
            adminOrdersTable->setItem(r,0,new QTableWidgetItem("#"+QString::number(o.getOrderId())));
            adminOrdersTable->setItem(r,1,new QTableWidgetItem(QString::number(o.getCustomerId())));
            adminOrdersTable->setItem(r,2,new QTableWidgetItem(QString("PKR %1").arg(o.getTotalAmount(),0,'f',0)));
            adminOrdersTable->setItem(r,3,new QTableWidgetItem(QString::fromStdString(o.getPaymentString())));
            adminOrdersTable->setItem(r,4,new QTableWidgetItem(QString::fromStdString(o.getStatusString())));
            auto* combo=new QComboBox;
            combo->addItems({"Pending","Confirmed","Preparing","Ready for Delivery","Delivered","Cancelled"});
            combo->setCurrentText(QString::fromStdString(o.getStatusString()));
            combo->setStyleSheet(QString("background:#1a1a1a;color:%1;border:1px solid #2a2a2a;border-radius:6px;padding:4px;").arg(TEXT_MAIN));
            int oid=o.getOrderId();
            connect(combo,&QComboBox::currentTextChanged,[this,oid](const QString& s){onAdminUpdateStatus(oid,s);});
            adminOrdersTable->setCellWidget(r,5,combo);
            adminOrdersTable->setItem(r,6,new QTableWidgetItem(QString::fromStdString(o.getOrderTime())));
            adminOrdersTable->setRowHeight(r,44);
        }
    } catch(const std::exception& e){showNotif("Error: "+QString(e.what()),false);}
}

void MainWindow::refreshAnalytics(){
    try{
        totalRevenueLabel->setText(QString("PKR %1").arg(foodService->getTotalRevenue(),0,'f',0));
        totalOrdersLabel->setText(QString::number(foodService->getTotalOrders()));
        auto* tl=qobject_cast<QVBoxLayout*>(topItemsContainer->layout());
        if(!tl) tl=new QVBoxLayout(topItemsContainer);
        while(tl->count()){auto* it=tl->takeAt(0);if(it->widget())it->widget()->deleteLater();delete it;}
        auto top=foodService->getTopSellingItems(8);
        int mx=top.empty()?1:top[0].second; int rank=1;
        for(const auto& [nm,sold]:top){
            auto* rw=new QWidget;
            rw->setStyleSheet(QString("background:%1;border-radius:10px;border:1px solid #222;").arg(BG_CARD));
            rw->setFixedHeight(54);
            auto* rl=new QHBoxLayout(rw); rl->setContentsMargins(14,6,14,6);
            auto* rnk=new QLabel(QString("#%1").arg(rank++));
            rnk->setStyleSheet(QString("color:%1;font-weight:bold;font-size:14px;background:transparent;").arg(ORANGE)); rnk->setFixedWidth(36);
            auto* nml=new QLabel(QString::fromStdString(nm)); nml->setStyleSheet(QString("color:%1;font-size:12px;background:transparent;").arg(TEXT_MAIN));
            auto* bar=new QProgressBar; bar->setRange(0,mx); bar->setValue(sold); bar->setFixedHeight(6); bar->setTextVisible(false);
            auto* sl2=new QLabel(QString("%1 sold").arg(sold)); sl2->setStyleSheet(QString("color:%1;font-size:11px;background:transparent;").arg(TEXT_DIM)); sl2->setFixedWidth(65);
            rl->addWidget(rnk); rl->addWidget(nml); rl->addWidget(bar,1); rl->addWidget(sl2);
            tl->addWidget(rw);
        }
        if(top.empty()){auto* e=new QLabel("No data yet"); e->setStyleSheet(QString("color:%1;").arg(TEXT_DIM)); tl->addWidget(e);}
        tl->addStretch();
    } catch(const std::exception& e){showNotif("Analytics error: "+QString(e.what()),false);}
}

// ─── SLOTS ───────────────────────────────────────────────────────
void MainWindow::onLogin(){
    auto* eE=loginPage->findChild<QLineEdit*>("lEmail");
    auto* pE=loginPage->findChild<QLineEdit*>("lPass");
    auto* aB=loginPage->findChild<QPushButton*>("admRoleBtn");
    if(!eE||!pE) return;
    if(eE->text().isEmpty()||pE->text().isEmpty()){showNotif("Please fill all fields!",false);return;}
    bool isAdmin=aB&&aB->isChecked();
    try{
        if(isAdmin){
            delete currentAdmin;
            currentAdmin=foodService->loginAdmin(eE->text().toStdString(),pE->text().toStdString());
            FileManager::logActivity("Admin login",currentAdmin->getName());
            mainStack->setCurrentIndex(3); refreshAdminOrders();
        } else {
            delete currentCustomer;
            currentCustomer=foodService->loginCustomer(eE->text().toStdString(),pE->text().toStdString());
            cartItems.clear();
            welcomeLabel->setText("Hi, "+QString::fromStdString(currentCustomer->getName())+"!");
            loyaltyPointsLabel->setText(QString("⭐ %1 pts").arg(currentCustomer->getLoyaltyPoints(),0,'f',0));
            if(auto* n=profilePage->findChild<QLabel*>("pName")) n->setText(QString::fromStdString(currentCustomer->getName()));
            if(auto* e=profilePage->findChild<QLabel*>("pEmail")) e->setText(QString::fromStdString(currentCustomer->getEmail()));
            if(auto* p=profilePage->findChild<QLabel*>("pPhone")) p->setText(QString::fromStdString(currentCustomer->getPhone()));
            if(auto* a=profilePage->findChild<QLabel*>("pAddress")) a->setText(QString::fromStdString(currentCustomer->getAddress()));
            if(auto* pt=profilePage->findChild<QLabel*>("pPoints")) pt->setText(QString("%1").arg(currentCustomer->getLoyaltyPoints(),0,'f',0));
            FileManager::logActivity("Customer login",currentCustomer->getName());
            mainStack->setCurrentIndex(2); customerStack->setCurrentIndex(0);
            refreshHomePage();
        }
        pE->clear(); showNotif("✓ Welcome back!");
    } catch(const std::exception& e){showNotif(QString("Login failed: ")+e.what(),false);}
}

void MainWindow::onRegister(){
    auto f=[&](const QString& n){return registerPage->findChild<QLineEdit*>(n);};
    if(f("rName")->text().isEmpty()||f("rEmail")->text().isEmpty()){showNotif("Please fill all fields!",false);return;}
    if(f("rPass")->text()!=f("rConf")->text()){showNotif("Passwords do not match!",false);return;}
    try{
        foodService->registerCustomer(f("rName")->text().toStdString(),f("rEmail")->text().toStdString(),
            f("rPhone")->text().toStdString(),f("rAddr")->text().toStdString(),f("rPass")->text().toStdString());
        showNotif("✓ Account created! Sign in now.");
        FileManager::logActivity("Registration",f("rName")->text().toStdString());
        mainStack->setCurrentIndex(0);
        for(auto* e:registerPage->findChildren<QLineEdit*>()) e->clear();
    } catch(const std::exception& e){showNotif(QString("Failed: ")+e.what(),false);}
}

void MainWindow::onLogout(){
    delete currentCustomer; currentCustomer=nullptr;
    delete currentAdmin; currentAdmin=nullptr;
    cartItems.clear(); mainStack->setCurrentIndex(0);
    showNotif("Signed out. See you soon!");
}

void MainWindow::onSearch(){refreshMenu();}
void MainWindow::onCategoryFilter(const QString&){refreshMenu();}

void MainWindow::onAddToCart(int itemId){
    try{
        MenuItem item=foodService->getMenuItemById(itemId);
        bool found=false;
        for(auto& oi:cartItems){if(oi.getMenuItem().getItemId()==itemId){oi.setQuantity(oi.getQuantity()+1);found=true;break;}}
        if(!found){OrderItem oi(item,1);cartItems.push_back(oi);}
        updateCartBadge();
        showNotif("✓ "+QString::fromStdString(item.getName())+" added to cart!");
    } catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onRemoveFromCart(int itemId){
    cartItems.erase(std::remove_if(cartItems.begin(),cartItems.end(),[&](const OrderItem& oi){
        return oi.getMenuItem().getItemId()==itemId;}),cartItems.end());
    refreshCart();
}

void MainWindow::onPlaceOrder(){
    if(!currentCustomer||cartItems.empty()){showNotif("Cart is empty!",false);return;}
    PaymentMethod pm=PaymentMethod::CASH_ON_DELIVERY;
    QString ps=paymentCombo->currentText();
    if(ps=="Credit Card") pm=PaymentMethod::CREDIT_CARD;
    else if(ps=="Debit Card") pm=PaymentMethod::DEBIT_CARD;
    else if(ps=="Mobile Wallet") pm=PaymentMethod::MOBILE_WALLET;
    Order order(0,currentCustomer->getId(),currentCustomer->getAddress(),pm);
    order.setDeliveryFee(50); order.setNotes(notesEdit->toPlainText().toStdString());
    for(const auto& item:cartItems) order.addItem(item);
    try{
        int oid=foodService->placeOrder(order);
        Order placed=foodService->getOrderById(oid);
        FileManager::saveReceipt(placed,currentCustomer->getName());
        FileManager::logActivity("Order #"+std::to_string(oid),currentCustomer->getName());
        cartItems.clear(); notesEdit->clear(); updateCartBadge(); refreshCart();
        showNotif(QString("✓ Order #%1 placed! ETA: 30-45 mins").arg(oid));
        customerStack->setCurrentIndex(3); refreshOrders();
    } catch(const std::exception& e){showNotif(QString("Order failed: ")+e.what(),false);}
}

void MainWindow::onCancelOrder(int orderId){
    if(!currentCustomer) return;
    try{
        foodService->cancelOrder(orderId,currentCustomer->getId());
        showNotif("Order #"+QString::number(orderId)+" cancelled."); refreshOrders();
    } catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onAdminUpdateStatus(int orderId,const QString& status){
    OrderStatus os=OrderStatus::PENDING;
    if(status=="Confirmed") os=OrderStatus::CONFIRMED;
    else if(status=="Preparing") os=OrderStatus::PREPARING;
    else if(status=="Ready for Delivery") os=OrderStatus::READY;
    else if(status=="Delivered") os=OrderStatus::DELIVERED;
    else if(status=="Cancelled") os=OrderStatus::CANCELLED;
    try{foodService->updateOrderStatus(orderId,os);}
    catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onAddMenuItem(){
    if(itemNameEdit->text().isEmpty()||itemPriceEdit->text().isEmpty()){showNotif("Name & price required!",false);return;}
    MenuItem item(0,itemNameEdit->text().toStdString(),itemDescEdit->text().toStdString(),
        itemPriceEdit->text().toDouble(),itemCategoryEdit->text().toStdString(),
        itemImageEdit->text().toStdString(),true,itemPrepEdit->text().isEmpty()?15:itemPrepEdit->text().toInt());
    try{
        foodService->addMenuItem(item); showNotif("✓ Item added!");
        itemNameEdit->clear();itemDescEdit->clear();itemPriceEdit->clear();
        itemCategoryEdit->clear();itemPrepEdit->clear();itemImageEdit->clear();
        refreshAdminMenuTable(); refreshMenu(); refreshHomePage();
    } catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onUpdateMenuItem(){
    if(editingItemId<0){showNotif("Select an item to edit!",false);return;}
    MenuItem item(editingItemId,itemNameEdit->text().toStdString(),itemDescEdit->text().toStdString(),
        itemPriceEdit->text().toDouble(),itemCategoryEdit->text().toStdString(),
        itemImageEdit->text().toStdString(),true,itemPrepEdit->text().isEmpty()?15:itemPrepEdit->text().toInt());
    try{
        foodService->updateMenuItem(item); showNotif("✓ Item updated!"); editingItemId=-1;
        refreshAdminMenuTable(); refreshMenu(); refreshHomePage();
    } catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onDeleteMenuItem(int itemId){
    if(QMessageBox::question(this,"Delete","Delete this item?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes){
        try{foodService->deleteMenuItem(itemId);showNotif("Item deleted.");refreshAdminMenuTable();refreshMenu();}
        catch(const std::exception& e){showNotif(QString(e.what()),false);}
    }
}

void MainWindow::onToggleAvail(int itemId){
    try{foodService->toggleMenuItemAvailability(itemId);showNotif("Availability toggled.");refreshAdminMenuTable();}
    catch(const std::exception& e){showNotif(QString(e.what()),false);}
}

void MainWindow::onBrowseImage(){
    QString p=QFileDialog::getOpenFileName(this,"Select Food Image","","Images (*.png *.jpg *.jpeg *.bmp)");
    if(!p.isEmpty()) itemImageEdit->setText(p);
}

void MainWindow::onExportMenu(){
    try{FileManager::exportMenuToCSV(foodService->getAllMenuItems(),"menu_export.csv");showNotif("✓ Menu exported!");}
    catch(...){showNotif("Export failed!",false);}
}

void MainWindow::onExportOrders(){
    try{FileManager::exportOrdersToCSV(foodService->getAllOrders(),"orders_export.csv");showNotif("✓ Orders exported!");}
    catch(...){showNotif("Export failed!",false);}
}
