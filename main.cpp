// main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QProgressBar>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QTimer>
#include <QDateTime>
#include <QProcess>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QThread>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QPalette>
#include <QStyle>
#include <QFont>
#include <QScrollArea>
#include <QInputDialog>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// ====================== Keyboard Monitor ======================
class KeyboardMonitor : public QObject {
    Q_OBJECT
public:
    static KeyboardMonitor* instance();
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const { return m_monitoring; }
signals:
    void forbiddenWordTyped(const QString& word);
private:
    explicit KeyboardMonitor(QObject* parent = nullptr);
    static KeyboardMonitor* s_instance;
    bool m_monitoring = false;
#ifdef Q_OS_WIN
    HHOOK m_keyboardHook = nullptr;
    static LRESULT CALLBACK keyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif
};

KeyboardMonitor* KeyboardMonitor::s_instance = nullptr;

KeyboardMonitor* KeyboardMonitor::instance() {
    if (!s_instance) s_instance = new KeyboardMonitor();
    return s_instance;
}

KeyboardMonitor::KeyboardMonitor(QObject* parent) : QObject(parent) { s_instance = this; }

void KeyboardMonitor::startMonitoring() {
    if (m_monitoring) return;
#ifdef Q_OS_WIN
    m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, GetModuleHandle(NULL), 0);
    if (m_keyboardHook) m_monitoring = true;
#endif
}

void KeyboardMonitor::stopMonitoring() {
#ifdef Q_OS_WIN
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }
#endif
    m_monitoring = false;
}

#ifdef Q_OS_WIN
LRESULT CALLBACK KeyboardMonitor::keyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        static QString currentWord;
        if (p->vkCode >= 'A' && p->vkCode <= 'Z') {
            // FIX: Explicitly cast DWORD to uint to resolve ambiguity
            currentWord.append(QChar(static_cast<uint>(p->vkCode)));
            QString wordLower = currentWord.toLower();
            QStringList forbidden = {"porn", "xxx", "adult", "sex", "nude", "erotic"};
            for (const QString& f : forbidden) {
                if (wordLower.contains(f)) {
                    emit KeyboardMonitor::instance()->forbiddenWordTyped(f);
                    currentWord.clear();
                    break;
                }
            }
        } else if (p->vkCode == VK_SPACE || p->vkCode == VK_RETURN) {
            currentWord.clear();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
#endif

// ====================== Halal Guard Overlay ======================
class HalalGuardOverlay : public QWidget {
    Q_OBJECT
public:
    explicit HalalGuardOverlay(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
        setStyleSheet("background-color: black;");
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        QLabel* label = new QLabel("⚠️  PROTECTION ACTIVE  ⚠️\n\nPlease maintain purity in your speech and typing.", this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
        layout->addWidget(label);
        QTimer::singleShot(3000, this, &QWidget::close);
    }
    void showEvent(QShowEvent* event) override {
        if (QScreen* screen = QGuiApplication::primaryScreen()) {
            setGeometry(screen->geometry());
        }
        QWidget::showEvent(event);
    }
};

// ====================== Focus Lock Window ======================
class FocusLockWindow : public QWidget {
    Q_OBJECT
public:
    explicit FocusLockWindow(const QString& profileName, int remainingSeconds, QWidget* parent = nullptr)
        : QWidget(parent), m_remainingSeconds(remainingSeconds) {
        
        Q_UNUSED(profileName); // FIX: Suppress unused parameter warning

        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
        setStyleSheet("background-color: #1E293B;");
        if (QScreen* screen = QGuiApplication::primaryScreen()) {
            setGeometry(screen->geometry());
        }
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        
        QLabel* titleLabel = new QLabel("🔒 FOCUS MODE ACTIVE", this);
        titleLabel->setStyleSheet("color: #3B82F6; font-size: 32px; font-weight: bold; margin-bottom: 20px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);
        
        m_timeLabel = new QLabel(this);
        m_timeLabel->setStyleSheet("color: #F59E0B; font-size: 48px; font-weight: bold; font-family: monospace;");
        m_timeLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_timeLabel);
        
        updateTimerDisplay();
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &FocusLockWindow::updateTimer);
        m_timer->start(1000);
        
        installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_F4 || keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Escape || keyEvent->key() == Qt::Key_Super_L) {
                return true;
            }
        }
        return QWidget::eventFilter(obj, event);
    }
    
signals:
    void sessionEnded();

