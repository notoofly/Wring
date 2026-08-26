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
    enum Modifier {
        Super = 4,
        Ctrl = 2,
        Alt = 8,
        CtrlAlt = 10
    };
    Q_ENUM(Modifier)

    enum MouseButton {
        Right = 3,
        Middle = 2,
        Left = 1
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
