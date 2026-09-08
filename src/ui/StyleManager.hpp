#ifndef STYLE_MANAGER_HPP
#define STYLE_MANAGER_HPP

#include <QString>
#include <QPushButton>

class StyleManager
{
private:
    static constexpr const char* DARK_BG_PRIMARY = "#1e1e1e";
    static constexpr const char* DARK_BG_SECONDARY = "#2d2d2d";
    static constexpr const char* DARK_BG_TERTIARY = "#3a3a3a";
    static constexpr const char* DARK_BORDER = "#3d3d3d";

    static constexpr const char* PRIMARY_COLOR = "#4CAF50";
    static constexpr const char* PRIMARY_HOVER = "#45a049";
    static constexpr const char* PRIMARY_ACTIVE = "#3d8b40";

    static constexpr const char* TEXT_PRIMARY = "#ffffff";
    static constexpr const char* TEXT_SECONDARY = "#aaaaaa";
    static constexpr const char* TEXT_TERTIARY = "#666666";

    static constexpr const char* ERROR_COLOR = "#ff6b6b";
    static constexpr const char* ERROR_BG = "rgba(255, 107, 107, 0.1)";

    static constexpr const char* DISABLE_BG = "#555555";
    static constexpr const char* DISABLE_TEXT = "#999999";

    QString baseButtonStyle(const QString& bgColor, const QString& bgHover,
                            const QString& bgActive, const QString& textColor,
                            bool disabled = false);
    QString baseListWidgetStyle(const QString& bgColor, const QString& itemBgColor,
                                const QString& hoverColor, const QString& selectColor);

public:
    StyleManager() = default;
    ~StyleManager() = default;

    QString getListWidgetStyle();
    QString getLineEditStyle();
    QString getLineEditErrorStyle();
    QString getCheckBoxStyle();
    QString getTeacherListWidgetStyle();
    QString getItemWidgetStyle();
    QString getLabelTitleStyle();
    QString getLabelSubtitleStyle();
    QString getLabelSectionStyle();
    QString getLabelDescriptionStyle();
    QString getButtonStyle(const QString& backgroundColor = PRIMARY_COLOR,
                           const QString& textColor = TEXT_PRIMARY);
    QString getDisabledButtonStyle(const QString& backgroundColor = DISABLE_BG,
                                   const QString& textColor = DISABLE_TEXT);

    void applyDisabledStyle(QPushButton* btn);
    void removeDisabledStyle(QPushButton* btn);

    static constexpr const char* getPrimaryColor() { return PRIMARY_COLOR; }
    static constexpr const char* getErrorColor() { return ERROR_COLOR; }
    static constexpr const char* getDarkBgPrimary() { return DARK_BG_PRIMARY; }
};

#endif
