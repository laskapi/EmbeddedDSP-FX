#ifndef EMBEDDEDDSP_HOST_SYNCHRONIZEDRACK_H
#define EMBEDDEDDSP_HOST_SYNCHRONIZEDRACK_H

#include <QFrame>
#include <QLayout>
#include <QList>
#include <concepts>
#include "SynchronizedWidget.h"

namespace Host::UI {

/**
 * @brief C++20 Concept to ensure T is a proper SynchronizedWidget.
 */
template<typename T>
concept IsSynchronizedWidget = std::derived_from<T, SynchronizedWidget>;

/**
 * @brief Generic engine for managing a synchronized list of widgets.
 * Ensures that the memory vector, the UI layout, and the widgets' internal IDs
 * are always in perfect sync.
 */
template <IsSynchronizedWidget T, typename LayoutT = QVBoxLayout>
class SynchronizedRack : public QFrame {
public:
    explicit SynchronizedRack(QWidget* parent = nullptr) : QFrame(parent) {
        m_layout = new LayoutT(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
    }

    /// @brief Direct access to the underlying layout (standard Qt practice).
    LayoutT* layout() const { return m_layout; }

protected:
    void addSynchronized(T* item, int stretch = 0) {
        if (!item) return;
        item->setLogicalIndex(m_items.size());
        m_items.append(item);
        m_layout->addWidget(item, stretch);
    }

    void moveSynchronized(int from, int to) {
        if (from < 0 || from >= m_items.size() || to < 0 || to >= m_items.size()) return;
        
        m_items.move(from, to);
        m_layout->insertWidget(to, m_items[to]);

        for (int i = std::min(from, to); i <= std::max(from, to); ++i) {
            m_items[i]->setLogicalIndex(i);
        }
    }

    T* getSynchronized(int index) const {
        return (index >= 0 && index < m_items.size()) ? m_items[index] : nullptr;
    }

    void clearSynchronized() {
        qDeleteAll(m_items);
        m_items.clear();
    }

    bool isRackEmpty() const { return m_items.isEmpty(); }
    int synchronizedCount() const { return m_items.size(); }

private:
    LayoutT* m_layout;
    QList<T*> m_items; // The "Shadow List" for O(1) performance
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_SYNCHRONIZEDRACK_H
