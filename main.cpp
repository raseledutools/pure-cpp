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
#include <QMessageBox>

// --- OS Specific Headers ---
#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniObject>
#endif

// ==========================================
// গ্লোবাল ভ্যারিয়েবল ও ডাটা
// ==========================================
bool isSessionActive = false;
int focusSeconds = 0;

QStringList explicitKeywords = {"porn", "xxx", "sex", "nude", "adult video", "pornhub", "xvideos"};
QStringList blockedApps = {"facebook.exe", "chrome.exe", "msedge.exe"}; // পিসির জন্য ডেমো

// ==========================================
// ১. ড্যাশবোর্ড কার্ড (UI Component)
// ==========================================
class InfoCard : public QFrame {
    QLabel *lblValue;
public:
    InfoCard(QString iconStr, QString title, QString value, QString stat, QWidget *parent = nullptr) : QFrame(parent) {
        setStyleSheet(R"(
            QFrame { background-color: white; border-radius: 15px; padding: 15px; border: 1px solid #e0e6ed; }
            QLabel#ValueLabel { font-size: 20px; font-weight: bold; color: #1a1d21; border: none; }
            QLabel#TitleLabel { font-size: 14px; color: #7f8c8d; border: none; }
            QLabel#StatLabel  { font-size: 12px; color: #2ecc71; font-weight: bold; border: none; }
            QLabel#IconLabel  { font-size: 24px; border: none; }
        )");

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(15); shadow->setColor(QColor(0, 0, 0, 30)); shadow->setOffset(0, 5);
        setGraphicsEffect(shadow);

        QVBoxLayout *layout = new QVBoxLayout(this); layout->setSpacing(5);
        
        QLabel *lblIcon = new QLabel(iconStr); lblIcon->setObjectName("IconLabel"); layout->addWidget(lblIcon);
        layout->addStretch();
        
        QLabel *lblTitle = new QLabel(title); lblTitle->setObjectName("TitleLabel"); layout->addWidget(lblTitle);
        
        lblValue = new QLabel(value); lblValue->setObjectName("ValueLabel"); layout->addWidget(lblValue);
        
        QLabel *lblStat = new QLabel(stat); lblStat->setObjectName("StatLabel"); layout->addWidget(lblStat);
    }

    // ভ্যালু আপডেট করার ফাংশন (টাইমার থেকে কল হবে)
    void updateValue(QString newValue) {
        lblValue->setText(newValue);
    }
};

// ==========================================
// ২. ব্যানার কার্ড (UI Component)
// ==========================================
class BannerCard : public QFrame {
public:
    BannerCard(QString color, QString iconStr, QString title, QString desc, QWidget *parent = nullptr) : QFrame(parent) {
        setStyleSheet(QString(R"(
            QFrame { background-color: %1; border-radius: 15px; padding: 20px; }
            QLabel#TitleLabel { font-size: 18px; font-weight: bold; color: #2c3e50; border: none; }
            QLabel#DescLabel  { font-size: 14px; color: #34495e; border: none; }
            QLabel#IconLabel  { font-size: 28px; border: none; }
        )").arg(color));

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(10); shadow->setColor(QColor(0, 0, 0, 20)); shadow->setOffset(0, 4);
        setGraphicsEffect(shadow);

        QHBoxLayout *mainLayout = new QHBoxLayout(this); mainLayout->setSpacing(15);
        QLabel *lblIcon = new QLabel(iconStr); lblIcon->setObjectName("IconLabel"); mainLayout->addWidget(lblIcon, 0, Qt::AlignTop);

        QVBoxLayout* textLayout = new QVBoxLayout(); textLayout->setSpacing(5);
        QLabel *lblTitle = new QLabel(title); lblTitle->setObjectName("TitleLabel"); textLayout->addWidget(lblTitle);
        QLabel *lblDesc = new QLabel(desc); lblDesc->setObjectName("DescLabel"); lblDesc->setWordWrap(true); textLayout->addWidget(lblDesc);

        mainLayout->addLayout(textLayout, 1);
    }
};

// ==========================================
// ৩. MAIN APPLICATION (লজিক + UI)
// ==========================================
class FocusApp : public QMainWindow {
    QTimer *coreTimer;
    InfoCard *cardScreenTime;
    QPushButton *btnToggleFocus;

public:
    FocusApp() {
        setWindowTitle("RasFocus+ Pro");
        resize(380, 750); 

        setStyleSheet(R"(
            QMainWindow { background-color: white; }
            QLabel { font-family: 'Segoe UI', Arial, sans-serif; }
            QPushButton { font-family: 'Segoe UI', Arial, sans-serif; border-radius: 10px; font-weight: bold;}
            QScrollArea { border: none; background-color: transparent; }
            QWidget#MainContent { background-color: transparent; }
        )");

        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        setCentralWidget(scrollArea);

        QWidget* mainContent = new QWidget();
        mainContent->setObjectName("MainContent");
        scrollArea->setWidget(mainContent);

        QVBoxLayout* mainLayout = new QVBoxLayout(mainContent);
        mainLayout->setContentsMargins(15, 0, 15, 15);
        mainLayout->setSpacing(15);

        // --- হেডার ---
        QWidget* headerArea = new QWidget();
        headerArea->setFixedHeight(160);
        headerArea->setStyleSheet("QWidget { background: QLinearGradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6c5ce7, stop:1 #81ecec); border-bottom-left-radius: 25px; border-bottom-right-radius: 25px; }");
        QVBoxLayout* headerLayout = new QVBoxLayout(headerArea);
        mainLayout->addWidget(headerArea);

        // --- ওয়েলকাম এরিয়া ---
        QVBoxLayout* welcomeLayout = new QVBoxLayout(); welcomeLayout->setSpacing(0);
        QLabel* lblGreeting = new QLabel("Good Morning, Rasel!"); lblGreeting->setStyleSheet("font-size: 22px; font-weight: bold; color: #1a1d21; border: none;");
        welcomeLayout->addWidget(lblGreeting);
        mainLayout->addLayout(welcomeLayout);

        // --- এনালাইটিক্স কার্ড ---
        QHBoxLayout* cardLayout = new QHBoxLayout(); cardLayout->setSpacing(10);
        cardScreenTime = new InfoCard("⏰", "Screen Time", "0 mins 0 sec", "Today", this);
        InfoCard* cardAppLaunches = new InfoCard("🚀", "App Launches", "12", "Tracked", this);
        cardLayout->addWidget(cardScreenTime); cardLayout->addWidget(cardAppLaunches);
        mainLayout->addLayout(cardLayout);

        // --- ব্যানার কার্ড ---
        mainLayout->addWidget(new BannerCard("#e8daef", "☕", "Take a Break", "Focus on things that really matter.", this));

        // --- START/STOP BUTTON ---
        btnToggleFocus = new QPushButton("Start Protection");
        btnToggleFocus->setMinimumHeight(55);
        btnToggleFocus->setStyleSheet("background-color: #2ecc71; color: white; font-size: 18px;");
        connect(btnToggleFocus, &QPushButton::clicked, this, &FocusApp::toggleFocus);
        mainLayout->addWidget(btnToggleFocus);

        mainLayout->addStretch();

        // --- কোর ব্লকিং টাইমার ---
        coreTimer = new QTimer(this);
        connect(coreTimer, &QTimer::timeout, this, &FocusApp::coreLoop);
    }

