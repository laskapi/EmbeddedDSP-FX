#ifndef EMBEDDEDDSP_HOST_EQ_CURVE_COLOR_POOL_H
#define EMBEDDEDDSP_HOST_EQ_CURVE_COLOR_POOL_H

#include <QColor>
#include <QVector>

namespace Host::UI::Effects {

/// @brief Sticky EQ curve colors (not tied to slot index — survives rack reorder).
class EqCurveColorPool {
public:
    EqCurveColorPool() {
        m_entries = {
            {QColor(QStringLiteral("#5ec8ff")), false},
            {QColor(QStringLiteral("#ffb454")), false},
            {QColor(QStringLiteral("#c792ea")), false},
            {QColor(QStringLiteral("#82aaff")), false},
            {QColor(QStringLiteral("#ff7b72")), false},
            {QColor(QStringLiteral("#7ee787")), false},
        };
    }

    /// @brief Reserves a palette color.
    [[nodiscard]] QColor acquire() {
        for (Entry& e : m_entries) {
            if (!e.inUse) {
                e.inUse = true;
                return e.color;
            }
        }
        return m_entries.isEmpty() ? QColor() : m_entries.front().color;
    }

    void release(const QColor& color) {
        for (Entry& e : m_entries) {
            if (e.color == color) {
                e.inUse = false;
                return;
            }
        }
    }

    void reset() {
        for (Entry& e : m_entries) {
            e.inUse = false;
        }
    }

private:
    struct Entry {
        QColor color;
        bool inUse{false};
    };

    QVector<Entry> m_entries;
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EQ_CURVE_COLOR_POOL_H
