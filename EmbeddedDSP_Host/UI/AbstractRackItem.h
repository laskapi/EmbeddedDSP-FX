#ifndef EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H
#define EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H

#include <QWidget>

namespace Host::UI {

/// @brief Base class for widgets that are part of an AbstractRack.
class AbstractRackItem : public QWidget {
    Q_OBJECT
public:
    virtual ~AbstractRackItem() override = default;

    /// @brief Gets the current index.
    [[nodiscard]] int index() const { return m_index; }

    /// @brief Sets the index of the widget within the rack.
    void setIndex(int index) { m_index = index; }

protected:
    /// @brief Protected constructor requires an index.
    explicit AbstractRackItem(int index, QWidget* parent = nullptr) 
        : QWidget(parent), m_index(index) {}

    int m_index{-1};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H
