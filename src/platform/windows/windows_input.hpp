#pragma once

#include <QObject>

class WindowsInput : public QObject {
    Q_OBJECT

public:
    explicit WindowsInput(QObject* parent = nullptr);
    ~WindowsInput() override;
};
