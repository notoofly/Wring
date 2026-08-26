#pragma once

#include <QObject>
#include <QSettings>

class WringSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(int triggerModifier READ triggerModifier WRITE setTriggerModifier NOTIFY triggerModifierChanged)
    Q_PROPERTY(int triggerButton READ triggerButton WRITE setTriggerButton NOTIFY triggerButtonChanged)
    Q_PROPERTY(int triggerKey READ triggerKey WRITE setTriggerKey NOTIFY triggerKeyChanged)
    Q_PROPERTY(QString triggerModifierName READ triggerModifierName NOTIFY triggerModifierChanged)
    Q_PROPERTY(QString triggerButtonName READ triggerButtonName NOTIFY triggerButtonChanged)
    Q_PROPERTY(QString triggerKeyName READ triggerKeyName NOTIFY triggerKeyChanged)

public:
    // X11 modifier masks
    enum Modifier {
        Shift  = 1,
        Ctrl   = 4,
        Alt    = 8,
        Super  = 64
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
    int triggerKey() const { return m_triggerKey; }

    QString triggerModifierName() const;
    QString triggerButtonName() const;
    QString triggerKeyName() const;

    Q_INVOKABLE void setTriggerModifier(int modifier);
    Q_INVOKABLE void setTriggerButton(int button);
    Q_INVOKABLE void setTriggerKey(int key);
    Q_INVOKABLE void toggleModifier(int mask);
    Q_INVOKABLE bool hasModifier(int mask) const;
    Q_INVOKABLE QString shortcutString() const;

signals:
    void triggerModifierChanged();
    void triggerButtonChanged();
    void triggerKeyChanged();

private:
    void load();
    void save();

    QSettings m_settings;
    int m_triggerModifier = Super;
    int m_triggerButton = Left;
    int m_triggerKey = 0; // 0 = no keyboard shortcut
};
