#ifndef EMBEDDEDDSP_HOST_SYNCHRONIZEDWIDGET_H
#define EMBEDDEDDSP_HOST_SYNCHRONIZEDWIDGET_H

#include <QWidget>

namespace Host::UI {

/**
 * @brief Base class for widgets that are part of a SynchronizedRack.
 * Keeps track of its own logical index (ID) for efficient signal bubbling.
 */
class SynchronizedWidget : public QWidget {
    Q_OBJECT
public:
    explicit SynchronizedWidget(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual ~SynchronizedWidget() override = default;

    /// @brief Sets the logical index of the widget. Called by SynchronizedRack.
    void setLogicalIndex(int index) { m_logicalIndex = index; }

    /// @brief Gets the current logical index.
    [[nodiscard]] int logicalIndex() const { return m_logicalIndex; }

protected:
    int m_logicalIndex{-1};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_SYNCHRONIZEDWIDGET_H