private slots:
    void updateTimer() {
        if (m_remainingSeconds > 0) {
            m_remainingSeconds--;
            updateTimerDisplay();
        } else {
            m_timer->stop();
            emit sessionEnded();
            close();
        }
    }
private:
    void updateTimerDisplay() {
        int hours = m_remainingSeconds / 3600;
        int minutes = (m_remainingSeconds % 3600) / 60;
        int seconds = m_remainingSeconds % 60;
        m_timeLabel->setText(QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
    }
    int m_remainingSeconds;
    QLabel* m_timeLabel;
    QTimer* m_timer;
};

// ====================== Main Window ======================
class RasBlockerPro : public QMainWindow {
    Q_OBJECT
public:
    RasBlockerPro(QWidget* parent = nullptr) : QMainWindow(parent), m_activeLockWindow(nullptr) {
        setWindowTitle("RasBlocker Pro");
        setMinimumSize(800, 600);
        
        setupTrayIcon();
        setupAutoStart(); 
        
        QWidget* central = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(central);
        QPushButton* startBtn = new QPushButton("Start 1 Hour Focus Session", this);
        layout->addWidget(startBtn);
        setCentralWidget(central);
        
        connect(startBtn, &QPushButton::clicked, [this]() {
            startSession(3600); // 1 Hour
        });

        connect(KeyboardMonitor::instance(), &KeyboardMonitor::forbiddenWordTyped,
                this, &RasBlockerPro::onForbiddenWordTyped);
        
        KeyboardMonitor::instance()->startMonitoring();
        checkPreviousSession();
    }

    ~RasBlockerPro() {
        KeyboardMonitor::instance()->stopMonitoring();
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        if (trayIcon->isVisible() && !m_isForceQuitting) {
            QMessageBox::information(this, "Running in Background", "App is still guarding your PC in the background.");
            hide();
            event->ignore();
        }
    }

private slots:
    void startSession(int durationSeconds) {
        if (m_activeLockWindow) return;
        
        QDateTime endTime = QDateTime::currentDateTime().addSecs(durationSeconds);
        QSettings settings("RasBlocker", "Pro");
        settings.setValue("activeSessionEndTime", endTime);
        
        launchLockWindow(durationSeconds);
    }

    void onForbiddenWordTyped(const QString& word) {
        Q_UNUSED(word); // FIX: Suppress unused parameter warning
        HalalGuardOverlay* overlay = new HalalGuardOverlay();
        overlay->show();
    }

    void quitApplication() {
        m_isForceQuitting = true;
        QApplication::quit();
    }

private:
    void setupTrayIcon() {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
        
        QMenu* trayMenu = new QMenu(this);
        QAction* showAction = trayMenu->addAction("Open Dashboard");
        QAction* quitAction = trayMenu->addAction("Quit (Requires Admin)");
        
        connect(showAction, &QAction::triggered, this, &QWidget::show);
        connect(quitAction, &QAction::triggered, this, &RasBlockerPro::quitApplication);
        
        trayIcon->setContextMenu(trayMenu);
        trayIcon->show();
    }

    void setupAutoStart() {
        QSettings bootSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        bootSettings.setValue("RasBlockerPro", appPath);
    }

    void checkPreviousSession() {
        QSettings settings("RasBlocker", "Pro");
        if (settings.contains("activeSessionEndTime")) {
            QDateTime endTime = settings.value("activeSessionEndTime").toDateTime();
            QDateTime now = QDateTime::currentDateTime();
            
            if (endTime > now) {
                int remainingSeconds = now.secsTo(endTime);
                launchLockWindow(remainingSeconds);
            } else {
                settings.remove("activeSessionEndTime");
            }
        }
    }

    void launchLockWindow(int remainingSeconds) {
        m_activeLockWindow = new FocusLockWindow("Active Profile", remainingSeconds, nullptr);
        m_activeLockWindow->showFullScreen();
        
        connect(m_activeLockWindow, &FocusLockWindow::sessionEnded, this, [this]() {
            QSettings settings("RasBlocker", "Pro");
            settings.remove("activeSessionEndTime"); 
            m_activeLockWindow->deleteLater();
            m_activeLockWindow = nullptr;
        });
    }

    FocusLockWindow* m_activeLockWindow;
    QSystemTrayIcon* trayIcon;
    bool m_isForceQuitting = false;
};

// ====================== Main Entry ======================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false); 
    app.setStyle("Fusion");
    
    RasBlockerPro window;
    window.show(); 
    
    return app.exec();
}

#include "main.moc"
