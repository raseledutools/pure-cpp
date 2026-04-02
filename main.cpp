#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QSpinBox>
#include <QTimer>
#include <QLineEdit>

// ==========================================
// গ্লোবাল ভ্যারিয়েবল ও ডাটা
// ==========================================
int remainingSeconds = 0;
bool isBreakActive = false;

// ==========================================
// MAIN APPLICATION CLASS
// ==========================================
class RasFocusPro : public QMainWindow {
    QStackedWidget *stackedWidget;
    
    // Pages
    QWidget *homePage;
    QWidget *takeBreakPage;
    QWidget *activeSessionPage;
    QWidget *overlayPage; // কোরআন/হাদিসের পেজ
    
    // UI Elements
    QLabel *lblTimerDisplay;
    QTimer *countdownTimer;

public:
    RasFocusPro() {
        setWindowTitle("RasFocus+ Ultimate");
        resize(420, 850); // মোবাইল ভিউ

        // পুরো অ্যাপের গ্লোবাল স্টাইল (বড় ফন্ট ও মডার্ন লুক)
        setStyleSheet(R"(
            QMainWindow { background-color: #f4f6f9; }
            QLabel { font-family: 'Segoe UI', sans-serif; color: #2c3e50; }
            QPushButton { font-family: 'Segoe UI', sans-serif; font-weight: bold; border-radius: 15px; }
            QLineEdit { font-size: 18px; padding: 15px; border-radius: 10px; border: 2px solid #bdc3c7; }
            QSpinBox { font-size: 24px; padding: 10px; }
        )");

        // মেইন কন্টেইনার
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Stacked Widget (পেজ পাল্টানোর জন্য)
        stackedWidget = new QStackedWidget(this);
        
        setupHomePage();
        setupTakeBreakPage();
        setupActiveSessionPage();
        setupOverlayPage();

        stackedWidget->addWidget(homePage);          // Index 0
        stackedWidget->addWidget(takeBreakPage);     // Index 1
        stackedWidget->addWidget(activeSessionPage); // Index 2
        stackedWidget->addWidget(overlayPage);       // Index 3
        
        mainLayout->addWidget(stackedWidget);

        // --- Bottom Navigation Bar ---
        QWidget *bottomNav = new QWidget();
        bottomNav->setStyleSheet("background-color: white; border-top: 1px solid #e0e0e0; padding: 10px;");
        QHBoxLayout *navLayout = new QHBoxLayout(bottomNav);
        
        QPushButton *btnWhite = new QPushButton("✅ Whitelist");
        QPushButton *btnBlack = new QPushButton("🚫 Blacklist");
        QPushButton *btnAppCtrl = new QPushButton("⏱️ App Control");
        
        // ন্যাভিগেশন বাটন স্টাইল
        QString navStyle = "background-color: #ecf0f1; color: #34495e; font-size: 14px; padding: 15px; border-radius: 10px;";
        btnWhite->setStyleSheet(navStyle);
        btnBlack->setStyleSheet(navStyle);
        btnAppCtrl->setStyleSheet(navStyle);
        
        navLayout->addWidget(btnWhite);
        navLayout->addWidget(btnBlack);
        navLayout->addWidget(btnAppCtrl);
        
        mainLayout->addWidget(bottomNav);

        // টাইমার সেটআপ
        countdownTimer = new QTimer(this);
        connect(countdownTimer, &QTimer::timeout, this, &RasFocusPro::updateTimer);
    }

private:
    // ==========================================
    // ১. Home Page
    // ==========================================
    void setupHomePage() {
        homePage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(homePage);
        layout->setContentsMargins(20, 40, 20, 20);

        QLabel *title = new QLabel("RasFocus+");
        title->setStyleSheet("font-size: 36px; font-weight: bold; color: #2980b9;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        layout->addStretch();

        // পাসওয়ার্ড ফিল্ড (সিকিউরিটির জন্য)
        QLineEdit *txtPassword = new QLineEdit();
        txtPassword->setPlaceholderText("Enter Security PIN");
        txtPassword->setEchoMode(QLineEdit::Password);
        layout->addWidget(txtPassword);

        // Take a Break Button
        QPushButton *btnTakeBreak = new QPushButton("☕ Take a Break");
        btnTakeBreak->setStyleSheet("background-color: #8e44ad; color: white; font-size: 24px; padding: 25px; margin-top: 20px;");
        connect(btnTakeBreak, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(1); });
        layout->addWidget(btnTakeBreak);

        // Start Auto Protection
        QPushButton *btnAutoProtect = new QPushButton("🛡️ Start Auto Protection");
        btnAutoProtect->setStyleSheet("background-color: #27ae60; color: white; font-size: 20px; padding: 20px; margin-top: 10px;");
        layout->addWidget(btnAutoProtect);

        layout->addStretch();
    }

    // ==========================================
    // ২. Take a Break Page (টাইম সেটআপ)
    // ==========================================
    void setupTakeBreakPage() {
        takeBreakPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(takeBreakPage);
        layout->setContentsMargins(20, 40, 20, 20);

        QLabel *title = new QLabel("Set Focus Duration");
        title->setStyleSheet("font-size: 28px; font-weight: bold;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        // Day, Hour, Min Layout
        QHBoxLayout *timeLayout = new QHBoxLayout();
        
        QVBoxLayout *dLayout = new QVBoxLayout();
        QSpinBox *spinDay = new QSpinBox(); spinDay->setMaximum(30);
        QLabel *lblD = new QLabel("Days"); lblD->setAlignment(Qt::AlignCenter);
        dLayout->addWidget(spinDay); dLayout->addWidget(lblD);
        
        QVBoxLayout *hLayout = new QVBoxLayout();
        QSpinBox *spinHour = new QSpinBox(); spinHour->setMaximum(23);
        QLabel *lblH = new QLabel("Hours"); lblH->setAlignment(Qt::AlignCenter);
        hLayout->addWidget(spinHour); hLayout->addWidget(lblH);

        QVBoxLayout *mLayout = new QVBoxLayout();
        QSpinBox *spinMin = new QSpinBox(); spinMin->setMaximum(59); spinMin->setValue(25); // Default 25 min
        QLabel *lblM = new QLabel("Mins"); lblM->setAlignment(Qt::AlignCenter);
        mLayout->addWidget(spinMin); mLayout->addWidget(lblM);

        timeLayout->addLayout(dLayout);
        timeLayout->addLayout(hLayout);
        timeLayout->addLayout(mLayout);
        layout->addLayout(timeLayout);

        // Whitelist Selection Button
        QPushButton *btnSelectApps = new QPushButton("📋 Select Whitelisted Apps");
        btnSelectApps->setStyleSheet("background-color: #f39c12; color: white; font-size: 18px; padding: 20px; margin-top: 20px;");
        layout->addWidget(btnSelectApps);

        layout->addStretch();

        // Start Session Button
        QPushButton *btnStartSession = new QPushButton("🚀 Start Hardcore Session");
        btnStartSession->setStyleSheet("background-color: #c0392b; color: white; font-size: 22px; padding: 25px;");
        connect(btnStartSession, &QPushButton::clicked, [=]() {
            // Calculate total seconds
            remainingSeconds = (spinDay->value() * 86400) + (spinHour->value() * 3600) + (spinMin->value() * 60);
            if(remainingSeconds > 0) {
                isBreakActive = true;
                countdownTimer->start(1000);
                stackedWidget->setCurrentIndex(2); // Go to Active Session Page
            }
        });
        layout->addWidget(btnStartSession);

        // Back Button
        QPushButton *btnBack = new QPushButton("Back");
        btnBack->setStyleSheet("background-color: transparent; color: #7f8c8d; font-size: 18px; margin-top: 10px;");
        connect(btnBack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnBack);
    }

    // ==========================================
    // ৩. Active Session Page (Spinning Timer)
    // ==========================================
    void setupActiveSessionPage() {
        activeSessionPage = new QWidget();
        activeSessionPage->setStyleSheet("background-color: #2c3e50;"); // গাঢ় ব্যাকগ্রাউন্ড
        QVBoxLayout *layout = new QVBoxLayout(activeSessionPage);
        layout->setContentsMargins(20, 60, 20, 40);

        QLabel *title = new QLabel("Stay Focused!");
        title->setStyleSheet("font-size: 32px; font-weight: bold; color: white;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        layout->addStretch();

        // Timer Display (বিশাল সাইজের ফন্ট)
        lblTimerDisplay = new QLabel("00:00:00");
        lblTimerDisplay->setStyleSheet(R"(
            font-size: 65px; 
            font-weight: bold; 
            color: #2ecc71; 
            background-color: #34495e; 
            border-radius: 20px; 
            padding: 40px;
        )");
        lblTimerDisplay->setAlignment(Qt::AlignCenter);
        layout->addWidget(lblTimerDisplay);

        layout->addStretch();

        // Allowed Apps Icon Area
        QLabel *lblAllowed = new QLabel("Allowed Apps");
        lblAllowed->setStyleSheet("font-size: 18px; color: #bdc3c7;");
        lblAllowed->setAlignment(Qt::AlignCenter);
        layout->addWidget(lblAllowed);

        QPushButton *btnOpenAllowed = new QPushButton("📱 Open Whitelisted App");
        btnOpenAllowed->setStyleSheet("background-color: #3498db; color: white; font-size: 20px; padding: 20px; border-radius: 15px;");
        layout->addWidget(btnOpenAllowed);
    }

    // ==========================================
    // ৪. Block Overlay Page (কোরআন/হাদিস)
    // ==========================================
    void setupOverlayPage() {
        overlayPage = new QWidget();
        overlayPage->setStyleSheet("background-color: #c0392b;"); // লাল অ্যালার্ট ব্যাকগ্রাউন্ড
        QVBoxLayout *layout = new QVBoxLayout(overlayPage);
        layout->setContentsMargins(20, 40, 20, 20);

        QLabel *alertIcon = new QLabel("🛑");
        alertIcon->setStyleSheet("font-size: 80px;");
        alertIcon->setAlignment(Qt::AlignCenter);
        layout->addWidget(alertIcon);

        QLabel *warning = new QLabel("Access Blocked!");
        warning->setStyleSheet("font-size: 36px; font-weight: bold; color: white;");
        warning->setAlignment(Qt::AlignCenter);
        layout->addWidget(warning);

        layout->addStretch();

        // ইসলামিক বাণী (বড় ফন্টে)
        QLabel *quote = new QLabel("“হে মুমিনগণ! তোমরা নিজেদেরকে এবং তোমাদের পরিবার-পরিজনকে রক্ষা কর অগ্নি হতে...”\n\n- সূরা আত-তাহরীম: ৬");
        quote->setStyleSheet("font-size: 26px; color: white; font-weight: bold;");
        quote->setAlignment(Qt::AlignCenter);
        quote->setWordWrap(true);
        layout->addWidget(quote);

        layout->addStretch();

        // Back to work button
        QPushButton *btnReturn = new QPushButton("Return to Focus");
        btnReturn->setStyleSheet("background-color: white; color: #c0392b; font-size: 22px; padding: 20px;");
        connect(btnReturn, &QPushButton::clicked, [=]() { 
            if(isBreakActive) stackedWidget->setCurrentIndex(2); 
            else stackedWidget->setCurrentIndex(0);
        });
        layout->addWidget(btnReturn);
    }

    // ==========================================
    // Timer Logic
    // ==========================================
    void updateTimer() {
        if (remainingSeconds > 0) {
            remainingSeconds--;
            int h = remainingSeconds / 3600;
            int m = (remainingSeconds % 3600) / 60;
            int s = remainingSeconds % 60;
            lblTimerDisplay->setText(QString("%1:%2:%3")
                                     .arg(h, 2, 10, QChar('0'))
                                     .arg(m, 2, 10, QChar('0'))
                                     .arg(s, 2, 10, QChar('0')));
        } else {
            countdownTimer->stop();
            isBreakActive = false;
            lblTimerDisplay->setText("00:00:00");
            stackedWidget->setCurrentIndex(0); // Return to home
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    RasFocusPro window;
    window.show();
    return app.exec();
}
