#ifndef EMBEDDEDDSP_HOST_WIDGETRACK_H
#define EMBEDDEDDSP_HOST_WIDGETRACK_H

#include <QWidget>
#include <QLayout>
#include <QMap>

namespace UI {

/**
 * @brief A generic container widget that manages child widgets using both a layout and an ID-based map.
 * @tparam ID Type of the unique identifier (e.g., uint8_t, QString).
 * @tparam WidgetT Type of child widgets (must inherit from QWidget).
 * @tparam LayoutT Type of layout to use (defaults to QVBoxLayout).
 */
template <typename ID, typename WidgetT, typename LayoutT = QVBoxLayout>
class WidgetRack : public QWidget {
public:
    explicit WidgetRack(QWidget* parent = nullptr) : QWidget(parent) {
        m_layout = new LayoutT(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
    }

    /**
     * @brief Adds a widget to the rack.
     * @param id Unique identifier for the widget.
     * @param widget The widget instance.
     * @param stretch Optional stretch factor for the layout.
     */
    void add(const ID& id, WidgetT* widget, int stretch = 0) {
        m_map.insert(id, widget);
        m_layout->addWidget(widget, stretch);
    }

    /**
     * @brief Retrieves a widget by its ID.
     * @return Pointer to the widget or nullptr if not found.
     */
    WidgetT* get(const ID& id) const {
        return m_map.value(id, nullptr);
    }

    /**
     * @brief Removes and deletes all widgets from the rack.
     */
    void clear() {
        qDeleteAll(m_map);
        m_map.clear();
    }

    /**
     * @brief Returns the underlying layout for fine-tuning (margins, spacing, etc.).
     */
    LayoutT* layout() const { return m_layout; }

    /**
     * @brief Returns true if the rack contains no widgets.
     */
    bool isEmpty() const { return m_map.isEmpty(); }

    /**
     * @brief Returns all registered IDs.
     */
    QList<ID> ids() const { return m_map.keys(); }

private:
    LayoutT* m_layout;
    QMap<ID, WidgetT*> m_map;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_WIDGETRACK_H
