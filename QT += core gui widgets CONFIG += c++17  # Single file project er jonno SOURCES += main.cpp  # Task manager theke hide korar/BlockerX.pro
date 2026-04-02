QT += core gui widgets
CONFIG += c++17

# Single file project er jonno
SOURCES += main.cpp

# Task manager theke hide korar/Console hide korar jonno
win32:CONFIG += console
win32:CONFIG -= console

TARGET = BlockerX
TEMPLATE = app
