#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QScrollArea>
#include <QTimer>

// --- OS Specific Headers ---
#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#endif

// গ্লোবাল ভ্যারিয়েবল
bool isProtectionActive = false;
int focusTimeSeconds = 0;

// ==========================================
// ১. ছোট ইনফো কার্ড (Screen Time, App Launches)
// ==========================================
class InfoCard : public QFrame {
    QLabel *lblValue;
public:
    InfoCard(QString iconStr, QString title, QString value, QString stat, QWidget *parent = nullptr) : QFrame(parent) {
        setStyleSheet(R"(
            QFrame { background-color: white; border-radius: 15px; padding: 15px; border: 1px solid #f0f0f0; }
            QLabel#ValueLabel { font-size: 18px; font-weight: bold; color: #1a1d21; border: none; }
            QLabel#TitleLabel { font-size: 13px; color: #7f8c8d; border: none; }
            QLabel#StatLabel  { font-size: 11px; color: #2ecc71; font-weight: bold; border: none; }
            QLabel#IconLabel  { font-size: 22px; border: none; }
        )");

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(10); shadow->setColor(QColor(0, 0, 0, 15)); shadow->setOffset(0, 3);
        setGraphicsEffect(shadow);

        QVBoxLayout *layout = new QVBoxLayout(this); 
        layout->setSpacing(4);
        
        QLabel *lblIcon = new QLabel(iconStr); lblIcon->setObjectName("IconLabel"); layout->addWidget(lblIcon);
        layout->addStretch();
        
        QLabel *lblTitle = new QLabel(title); lblTitle->setObjectName("TitleLabel"); layout->addWidget(lblTitle);
        lblValue = new QLabel(value); lblValue->setObjectName("ValueLabel"); layout->addWidget(lblValue);
        
        QLabel *lblStat = new QLabel(stat); lblStat->setObjectName("StatLabel"); layout->addWidget(lblStat);
    }
    void updateValue(QString newValue) { lblValue->setText(newValue); }
};

// ==========================================
// ২. বড় ব্যানার কার্ড (Take a Break)
// ==========================================
class BannerCard : public QFrame {
public:
    BannerCard(QString color, QString iconStr, QString title, QString desc, QWidget *parent = nullptr) : QFrame(parent) {
        setStyleSheet(QString(R"(
            QFrame { background-color: %1; border-radius: 15px; padding: 18px; }
            QLabel#TitleLabel { font-size: 16px; font-weight: bold; color: #2c3e50; border: none; }
            QLabel#DescLabel  { font-size: 13px; color: #34495e; border: none; }
            QLabel#IconLabel  { font-size: 26px; border: none; }
        )").arg(color));

        QHBoxLayout *mainLayout = new QHBoxLayout(this); mainLayout->setSpacing(12);
        QLabel *lblIcon = new QLabel(iconStr); lblIcon->setObjectName("IconLabel"); mainLayout->addWidget(lblIcon, 0, Qt::AlignTop);

        QVBoxLayout* textLayout = new QVBoxLayout(); textLayout->setSpacing(4);
        QLabel *lblTitle = new QLabel(title); lblTitle->setObjectName("TitleLabel"); textLayout->addWidget(lblTitle);
        QLabel *lblDesc = new QLabel(desc); lblDesc->setObjectName("DescLabel"); lblDesc->setWordWrap(true); textLayout->addWidget(lblDesc);

        mainLayout->addLayout(textLayout, 1);
    }
};

// ==========================================
// ৩. মেইন অ্যাপ উইন্ডো
// ==========================================
class FocusApp : public QMainWindow {
    QTimer *coreTimer;
    InfoCard *cardScreenTime;
    QPushButton *btnToggle;

public:
    FocusApp() {
        setWindowTitle("RasFocus+");
        resize(400, 800); // স্ট্যান্ডার্ড ফোনের সাইজ

        // পুরো অ্যাপের বেসিক স্টাইল
        setStyleSheet(R"(
            QMainWindow { background-color: #f8f9fa; }
            QScrollArea { border: none; background-color: transparent; }
            QWidget#MainContainer { background-color: transparent; }
        )");

        // স্ক্রল এরিয়া সেটআপ (যাতে স্ক্রিন ছোট হলেও সব দেখা যায়)
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        setCentralWidget(scrollArea);

        QWidget* mainContent = new QWidget();
        mainContent->setObjectName("MainContainer");
        scrollArea->setWidget(mainContent);

        QVBoxLayout* mainLayout = new QVBoxLayout(mainContent);
        mainLayout->setContentsMargins(0, 0, 0, 20); // ডানে-বামে জিরো মার্জিন হেডারের জন্য
        mainLayout->setSpacing(15);

        // --- হেডার এরিয়া (নীল গ্রাডিয়েন্ট) ---
        QWidget* headerArea = new QWidget();
        headerArea->setMinimumHeight(180);
        headerArea->setStyleSheet(R"(
            QWidget { background: QLinearGradient(x1:0, y1:0, x2:1, y2:1, stop:0 #1a73e8, stop:1 #4fc3f7); border-bottom-left-radius: 30px; border-bottom-right-radius: 30px; }
            QPushButton { background-color: rgba(255,255,255,0.2); color: white; font-size: 20px; border-radius: 20px; border: none; }
        )");
        
        QVBoxLayout* headerLayout = new QVBoxLayout(headerArea);
        headerLayout->setContentsMargins(20, 30, 20, 20);

        QHBoxLayout* topBar = new QHBoxLayout();
        QPushButton* btnMenu = new QPushButton("≡"); btnMenu->setFixedSize(40, 40);
        QPushButton* btnBell = new QPushButton("🔔"); btnBell->setFixedSize(40, 40);
        topBar->addWidget(btnMenu); topBar->addStretch(); topBar->addWidget(btnBell);
        headerLayout->addLayout(topBar);
        headerLayout->addStretch();
        mainLayout->addWidget(headerArea);

        // --- কন্টেন্ট এরিয়া (মার্জিন সহ) ---
        QVBoxLayout* contentLayout = new QVBoxLayout();
        contentLayout->setContentsMargins(20, 0, 20, 0);
        contentLayout->setSpacing(15);

        // ওয়েলকাম টেক্সট
        QLabel* lblWelcome = new QLabel("Welcome\n<span style='font-size:24px; font-weight:bold; color:#1a1d21;'>Good Morning, Rasel</span>");
        lblWelcome->setTextFormat(Qt::RichText);
        contentLayout->addWidget(lblWelcome);

        // অ্যানালিটিক্স হেডার
        QLabel* lblAnalytics = new QLabel("📊 Analytics");
        lblAnalytics->setStyleSheet("font-size: 15px; color: #7f8c8d; font-weight: bold; margin-top: 10px;");
        contentLayout->addWidget(lblAnalytics);

        // ইনফো কার্ডস (পাশাপাশি)
        QHBoxLayout* cardLayout = new QHBoxLayout(); cardLayout->setSpacing(15);
        cardScreenTime = new InfoCard("⏰", "Screen Time", "0m 0s", "Tracking...", this);
        InfoCard* cardAppLaunches = new InfoCard("🚀", "App Launches", "Active", "Secured", this);
        cardLayout->addWidget(cardScreenTime); cardLayout->addWidget(cardAppLaunches);
        contentLayout->addLayout(cardLayout);

        // ব্যানার কার্ডস
        contentLayout->addWidget(new BannerCard("#e8daef", "☕", "Take a Break", "Take a break from your phone and focus on things that really matter.", this));
        contentLayout->addWidget(new BannerCard("#fef9e7", "📱", "Strictness Level", "Level: Maximum 🔒", this));

        // অ্যাকশন বাটন (Start/Stop)
        btnToggle = new QPushButton("🛡️ Start Protection");
        btnToggle->setMinimumHeight(55);
        btnToggle->setStyleSheet("background-color: #2ecc71; color: white; font-size: 16px; font-weight: bold; border-radius: 12px; margin-top: 10px;");
        connect(btnToggle, &QPushButton::clicked, this, &FocusApp::toggleProtection);
        contentLayout->addWidget(btnToggle);

        contentLayout->addStretch();
        mainLayout->addLayout(contentLayout);

        // টাইমার সেটআপ
        coreTimer = new QTimer(this);
        connect(coreTimer, &QTimer::timeout, this, &FocusApp::systemLoop);
    }

    void toggleProtection() {
        if (!isProtectionActive) {
            isProtectionActive = true;
            btnToggle->setText("🛑 Stop Protection");
            btnToggle->setStyleSheet("background-color: #e74c3c; color: white; font-size: 16px; font-weight: bold; border-radius: 12px; margin-top: 10px;");
            coreTimer->start(1000); 
        } else {
            isProtectionActive = false;
            btnToggle->setText("🛡️ Start Protection");
            btnToggle->setStyleSheet("background-color: #2ecc71; color: white; font-size: 16px; font-weight: bold; border-radius: 12px; margin-top: 10px;");
            coreTimer->stop();
        }
    }

    void systemLoop() {
        if (!isProtectionActive) return;

        // UI আপডেট
        focusTimeSeconds++;
        int m = focusTimeSeconds / 60;
        int s = focusTimeSeconds % 60;
        cardScreenTime->updateValue(QString("%1m %2s").arg(m).arg(s));

        // অ্যান্ড্রয়েড ব্লকিং লজিক কল (জাভা)
#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>(
            "com/rasel/rasfocus/BlockerService", 
            "checkAndBlock", 
            "(Landroid/content/Context;)V", 
            QtAndroid::androidContext().object()
        );
#endif

        // উইন্ডোজ ব্লকিং লজিক
#ifdef Q_OS_WIN
        QStringList blockedApps = {"facebook.exe", "chrome.exe"};
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
        PROCESSENTRY32W pe = {sizeof(pe)};
        if (Process32FirstW(hSnap, &pe)) {
            do {
                QString n = QString::fromWCharArray(pe.szExeFile).toLower();
                if (blockedApps.contains(n, Qt::CaseInsensitive)) {
                    HANDLE ph = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if(ph) { TerminateProcess(ph, 1); CloseHandle(ph); }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
#endif
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    FocusApp window;
    window.show(); // অ্যান্ড্রয়েডে এটি নিজে থেকেই ফুলস্ক্রিন হয়ে যাবে
    return app.exec();
}
