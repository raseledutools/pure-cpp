#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>
#include <QFontDatabase>
#include <QCloseEvent>

// ==========================================
// OS SPECIFIC INCLUDES
// ==========================================
#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

// ==========================================
// DATA STRUCTURES
// ==========================================
QStringList blockedApps, blockedWebs, allowedApps, allowedWebs;

QStringList systemApps = {
    "explorer.exe", "svchost.exe", "taskmgr.exe", "cmd.exe", "conhost.exe", "csrss.exe", "dwm.exe"
};

QStringList hiddenUIProcesses = {
    "svchost.exe", "smss.exe", "csrss.exe", "services.exe", "lsass.exe", "wininit.exe"
};

QStringList explicitKeywords = {"porn", "xxx", "sex", "nude", "nsfw", "adult video", "pornhub", "xvideos", "bangla choti"};
QStringList defaultAdultWebs = {"pornhub", "xvideos", "xnxx", "xhamster", "brazzers", "redtube"};

QStringList defaultSystemBlocks = {"regedit.exe", "mmc.exe", "systemsettings.exe", "control.exe", "gpedit.msc"};
QStringList defaultSystemTitles = {"control panel", "registry editor", "local group policy editor", "settings"};

QStringList islamicQuotes = {
    "\"মুমিনদের বলুন, তারা যেন তাদের দৃষ্টি নত রাখে এবং তাদের যৌনাঙ্গর হেফাযত করে।\" - (সূরা আন-নূর: ৩০)",
    "\"লজ্জাশীলতা কল্যাণ ছাড়া আর কিছুই বয়ে আনে না।\" - (সহীহ বুখারী)"
};

QStringList timeQuotes = {
    "\"যারা সময়কে মূল্যায়ন করে না, সময়ও তাদেরকে মূল্যায়ন করে না।\" - এ.পি.জে. আবদুল কালাম",
    "\"আজকের দিনটি নষ্ট করার অর্থ হলো ভবিষ্যতের একটি উজ্জ্বল দিন চুরি করা।\""
};

bool isSessionActive = false;
bool isTimeMode = false;
bool isPassMode = false;
bool useAllowMode = false; 
bool isOverlayVisible = false;

QString currentSessionPass = "";
int focusTimeTotalSeconds = 0;
int timerTicks = 0;

// ==========================================
// CROSS-PLATFORM UTILITY FUNCTIONS
// ==========================================
bool CheckAdminOrPermissions() {
#ifdef Q_OS_WIN
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
#elif defined(Q_OS_ANDROID)
    // Android e by default normal permission thake, OS level perm pore Java diye nite hobe
    return true; 
#endif
}

QString CleanURL(QString s) {
    s = s.toLower();
    QStringList to_erase = {"https://", "http://", "www.", "/*"};
    for (const QString& prefix : to_erase) s.replace(prefix, "");
    if(s.endsWith("/")) s.chop(1);
    
    QStringList domains = {".com", ".org", ".net", ".io", ".co", ".bd"};
    for (const QString& dom : domains) {
        int pos = s.indexOf(dom);
        if (pos != -1) s = s.left(pos);
    }
    return s; 
}

QStringList GetActiveProcesses() {
    QStringList processes;
#ifdef Q_OS_WIN
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = {sizeof(pe)}; 
    if (Process32FirstW(h, &pe)) {
        do { processes.append(QString::fromWCharArray(pe.szExeFile)); } while (Process32NextW(h, &pe));
    }
    CloseHandle(h);
#elif defined(Q_OS_ANDROID)
    // Android process fetching placeholder
    processes.append("com.android.chrome");
    processes.append("com.facebook.katana");
#endif
    processes.removeDuplicates();
    processes.sort(Qt::CaseInsensitive);
    return processes;
}

void SaveDataList(const QString& filename, const QStringList& vec) {
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& item : vec) out << item << "\n";
        file.close();
    }
}

// ==========================================
// MAIN UI CLASS
// ==========================================
class OverlayWindow : public QWidget {
public:
    QLabel* textLabel;
    QTimer* hideTimer;

    OverlayWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        
        resize(350, 200); // Mobile er jonno ektu choto kora holo
        QVBoxLayout* layout = new QVBoxLayout(this);
        textLabel = new QLabel(this);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setWordWrap(true);
        textLabel->setFont(QFont("Segoe UI", 16, QFont::Bold));
        layout->addWidget(textLabel);
        
