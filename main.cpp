#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QSpinBox>
#include <QTimer>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QCoreApplication>
#include <QFrame>

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroid>

// অটোমেটিক পারমিশন চাওয়ার ফাংশন
void requestPermissions() {
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject action = QAndroidJniObject::fromString("android.settings.ACCESSIBILITY_SETTINGS");
    QAndroidJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", action.object<jstring>());
    activity.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object<jobject>());
}
#endif

// গ্লোবাল ভ্যারিয়েবল
int remainingSeconds = 0;
bool isBreakActive = false;

class RasFocusPro : public QMainWindow {
    QStackedWidget *stackedWidget;
    
    QWidget *homePage;
    QWidget *takeBreakPage;
    QWidget *activeSessionPage;
    QWidget *overlayPage;
    QWidget *whitelistPage;
    QWidget *blacklistPage;
    QWidget *appControlPage;
    
    QLabel *lblTimerDisplay;
    QTimer *countdownTimer;

public:
    RasFocusPro() {
        setWindowTitle("RasFocus+ Ultimate");
        
        // ফুল ডার্ক মডার্ন থিম এবং বিশাল ফন্ট সাইজ
        setStyleSheet(R"(
            QMainWindow { background-color: #121212; }
            QLabel { color: #ffffff; font-family: 'Segoe UI', sans-serif; }
            QPushButton { font-weight: bold; border-radius: 20px; }
            QLineEdit { font-size: 40px; padding: 30px; border-radius: 15px; border: 2px solid #333; background-color: #1e1e1e; color: white; }
            QSpinBox { font-size: 60px; padding: 20px; background-color: #1e1e1e; color: #00e676; border: 2px solid #333; border-radius: 15px; }
            QListWidget { font-size: 40px; padding: 20px; border-radius: 15px; background-color: #1e1e1e; color: white; border: none; }
            QListWidget::item { padding: 30px; border-bottom: 2px solid #333; }
        )");

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        stackedWidget = new QStackedWidget(this);
        
        setupHomePage();
        setupTakeBreakPage();
        setupActiveSessionPage();
        setupOverlayPage();
        setupWhitelistPage();
        setupBlacklistPage();
        setupAppControlPage();

        stackedWidget->addWidget(homePage);          // Index 0
        stackedWidget->addWidget(takeBreakPage);     // Index 1
        stackedWidget->addWidget(activeSessionPage); // Index 2
        stackedWidget->addWidget(overlayPage);       // Index 3
        stackedWidget->addWidget(whitelistPage);     // Index 4
        stackedWidget->addWidget(blacklistPage);     // Index 5
        stackedWidget->addWidget(appControlPage);    // Index 6
        
        mainLayout->addWidget(stackedWidget);

        // --- Bottom Navigation Bar (বিশাল বাটন) ---
        QWidget *bottomNav = new QWidget();
        bottomNav->setStyleSheet("background-color: #1e1e1e; border-top: 2px solid #333; padding: 20px;");
        QHBoxLayout *navLayout = new QHBoxLayout(bottomNav);
        
        QPushButton *btnWhite = new QPushButton("✅ WL");
        QPushButton *btnBlack = new QPushButton("🚫 BL");
        QPushButton *btnAppCtrl = new QPushButton("⏱️ CTRL");
        
        QString navStyle = "background-color: #2c3e50; color: white; font-size: 35px; padding: 40px; border-radius: 20px;";
        btnWhite->setStyleSheet(navStyle); btnBlack->setStyleSheet(navStyle); btnAppCtrl->setStyleSheet(navStyle);
        
        connect(btnWhite, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(4); });
        connect(btnBlack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(5); });
        connect(btnAppCtrl, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(6); });

        navLayout->addWidget(btnWhite); navLayout->addWidget(btnBlack); navLayout->addWidget(btnAppCtrl);
        mainLayout->addWidget(bottomNav);

        countdownTimer = new QTimer(this);
        connect(countdownTimer, &QTimer::timeout, this, &RasFocusPro::updateTimer);

        // অ্যাপ ওপেন হওয়ার ১ সেকেন্ড পর পারমিশন চাইবে
        #ifdef Q_OS_ANDROID
        QTimer::singleShot(1000, [](){ requestPermissions(); });
        #endif
    }

private:
    void setupHomePage() {
        homePage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(homePage);
        layout->setContentsMargins(40, 60, 40, 40);

        QLabel *title = new QLabel("RASFOCUS+");
        title->setStyleSheet("font-size: 90px; font-weight: bold; color: #00e676; letter-spacing: 5px;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        layout->addStretch();

        QLineEdit *txtPassword = new QLineEdit();
        txtPassword->setPlaceholderText("Enter PIN");
        txtPassword->setEchoMode(QLineEdit::Password);
        layout->addWidget(txtPassword);

        QPushButton *btnTakeBreak = new QPushButton("☕ TAKE A BREAK");
        btnTakeBreak->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #8e44ad, stop:1 #9b59b6); color: white; font-size: 50px; padding: 60px; margin-top: 40px;");
        connect(btnTakeBreak, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(1); });
        layout->addWidget(btnTakeBreak);

        QPushButton *btnAutoProtect = new QPushButton("🛡️ ENABLE PROTECTION");
        btnAutoProtect->setStyleSheet("background-color: #27ae60; color: white; font-size: 40px; padding: 50px; margin-top: 20px;");
        connect(btnAutoProtect, &QPushButton::clicked, [=]() {
            #ifdef Q_OS_ANDROID
            requestPermissions();
            #endif
        });
        layout->addWidget(btnAutoProtect);
        layout->addStretch();
    }

    void setupTakeBreakPage() {
        takeBreakPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(takeBreakPage);
        layout->setContentsMargins(40, 60, 40, 40);

        QLabel *title = new QLabel("SET DURATION");
        title->setStyleSheet("font-size: 70px; font-weight: bold; color: #00e676;");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        // মডার্ন স্ক্রলিং স্টাইল স্পিনবক্স
        QHBoxLayout *timeLayout = new QHBoxLayout();
        QVBoxLayout *dLayout = new QVBoxLayout(); QSpinBox *spinDay = new QSpinBox(); spinDay->setMaximum(30); QLabel *lblD = new QLabel("Days"); lblD->setStyleSheet("font-size: 35px; color:#aaa;"); lblD->setAlignment(Qt::AlignCenter); dLayout->addWidget(spinDay); dLayout->addWidget(lblD);
        QVBoxLayout *hLayout = new QVBoxLayout(); QSpinBox *spinHour = new QSpinBox(); spinHour->setMaximum(23); QLabel *lblH = new QLabel("Hours"); lblH->setStyleSheet("font-size: 35px; color:#aaa;"); lblH->setAlignment(Qt::AlignCenter); hLayout->addWidget(spinHour); hLayout->addWidget(lblH);
        QVBoxLayout *mLayout = new QVBoxLayout(); QSpinBox *spinMin = new QSpinBox(); spinMin->setMaximum(59); spinMin->setValue(25); QLabel *lblM = new QLabel("Mins"); lblM->setStyleSheet("font-size: 35px; color:#aaa;"); lblM->setAlignment(Qt::AlignCenter); mLayout->addWidget(spinMin); mLayout->addWidget(lblM);
        timeLayout->addLayout(dLayout); timeLayout->addLayout(hLayout); timeLayout->addLayout(mLayout);
        layout->addLayout(timeLayout);

        QPushButton *btnStartSession = new QPushButton("🚀 START HARDCORE SESSION");
        btnStartSession->setStyleSheet("background-color: #e74c3c; color: white; font-size: 50px; padding: 60px; margin-top: 50px;");
        connect(btnStartSession, &QPushButton::clicked, [=]() {
            remainingSeconds = (spinDay->value() * 86400) + (spinHour->value() * 3600) + (spinMin->value() * 60);
            if(remainingSeconds > 0) { isBreakActive = true; countdownTimer->start(1000); stackedWidget->setCurrentIndex(2); }
        });
        layout->addWidget(btnStartSession);

        QPushButton *btnBack = new QPushButton("Back to Home");
        btnBack->setStyleSheet("background-color: transparent; color: #7f8c8d; font-size: 40px; margin-top: 30px;");
        connect(btnBack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnBack);
        layout->addStretch();
    }

    void setupActiveSessionPage() {
        activeSessionPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(activeSessionPage);
        layout->setContentsMargins(40, 80, 40, 60);

        QLabel *title = new QLabel("STAY FOCUSED!"); title->setStyleSheet("font-size: 80px; font-weight: bold; color: #e74c3c;"); title->setAlignment(Qt::AlignCenter); layout->addWidget(title);
        layout->addStretch();

        lblTimerDisplay = new QLabel("00:00:00");
        lblTimerDisplay->setStyleSheet("font-size: 150px; font-weight: bold; color: #00e676; background-color: #1e1e1e; border-radius: 40px; padding: 60px; border: 3px solid #333;");
        lblTimerDisplay->setAlignment(Qt::AlignCenter);
        layout->addWidget(lblTimerDisplay);
        layout->addStretch();

        QPushButton *btnOpenAllowed = new QPushButton("📱 Whitelisted Apps");
        btnOpenAllowed->setStyleSheet("background-color: #3498db; color: white; font-size: 50px; padding: 50px;");
        layout->addWidget(btnOpenAllowed);
    }

    void setupOverlayPage() {
        overlayPage = new QWidget();
        overlayPage->setStyleSheet("background-color: #c0392b;");
        QVBoxLayout *layout = new QVBoxLayout(overlayPage);
        
        QLabel *warning = new QLabel("🛑\nBLOCKED!");
        warning->setStyleSheet("font-size: 100px; font-weight: bold; color: white;"); warning->setAlignment(Qt::AlignCenter); layout->addWidget(warning);
        layout->addStretch();

        QLabel *quote = new QLabel("“হে মুমিনগণ! তোমরা নিজেদেরকে এবং তোমাদের পরিবার-পরিজনকে রক্ষা কর অগ্নি হতে...”\n\n- সূরা আত-তাহরীম: ৬");
        quote->setStyleSheet("font-size: 55px; color: white; font-weight: bold;"); quote->setAlignment(Qt::AlignCenter); quote->setWordWrap(true); layout->addWidget(quote);
        layout->addStretch();

        QPushButton *btnReturn = new QPushButton("Return to Focus");
        btnReturn->setStyleSheet("background-color: white; color: #c0392b; font-size: 50px; padding: 50px;");
        connect(btnReturn, &QPushButton::clicked, [=]() { if(isBreakActive) stackedWidget->setCurrentIndex(2); else stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnReturn);
    }

    void setupWhitelistPage() {
        whitelistPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(whitelistPage);
        layout->setContentsMargins(40, 60, 40, 40);

        QLabel *title = new QLabel("✅ WHITELIST");
        title->setStyleSheet("font-size: 60px; font-weight: bold; color: #00e676;"); layout->addWidget(title);

        QListWidget *listWidget = new QListWidget();
        QStringList dummyApps = {"Calculator", "Dictionary", "Notes"};
        for(const QString &app : dummyApps) {
            QListWidgetItem *item = new QListWidgetItem(app, listWidget);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); item->setCheckState(Qt::Checked);
        }
        layout->addWidget(listWidget);

        QPushButton *btnBack = new QPushButton("SAVE");
        btnBack->setStyleSheet("background-color: #2980b9; color: white; font-size: 45px; padding: 40px;");
        connect(btnBack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnBack);
    }

    void setupBlacklistPage() {
        blacklistPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(blacklistPage);
        layout->setContentsMargins(40, 60, 40, 40);

        QLabel *title = new QLabel("🚫 BLACKLIST");
        title->setStyleSheet("font-size: 60px; font-weight: bold; color: #e74c3c;"); layout->addWidget(title);

        QListWidget *listWidget = new QListWidget();
        QStringList dummyApps = {"Facebook", "Instagram", "Chrome"};
        for(const QString &app : dummyApps) {
            QListWidgetItem *item = new QListWidgetItem(app, listWidget);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); item->setCheckState(Qt::Checked);
        }
        layout->addWidget(listWidget);

        QPushButton *btnBack = new QPushButton("SAVE");
        btnBack->setStyleSheet("background-color: #c0392b; color: white; font-size: 45px; padding: 40px;");
        connect(btnBack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnBack);
    }

    void setupAppControlPage() {
        appControlPage = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(appControlPage);
        layout->setContentsMargins(40, 60, 40, 40);

        QLabel *title = new QLabel("⏱️ APP LIMITS");
        title->setStyleSheet("font-size: 60px; font-weight: bold; color: #f1c40f;"); layout->addWidget(title);

        QScrollArea *scrollArea = new QScrollArea();
        QWidget *scrollWidget = new QWidget();
        QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
        scrollLayout->setSpacing(20);
        
        QStringList controlApps = {"YouTube", "WhatsApp"};
        for(const QString &app : controlApps) {
            QFrame *frame = new QFrame();
            frame->setStyleSheet("background-color: #1e1e1e; border-radius: 20px; padding: 30px; border: 2px solid #333;");
            QHBoxLayout *fLayout = new QHBoxLayout(frame);
            QLabel *lblApp = new QLabel(app); lblApp->setStyleSheet("font-size: 45px; font-weight: bold;");
            QSpinBox *spinH = new QSpinBox(); spinH->setSuffix("h"); spinH->setMaximum(23);
            QSpinBox *spinM = new QSpinBox(); spinM->setSuffix("m"); spinM->setMaximum(59); spinM->setValue(30);
            fLayout->addWidget(lblApp); fLayout->addStretch(); fLayout->addWidget(spinH); fLayout->addWidget(spinM);
            scrollLayout->addWidget(frame);
        }
        scrollLayout->addStretch();
        scrollArea->setWidget(scrollWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("border: none; background: transparent;");
        layout->addWidget(scrollArea);

        QPushButton *btnBack = new QPushButton("SAVE");
        btnBack->setStyleSheet("background-color: #f39c12; color: white; font-size: 45px; padding: 40px;");
        connect(btnBack, &QPushButton::clicked, [=]() { stackedWidget->setCurrentIndex(0); });
        layout->addWidget(btnBack);
    }

    void updateTimer() {
        if (remainingSeconds > 0) {
            remainingSeconds--;
            int h = remainingSeconds / 3600;
            int m = (remainingSeconds % 3600) / 60;
            int s = remainingSeconds % 60;
            lblTimerDisplay->setText(QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
        } else {
            countdownTimer->stop();
            isBreakActive = false;
            lblTimerDisplay->setText("00:00:00");
            stackedWidget->setCurrentIndex(0);
        }
    }
};

int main(int argc, char *argv[]) {
    // মোবাইল হাই-রেজুলেশন ফিক্স (লেখা এবং বাটনগুলো মোবাইলের স্ক্রিন অনুযায়ী বড় হবে)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    RasFocusPro window;
    window.showMaximized(); 
    return app.exec();
}
