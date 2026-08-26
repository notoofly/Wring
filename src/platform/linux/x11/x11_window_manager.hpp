#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>

class X11WindowManager : public QObject {
    Q_OBJECT

public:
    explicit X11WindowManager(QObject* parent = nullptr);
    ~X11WindowManager() override;

    bool initialize(void* display, unsigned long rootWindow);
    void shutdown();
};