        hideTimer = new QTimer(this);
        connect(hideTimer, &QTimer::timeout, this, &OverlayWindow::hideOverlay);
    }

    void showMessage(int type) {
        QString quote = (type == 1) ? islamicQuotes[rand() % islamicQuotes.size()] : timeQuotes[rand() % timeQuotes.size()];
        if(type == 1) setStyleSheet("QWidget { background-color: #093d1f; border: 4px solid #f1c40f; border-radius: 10px; } QLabel { color: #ffffff; border: none; }");
        else setStyleSheet("QWidget { background-color: #1a252f; border: 4px solid #3498db; border-radius: 10px; } QLabel { color: #ffffff; border: none; }");
        
        textLabel->setText(quote);
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);

        isOverlayVisible = true; show(); hideTimer->start(6000); 
    }
    void hideOverlay() { hide(); hideTimer->stop(); isOverlayVisible = false; }
};

class FocusApp : public QMainWindow {
public:
    OverlayWindow* overlay;
    QTimer* coreTimer;
    
    QLineEdit* passEdit; QSpinBox* hrSpin; QSpinBox* minSpin;
    QPushButton* btnStart; QPushButton* btnStop; QLabel* lblTimeLeft;
    QRadioButton* rbBlock; QRadioButton* rbAllow;
    
    QListWidget* listBlockApps; QComboBox* inputBlockWeb;

    FocusApp() {
        setWindowTitle("RasFocus - Pro");
        resize(800, 600);
        
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

        // UI Setup (Simplified for cross platform view)
        QHBoxLayout* topLayout = new QHBoxLayout();
        passEdit = new QLineEdit(); passEdit->setPlaceholderText("Password");
        hrSpin = new QSpinBox(); hrSpin->setRange(0, 24);
        minSpin = new QSpinBox(); minSpin->setRange(0, 59);
        btnStart = new QPushButton("START");
        btnStop = new QPushButton("STOP");
        lblTimeLeft = new QLabel("00:00:00");
        
        topLayout->addWidget(passEdit); topLayout->addWidget(hrSpin); topLayout->addWidget(minSpin);
        topLayout->addWidget(btnStart); topLayout->addWidget(btnStop); topLayout->addWidget(lblTimeLeft);
        mainLayout->addLayout(topLayout);

        listBlockApps = new QListWidget();
        mainLayout->addWidget(new QLabel("Blocked Items:"));
        mainLayout->addWidget(listBlockApps);

        overlay = new OverlayWindow();

        connect(btnStart, &QPushButton::clicked, this, &FocusApp::startFocus);
        connect(btnStop, &QPushButton::clicked, this, &FocusApp::stopFocus);

        coreTimer = new QTimer(this);
        connect(coreTimer, &QTimer::timeout, this, &FocusApp::coreLoop);
        coreTimer->start(1000);
    }

    void startFocus() {
        if (!isSessionActive) {
            focusTimeTotalSeconds = (hrSpin->value() * 3600) + (minSpin->value() * 60);
            if(focusTimeTotalSeconds > 0) { isTimeMode = true; timerTicks = 0; }
            isSessionActive = true;
            QMessageBox::information(this, "Started", "RasFocus is now active!");
        }
    }

    void stopFocus() {
        isSessionActive = false;
        QMessageBox::information(this, "Stopped", "RasFocus disabled.");
    }

    void coreLoop() {
        if (!isSessionActive) return;

        if (isTimeMode && focusTimeTotalSeconds > 0) {
            timerTicks++;
            int timeLeft = focusTimeTotalSeconds - timerTicks;
            if (timeLeft <= 0) {
                isSessionActive = false; isTimeMode = false; lblTimeLeft->setText("00:00:00");
                QMessageBox::information(this, "Success", "Time is up!");
                return;
            }
            int h = timeLeft / 3600; int m = (timeLeft % 3600) / 60; int s = timeLeft % 60;
            lblTimeLeft->setText(QString("%1:%2:%3").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
        }

#ifdef Q_OS_WIN
        // WINDOWS ONLY LOGIC (Task killing)
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); PROCESSENTRY32W pe = {sizeof(pe)};
        if (Process32FirstW(hSnap, &pe)) {
            do {
                QString n = QString::fromWCharArray(pe.szExeFile).toLower();
                if (n == "taskmgr.exe") {
                    HANDLE ph = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if(ph) { TerminateProcess(ph, 1); CloseHandle(ph); }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
#elif defined(Q_OS_ANDROID)
        // ANDROID ONLY LOGIC (Placeholder for JNI calls)
        // Ekhane amra pore Android API call korbo
#endif
    }
};

int main(int argc, char *argv[]) {
    if (!CheckAdminOrPermissions()) {
#ifdef Q_OS_WIN
        WCHAR path[MAX_PATH]; GetModuleFileNameW(NULL, path, MAX_PATH);
        SHELLEXECUTEINFOW sei = { 0 }; sei.cbSize = sizeof(sei); sei.lpVerb = L"runas"; sei.lpFile = path; sei.nShow = SW_NORMAL;
        ShellExecuteExW(&sei);
        return 0; 
#endif
    }

    QApplication app(argc, argv);
    FocusApp window;
    window.show();
    return app.exec();
}
