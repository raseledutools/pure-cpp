QT       += core gui widgets
CONFIG   += c++17

TARGET   = RasFocusApp
TEMPLATE = app
SOURCES  += main.cpp

# Windows Specific Settings
win32 {
    CONFIG -= console
    LIBS += -luser32 -lshell32 -ladvapi32 -ldwmapi -lgdi32 -lOle32
}

# Android Specific Settings
android {
    QT += androidextras
}
