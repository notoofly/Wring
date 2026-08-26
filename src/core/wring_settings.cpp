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
    m_triggerModifier = m_settings.value("shortcut/modifier", Ctrl | Shift).toInt();
    m_triggerButton = m_settings.value("shortcut/button", Right).toInt();
}

void WringSettings::save()
{
    m_settings.setValue("shortcut/modifier", m_triggerModifier);
    m_settings.setValue("shortcut/button", m_triggerButton);
}

QString WringSettings::triggerModifierName() const
{
    if (m_triggerModifier == 0) return "None";

    QStringList parts;
    if (m_triggerModifier & Super)   parts << "Super";
    if (m_triggerModifier & Ctrl)    parts << "Ctrl";
    if (m_triggerModifier & Alt)     parts << "Alt";
    if (m_triggerModifier & Shift)   parts << "Shift";

    return parts.isEmpty() ? "None" : parts.join("+");
}

QString WringSettings::triggerButtonName() const
{
    switch (m_triggerButton) {
        case Left:   return "Left Click";
        case Middle: return "Middle Click";
        case Right:  return "Right Click";
        default:     return "Right Click";
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

void WringSettings::toggleModifier(int mask)
{
    int newMod = m_triggerModifier ^ mask;
    if (newMod == 0) return;
    setTriggerModifier(newMod);
}

bool WringSettings::hasModifier(int mask) const
{
    return (m_triggerModifier & mask) != 0;
}

QString WringSettings::shortcutString() const
{
    return triggerModifierName() + " + " + triggerButtonName();
}
