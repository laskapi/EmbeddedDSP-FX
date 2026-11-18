#ifndef EMBEDDEDDSP_HOST_ABSTRACTRACK_H
#define EMBEDDEDDSP_HOST_ABSTRACTRACK_H

#include <QLayout>
#include <QList>
#include <QWidget>
#include <algorithm>
#include <concepts>
#include "UI/Effects/AbstractRackItem.h"

namespace Host::UI::Effects {

template<typename T>
concept IsRackItem = std::derived_from<T, AbstractRackItem>;

template <IsRackItem T, typename LayoutT = QVBoxLayout>
class AbstractRack : public QWidget {
public:
    explicit AbstractRack(QWidget* parent = nullptr) : QWidget(parent) {
        m_layout = new LayoutT(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(10);
    }

    LayoutT* layout() const { return m_layout; }

    /// @brief Factory method to create and register a new rack item.
    template <typename... Args>
    T* createItem(Args&&... args) {
        int nextIndex = m_items.size();
        T* item = new T(nextIndex, std::forward<Args>(args)...);

        m_items.append(item);
        m_layout->addWidget(item);

        return item;
    }

protected:
    /// @brief Reorders @c m_items and refreshes indices (layout refresh is subclass duty).
    void moveItem(int from, int to) {
        if (from < 0 || from >= m_items.size() || to < 0 || to >= m_items.size() || from == to) {
            return;
        }
        m_items.move(from, to);
        for (int i = 0; i < m_items.size(); ++i) {
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

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_ABSTRACTRACK_H
