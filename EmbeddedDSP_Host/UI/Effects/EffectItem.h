#ifndef EMBEDDEDDSP_HOST_EFFECTITEM_H
#define EMBEDDEDDSP_HOST_EFFECTITEM_H

#include "UI/Effects/AbstractRackItem.h"
#include "Models/DeviceModel.h"
#include "Models/EqCurveState.h"
#include <QMap>
#include <optional>

class QComboBox;
class QPushButton;
class QVBoxLayout;
class QWidget;

namespace Host::UI::Effects {

class EqChrome;
class EqCurveColorPool;
class ParameterRack;

/// @brief UI component for a single effect in the chain.
class EffectItem : public AbstractRackItem {
    Q_OBJECT
public:
    explicit EffectItem(int index, QWidget *parent = nullptr);

    [[nodiscard]] int slotIndex() const { return m_index; }

    void applyEffectType(int effectTypeId);
    void applyBypass(bool bypassed);
    void applyParameter(int paramIndex, float value);

    /// @brief Sets the list of available effects for the combo box.
    void setAvailableEffects(const QMap<int, Host::Models::EffectSpec> &availableEffects);

    /// @brief Enables/disables one effect entry in the type combo (e.g. exclusive Delay).
    void setEffectChoiceEnabled(int effectTypeId, bool enabled);

    /// @brief Reserves ParameterRack capacity and locks this slot's total height.
    void lockHeightForParamCapacity(int maxParamCount);

    /// @brief Shared palette for sticky EQ curve colors (owned by EffectRack).
    void setEqCurveColorPool(EqCurveColorPool* pool);

    /// @brief Visible EQ curve DTO when Curve is on and slot is not bypassed.
    [[nodiscard]] std::optional<Host::Models::EqCurveState> eqCurveState() const;

signals:
    void parameterChanged(int slotIdx, int paramIdx, float newValue);
    void typeChanged(int slotIdx, int effectTypeId);
    void bypassToggled(int slotIdx, bool bypassed);
    void eqCurveDirty();

private slots:
    void onTypeChanged(int index);
    void onBypassToggled();

private:
    void setupUi();
    void setupEffect(int effectTypeId);
    void setBypassVisible(bool visible);

    /// @brief @p force true for Show toggle / EQ enter-leave; omit / false = only if curve is visible.
    void requestEqCurvePublish(bool force = false);

    QWidget* m_dragHandle{nullptr};
    QComboBox *m_typeCombo{nullptr};
    ParameterRack *m_parameterRack{nullptr};
    EqChrome* m_eqChrome{nullptr};
    QPushButton *m_bypassBtn{nullptr};
    QVBoxLayout *m_mainLayout{nullptr};

    QMap<int, Host::Models::EffectSpec> m_availableEffects;
    int m_effectTypeId{0};
};

} // namespace Host::UI::Effects

#endif // EMBEDDEDDSP_HOST_EFFECTITEM_H
