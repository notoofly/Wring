#include "wring_settings.hpp"
#include <QKeySequence>

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
    m_triggerButton = m_settings.value("shortcut/button", Left).toInt();
    m_triggerKey = m_settings.value("shortcut/key", 0x006a).toInt(); // XK_j
}

void WringSettings::save()
{
    m_settings.setValue("shortcut/modifier", m_triggerModifier);
    m_settings.setValue("shortcut/button", m_triggerButton);
    m_settings.setValue("shortcut/key", m_triggerKey);
}

QString WringSettings::triggerModifierName() const
{
    if (m_triggerModifier == 0) return "None";

    QStringList parts;
    if (m_triggerModifier & Super) parts << "Super";
    if (m_triggerModifier & Ctrl)  parts << "Ctrl";
    if (m_triggerModifier & Alt)   parts << "Alt";
    if (m_triggerModifier & Shift) parts << "Shift";

    return parts.isEmpty() ? "None" : parts.join("+");
}

QString WringSettings::triggerButtonName() const
{
    switch (m_triggerButton) {
        case Left:   return "Left Click";
        case Middle: return "Middle Click";
        case Right:  return "Right Click";
        default:     return "Left Click";
    }
}

QString WringSettings::triggerKeyName() const
{
    if (m_triggerKey == 0) return "None";

    // X11 keysym to display name
    // Common keysyms
    switch (m_triggerKey) {
        case 0x0041: case 0x0061: return "A";
        case 0x0042: case 0x0062: return "B";
        case 0x0043: case 0x0063: return "C";
        case 0x0044: case 0x0064: return "D";
        case 0x0045: case 0x0065: return "E";
        case 0x0046: case 0x0066: return "F";
        case 0x0047: case 0x0067: return "G";
        case 0x0048: case 0x0068: return "H";
        case 0x0049: case 0x0069: return "I";
        case 0x004a: case 0x006a: return "J";
        case 0x004b: case 0x006b: return "K";
        case 0x004c: case 0x006c: return "L";
        case 0x004d: case 0x006d: return "M";
        case 0x004e: case 0x006e: return "N";
        case 0x004f: case 0x006f: return "O";
        case 0x0050: case 0x0070: return "P";
        case 0x0051: case 0x0071: return "Q";
        case 0x0052: case 0x0072: return "R";
        case 0x0053: case 0x0073: return "S";
        case 0x0054: case 0x0074: return "T";
        case 0x0055: case 0x0075: return "U";
        case 0x0056: case 0x0076: return "V";
        case 0x0057: case 0x0077: return "W";
        case 0x0058: case 0x0078: return "X";
        case 0x0059: case 0x0079: return "Y";
        case 0x005a: case 0x007a: return "Z";
        case 0x0030: return "0";
        case 0x0031: return "1";
        case 0x0032: return "2";
        case 0x0033: return "3";
        case 0x0034: return "4";
        case 0x0035: return "5";
        case 0x0036: return "6";
        case 0x0037: return "7";
        case 0x0038: return "8";
        case 0x0039: return "9";
        case 0xff09: return "Tab";
        case 0xff0d: return "Return";
        case 0xff1b: return "Escape";
        case 0xff20: return "Space";
        case 0xffbe: return "F1";
        case 0xffbf: return "F2";
        case 0xffc0: return "F3";
        case 0xffc1: return "F4";
        case 0xffc2: return "F5";
        case 0xffc3: return "F6";
        case 0xffc4: return "F7";
        case 0xffc5: return "F8";
        case 0xffc6: return "F9";
        case 0xffc7: return "F10";
        case 0xffc8: return "F11";
        case 0xffc9: return "F12";
        default: return QString("Key(%1)").arg(m_triggerKey);
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

void WringSettings::setTriggerKey(int key)
{
    if (m_triggerKey == key) return;
    m_triggerKey = key;
    save();
    emit triggerKeyChanged();
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
    QString mod = triggerModifierName();

    if (m_triggerKey != 0) {
        if (mod == "None")
            return triggerKeyName();
        return mod + "+" + triggerKeyName();
    }

    return mod + " + " + triggerButtonName();
}
