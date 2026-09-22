#ifndef EMBEDDEDDSP_HOST_WIDGETRACK_H
#define EMBEDDEDDSP_HOST_WIDGETRACK_H

#include <QWidget>
#include <QLayout>
#include <QMap>

namespace Host::UI {

/**
 * @brief Generic container for managing child widgets with layout and ID-based map.
 */
template <typename ID, typename WidgetT, typename LayoutT = QVBoxLayout>
class WidgetRack : public QWidget {
public:
    explicit WidgetRack(QWidget* parent = nullptr) : QWidget(parent) {
        m_layout = new LayoutT(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
    }

    /// @brief Adds a widget to the rack under a unique ID.
    void add(const ID& id, WidgetT* widget, int stretch = 0) {
        m_map.insert(id, widget);
        m_layout->addWidget(widget, stretch);
    }

    /// @brief Retrieves a widget by ID.
    WidgetT* get(const ID& id) const { return m_map.value(id, nullptr); }

    /// @brief Removes and deletes all widgets from the rack.
    void clear() {
        qDeleteAll(m_map);
        m_map.clear();
    }

    /// @brief Direct access to the underlying layout.
    LayoutT* layout() const { return m_layout; }

    bool isEmpty() const { return m_map.isEmpty(); }
    QList<ID> ids() const { return m_map.keys(); }

private:
    LayoutT* m_layout;
    QMap<ID, WidgetT*> m_map;
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_WIDGETRACK_H
