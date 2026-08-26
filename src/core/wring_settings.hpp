#pragma once

#include <QObject>
#include <QSettings>

class WringSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(int triggerModifier READ triggerModifier WRITE setTriggerModifier NOTIFY triggerModifierChanged)
    Q_PROPERTY(int triggerButton READ triggerButton WRITE setTriggerButton NOTIFY triggerButtonChanged)
    Q_PROPERTY(QString triggerModifierName READ triggerModifierName NOTIFY triggerModifierChanged)
    Q_PROPERTY(QString triggerButtonName READ triggerButtonName NOTIFY triggerButtonChanged)

public:
    // X11 modifier masks — combinable via bitmask OR
    enum Modifier {
        Shift  = 1,   // ShiftMask
        Ctrl    = 4,   // ControlMask
        Alt     = 8,   // Mod1Mask
        Super   = 64   // Mod4Mask
    };
    Q_ENUM(Modifier)

    enum MouseButton {
        Left   = 1,
        Middle = 2,
        Right  = 3
    };
    Q_ENUM(MouseButton)

    explicit WringSettings(QObject* parent = nullptr);
    ~WringSettings() override;

    int triggerModifier() const { return m_triggerModifier; }
    int triggerButton() const { return m_triggerButton; }

    QString triggerModifierName() const;
    QString triggerButtonName() const;

    Q_INVOKABLE void setTriggerModifier(int modifier);
    Q_INVOKABLE void setTriggerButton(int button);
    Q_INVOKABLE void toggleModifier(int mask);
    Q_INVOKABLE bool hasModifier(int mask) const;
    Q_INVOKABLE QString shortcutString() const;

signals:
    void triggerModifierChanged();
    void triggerButtonChanged();

private:
    void load();
    void save();

    QSettings m_settings;
    int m_triggerModifier = Super;
    int m_triggerButton = Right;
};
