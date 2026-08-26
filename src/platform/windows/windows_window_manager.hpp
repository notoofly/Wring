#pragma once

#include <QObject>

class WindowsWindowManager : public QObject {
    Q_OBJECT

public:
    explicit WindowsWindowManager(QObject* parent = nullptr);
    ~WindowsWindowManager() override;
};
