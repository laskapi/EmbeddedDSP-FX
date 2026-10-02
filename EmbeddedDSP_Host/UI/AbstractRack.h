#ifndef EMBEDDEDDSP_HOST_ABSTRACTRACK_H
#define EMBEDDEDDSP_HOST_ABSTRACTRACK_H

#include <QFrame>
#include <QLayout>
#include <QList>
#include <concepts>
#include "AbstractRackItem.h"

namespace Host::UI {

/**
 * @brief C++20 Concept to ensure T is a proper AbstractRackItem.
 */
template<typename T>
concept IsRackItem = std::derived_from<T, AbstractRackItem>;

/**
 * @brief Generic engine for managing a list of widgets in a "Rack" layout.
 * Ensures that the memory vector, the UI layout, and the widgets' internal IDs
 * are always in perfect sync.
 */
template <IsRackItem T, typename LayoutT = QVBoxLayout>
class AbstractRack : public QFrame {
public:
    explicit AbstractRack(QWidget* parent = nullptr) : QFrame(parent) {
        m_layout = new LayoutT(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
    }

    LayoutT* layout() const { return m_layout; }

protected:
    void addItem(T* item, int stretch = 0) {
        if (!item) return;
        item->setIndex(m_items.size());
        m_items.append(item);
        m_layout->addWidget(item, stretch);
    }

    void moveItem(int from, int to) {
        if (from < 0 || from >= m_items.size() || to < 0 || to >= m_items.size()) return;
        
        m_items.move(from, to);
        m_layout->insertWidget(to, m_items[to]);

        for (int i = std::min(from, to); i <= std::max(from, to); ++i) {
            m_items[i]->setIndex(i);
        }
    }

    T* getItem(int index) const {
        return (index >= 0 && index < m_items.size()) ? m_items[index] : nullptr;
    }

    void clearRack() {
        qDeleteAll(m_items);
        m_items.clear();
    }

    bool isRackEmpty() const { return m_items.isEmpty(); }
    int itemCount() const { return m_items.size(); }

private:
    LayoutT* m_layout;
    QList<T*> m_items;
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_ABSTRACTRACK_H
