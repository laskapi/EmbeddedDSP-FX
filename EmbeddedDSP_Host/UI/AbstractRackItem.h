#ifndef EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H
#define EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H

#include <QWidget>

namespace Host::UI {

/**
 * @brief Base class for widgets that are part of an AbstractRack.
 * Keeps track of its own index (ID) for efficient signal bubbling.
 */
class AbstractRackItem : public QWidget {
    Q_OBJECT
public:
    virtual ~AbstractRackItem() override = default;

    /// @brief Sets the index of the widget within the rack. Called by AbstractRack.
    void setIndex(int index) { m_index = index; }

    /// @brief Gets the current index.
    [[nodiscard]] int index() const { return m_index; }

protected:
    /// @brief Protected constructor prevents direct instantiation.
    explicit AbstractRackItem(QWidget* parent = nullptr) : QWidget(parent) {}

    int m_index{-1};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_ABSTRACTRACKITEM_H