    void toggleFocus() {
        if (!isSessionActive) {
            isSessionActive = true;
            btnToggleFocus->setText("Stop Protection");
            btnToggleFocus->setStyleSheet("background-color: #e74c3c; color: white; font-size: 18px;");
            coreTimer->start(1000); // প্রতি ১ সেকেন্ডে চেক করবে
            QMessageBox::information(this, "Started", "RasFocus Protection is ON!");
        } else {
            isSessionActive = false;
            btnToggleFocus->setText("Start Protection");
            btnToggleFocus->setStyleSheet("background-color: #2ecc71; color: white; font-size: 18px;");
            coreTimer->stop();
        }
    }

    // ==========================================
    // কোর ব্লকিং লজিক (উইন্ডোজ এবং অ্যান্ড্রয়েড)
    // ==========================================
    void coreLoop() {
        if (!isSessionActive) return;

        // UI আপডেট (স্ক্রিন টাইম বাড়ানো)
        focusSeconds++;
        int m = focusSeconds / 60;
        int s = focusSeconds % 60;
        cardScreenTime->updateValue(QString("%1 mins %2 sec").arg(m).arg(s));

        // --- উইন্ডোজ পিসির ব্লকিং লজিক ---
#ifdef Q_OS_WIN
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
        PROCESSENTRY32W pe = {sizeof(pe)};
        if (Process32FirstW(hSnap, &pe)) {
            do {
                QString n = QString::fromWCharArray(pe.szExeFile).toLower();
                
                // যদি অ্যাপটি ব্লকলিস্টে থাকে, তাহলে কিল করবে
                if (blockedApps.contains(n, Qt::CaseInsensitive)) {
                    HANDLE ph = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if(ph) { TerminateProcess(ph, 1); CloseHandle(ph); }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);

        // --- ব্রাউজারের উইন্ডো টাইটেল চেক (অ্যাডাল্ট ব্লক) ---
        HWND hActive = GetForegroundWindow();
        if (hActive) {
            WCHAR title[512];
            if (GetWindowTextW(hActive, title, 512) > 0) {
                QString sTitle = QString::fromWCharArray(title).toLower();
                for (const QString& keyword : explicitKeywords) {
                    if (sTitle.contains(keyword)) { 
                        ShowWindow(hActive, SW_MINIMIZE); // মিনিমাইজ করে দেবে
                        break;
                    }
                }
            }
        }
#endif

        // --- অ্যান্ড্রয়েডের ব্লকিং লজিক (JNI Call) ---
#ifdef Q_OS_ANDROID
        /* অ্যান্ড্রয়েডে C++ সরাসরি অ্যাপ ব্লক করতে পারে না। 
           তাই আমরা Java এর একটি ফাংশন কল করছি। 
           এই ফাংশনটি আমাদের পরে Java ফাইলে লিখতে হবে।
        */
        QAndroidJniObject::callStaticMethod<void>(
            "com/rasel/rasfocus/BlockerService", // Java ক্লাসের নাম
            "checkAndBlock",                     // Java ফাংশনের নাম
            "()V"
        );
#endif
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    FocusApp window;
    window.show();
    return app.exec();
}
