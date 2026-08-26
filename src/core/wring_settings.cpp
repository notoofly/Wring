#include "wring_settings.hpp"

WringSettings::WringSettings(QObject* parent)
    : QObject(parent)
    , m_settings("wring", "wring")
{
    load();
}

WringSettings::~WringSettings() = default;

void WringSettings::load()
{
    m_triggerModifier = m_settings.value("shortcut/modifier", Super).toInt();
    m_triggerButton = m_settings.value("shortcut/button", Right).toInt();
}

void WringSettings::save()
{
    m_settings.setValue("shortcut/modifier", m_triggerModifier);
    m_settings.setValue("shortcut/button", m_triggerButton);
}

QString WringSettings::triggerModifierName() const
{
    switch (m_triggerModifier) {
        case Super: return "Super";
        case Ctrl: return "Ctrl";
        case Alt: return "Alt";
        case CtrlAlt: return "Ctrl+Alt";
        default: return "Super";
    }
}

QString WringSettings::triggerButtonName() const
{
    switch (m_triggerButton) {
        case Right: return "Right Click";
        case Middle: return "Middle Click";
        case Left: return "Left Click";
        default: return "Right Click";
    }
}

void WringSettings::setTriggerModifier(int modifier)
{
    if (m_triggerModifier == modifier) return;
    m_triggerModifier = modifier;
    save();
    emit triggerModifierChanged();
}

void WringSettings::setTriggerButton(int button)
{
    if (m_triggerButton == button) return;
    m_triggerButton = button;
    save();
    emit triggerButtonChanged();
}

QString WringSettings::shortcutString() const
{
    return triggerModifierName() + " + " + triggerButtonName();
}
