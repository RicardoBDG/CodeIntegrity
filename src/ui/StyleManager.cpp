#include "StyleManager.hpp"

QString StyleManager::baseButtonStyle(const QString& bgColor, const QString& bgHover,
                                      const QString& bgActive, const QString& textColor,
                                      bool disabled)
{
    if (disabled)
    {
        return QString(R"(
            QPushButton {
                background-color: %1;
                color: %2;
                border: none;
                border-radius: 6px;
                font-weight: 600;
                font-size: 14px;
                font-family: "Segoe UI", Arial, sans-serif;
                padding: 10px;
            }
            QPushButton:hover {
                background-color: %1;
            }
            QPushButton:pressed {
                background-color: %1;
            }
        )").arg(bgColor, textColor);
    }

    return QString(R"(
        QPushButton {
            background-color: %1;
            color: %3;
            border: none;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
            font-family: "Segoe UI", Arial, sans-serif;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: %2;
        }
        QPushButton:pressed {
            background-color: %4;
        }
    )").arg(bgColor, bgHover, textColor, bgActive);
}

QString StyleManager::baseListWidgetStyle(const QString& bgColor, const QString& itemBgColor,
                                          const QString& hoverColor, const QString& selectColor)
{
    return QString(R"(
        QListWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 6px;
            padding: 5px;
            outline: none;
        }
        QListWidget::item {
            padding: 8px;
            background-color: %3;
            color: #ffffff;
        }
        QListWidget::item:hover {
            background-color: %4;
        }
        QListWidget::item:selected {
            background-color: %5;
            color: #ffffff;
        }
        QScrollBar:vertical {
            border: none;
            background: #2a2a2a;
            width: 12px;
            margin: 15px 0;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #4a4a4a;
            min-height: 30px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #5a5a5a;
        }
        QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )").arg(bgColor, DARK_BORDER, itemBgColor, hoverColor, selectColor);
}

QString StyleManager::getListWidgetStyle()
{
    return baseListWidgetStyle(DARK_BG_PRIMARY, DARK_BG_SECONDARY,
                               "#4a4a4a", PRIMARY_COLOR);
}

QString StyleManager::getLineEditStyle()
{
    return QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 10px 12px;
            min-height: 30px;
            height: 30px;
            font-size: 13px;
            font-family: "Segoe UI", Arial, sans-serif;
            selection-background-color: %4;
        }
        QLineEdit:focus {
            border: 2px solid %4;
            background-color: %5;
        }
        QLineEdit::placeholder {
            color: #888888;
        }
    )").arg(DARK_BG_SECONDARY, TEXT_PRIMARY, DARK_BORDER, PRIMARY_COLOR, "#323232");
}

QString StyleManager::getLineEditErrorStyle()
{
    return QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 2px solid %3;
            border-radius: 6px;
            padding: 10px 12px;
            min-height: 30px;
            height: 30px;
            font-size: 13px;
            font-family: "Segoe UI", Arial, sans-serif;
            selection-background-color: rgba(255, 107, 107, 0.3);
        }
        QLineEdit:focus {
            border: 2px solid %3;
            background-color: %1;
        }
    )").arg(DARK_BG_SECONDARY, TEXT_PRIMARY, ERROR_COLOR);
}


QString StyleManager::getCheckBoxStyle()
{
    return QString(R"(
        QCheckBox {
            color: %1;
            font-size: 13px;
            font-weight: 500;
            spacing: 8px;
            padding: 6px;
        }
        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border-radius: 4px;
            border: 2px solid #555;
            background-color: %2;
        }
        QCheckBox::indicator:checked {
            background-color: %3;
            border-color: %3;
            image: url(:/icons/check_white.png);
        }
        QCheckBox::indicator:hover {
            border-color: %4;
        }
        QCheckBox::indicator:checked:hover {
            background-color: %4;
            border-color: %4;
        }
        QCheckBox:disabled {
            color: %5;
        }
        QCheckBox::indicator:disabled {
            border-color: #444;
            background-color: #1a1a1a;
        }
    )").arg(TEXT_PRIMARY, DARK_BG_TERTIARY, PRIMARY_COLOR, PRIMARY_HOVER, TEXT_TERTIARY);
}

QString StyleManager::getTeacherListWidgetStyle()
{
    return QString(R"(
        QListWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 5px;
            outline: none;
        }
        QListWidget::item {
            background-color: transparent;
            border: none;
            padding: 0px;
            margin: 2px 0px;
        }
        QListWidget::item:selected {
            background-color: rgba(76, 175, 80, 0.2);
            border-radius: 6px;
        }
        QListWidget::item:hover {
            background-color: rgba(255, 255, 255, 0.08);
            border-radius: 6px;
        }
        QScrollBar:vertical {
            background-color: #1f1f1f;
            width: 14px;
            border-radius: 7px;
            margin: 2px 0;
        }
        QScrollBar::handle:vertical {
            background-color: #4a4a4a;
            border-radius: 7px;
            min-height: 30px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #5a5a5a;
        }
        QScrollBar::handle:vertical:pressed {
            background-color: %3;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
            background: none;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )").arg(DARK_BG_PRIMARY, DARK_BORDER, PRIMARY_COLOR);
}

QString StyleManager::getItemWidgetStyle()
{
    return QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 8px;
            border: 1px solid %2;
        }
        QWidget:hover {
            background-color: %3;
            border-color: #5a5a5a;
        }
    )").arg(DARK_BG_TERTIARY, DARK_BORDER, "#454545");
}

QString StyleManager::getLabelTitleStyle()
{
    return QString(R"(
        QLabel {
            color: %1;
            font-weight: bold;
            font-size: 20px;
            font-family: "Segoe UI", Arial, sans-serif;
            padding: 0px;
            background-color: transparent;
        }
    )").arg(TEXT_PRIMARY);
}

QString StyleManager::getLabelSubtitleStyle()
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 13px;
            font-family: "Segoe UI", Arial, sans-serif;
            padding: 0px;
            background-color: transparent;
            font-weight: 300;
        }
    )").arg(TEXT_SECONDARY);
}

QString StyleManager::getLabelSectionStyle()
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 14px;
            font-weight: 600;
            font-family: "Segoe UI", Arial, sans-serif;
            background-color: transparent;
            letter-spacing: 1px;
        }
    )").arg(TEXT_PRIMARY);
}

QString StyleManager::getLabelDescriptionStyle()
{
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 12px;
            font-family: "Segoe UI", Arial, sans-serif;
            padding: 0px;
            background-color: transparent;
            font-weight: 500;
        }
    )").arg(TEXT_SECONDARY);
}

QString StyleManager::getButtonStyle(const QString& backgroundColor, const QString& textColor)
{
    return baseButtonStyle(backgroundColor, PRIMARY_HOVER, PRIMARY_ACTIVE, textColor, false);
}

QString StyleManager::getDisabledButtonStyle(const QString& backgroundColor, const QString& textColor)
{
    return baseButtonStyle(backgroundColor, backgroundColor, backgroundColor, textColor, true);
}

void StyleManager::applyDisabledStyle(QPushButton* btn)
{
    if (!btn) return;
    btn->setProperty("originalStyleSheet", btn->styleSheet());
    btn->setStyleSheet(getDisabledButtonStyle(DISABLE_BG, DISABLE_TEXT));
    btn->setEnabled(false);
}

void StyleManager::removeDisabledStyle(QPushButton* btn)
{
    if (!btn) return;
    QString originalStyle = btn->property("originalStyleSheet").toString();
    if (!originalStyle.isEmpty()) {
        btn->setStyleSheet(originalStyle);
    }
    btn->setEnabled(true);
}
