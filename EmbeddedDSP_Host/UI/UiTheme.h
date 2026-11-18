#ifndef EMBEDDEDDSP_HOST_UI_THEME_H
#define EMBEDDEDDSP_HOST_UI_THEME_H

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QString>

namespace Host::UI::UiTheme {

/// @brief Brand / focus color (QPalette::Highlight).
inline constexpr char kAccent[] = "#50c88c";
/// @brief Accent on hover.
inline constexpr char kAccentHover[] = "#70dbaf";
/// @brief Soft accent fill (combo selection, etc.).
inline constexpr char kAccentMuted[] = "#243d32";
/// @brief Selected row fill while hovered.
inline constexpr char kAccentMutedHover[] = "#2f5a45";

/// @brief Main window / app chrome background (darker than cards).
inline constexpr char kWindow[] = "#1e1e1e";
/// @brief Card / panel fill.
inline constexpr char kPanel[] = "#2a2a2a";
/// @brief Panel fill on hover.
inline constexpr char kPanelHover[] = "#303030";
/// @brief Dropdown / list row hover.
inline constexpr char kListHover[] = "#5c5c5c";
/// @brief Default border.
inline constexpr char kBorder[] = "#5a5a5a";
/// @brief Plot grid lines (not general UI text).
inline constexpr char kGrid[] = "#333333";
/// @brief Slider grooves. 
inline constexpr char kGroove[] = "#555555";
/// @brief Spectrum plot inner background.
inline constexpr char kPlotBg[] = "#000000";

/// @brief Primary UI text.
inline constexpr char kText[] = "#e0e0e0";
/// @brief Secondary labels (axes, units).
inline constexpr char kTextMuted[] = "#888888";
/// @brief Low-emphasis text (welcome).
inline constexpr char kTextDim[] = "#aaaaaa";

/// @brief Push theme colors into the application palette.
inline void applyApplicationPalette(QApplication& app) {
    QPalette pal = app.palette();
    pal.setColor(QPalette::Window, QColor(QString{kWindow}));
    pal.setColor(QPalette::WindowText, QColor(QString{kText}));
    pal.setColor(QPalette::Base, QColor(QString{kPanel}));
    pal.setColor(QPalette::AlternateBase, QColor(QString{kPlotBg}));
    pal.setColor(QPalette::Text, QColor(QString{kText}));
    pal.setColor(QPalette::Button, QColor(QString{kPanel}));
    pal.setColor(QPalette::ButtonText, QColor(QString{kText}));
    pal.setColor(QPalette::Highlight, QColor(QString{kAccent}));
    pal.setColor(QPalette::HighlightedText, QColor(QString{kWindow}));
    app.setPalette(pal);
}

/// @brief Fill {{token}} placeholders in a stylesheet from this theme.
[[nodiscard]] inline QString resolveStyleSheet(QString qss) {
    const auto put = [&](const char* token, const char* value) {
        qss.replace(QString{token}, QString{value});
    };
    put("{{accent}}", kAccent);
    put("{{accentHover}}", kAccentHover);
    put("{{accentMuted}}", kAccentMuted);
    put("{{accentMutedHover}}", kAccentMutedHover);
    put("{{window}}", kWindow);
    put("{{panel}}", kPanel);
    put("{{panelHover}}", kPanelHover);
    put("{{listHover}}", kListHover);
    put("{{border}}", kBorder);
    put("{{grid}}", kGrid);
    put("{{groove}}", kGroove);
    put("{{plotBg}}", kPlotBg);
    put("{{text}}", kText);
    put("{{textMuted}}", kTextMuted);
    put("{{textDim}}", kTextDim);
    return qss;
}

} // namespace Host::UI::UiTheme

#endif // EMBEDDEDDSP_HOST_UI_THEME_H
