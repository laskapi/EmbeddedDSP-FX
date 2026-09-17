#ifndef EMBEDDEDDSP_HOST_PARAMCONTROL_H
#define EMBEDDEDDSP_HOST_PARAMCONTROL_H

#include <QWidget>
#include "EffectSpec.h"

class QLabel;
class QSlider;

namespace UI {

class ParamControl : public QWidget {
    Q_OBJECT
public:
    explicit ParamControl(uint8_t paramId, const Host::ParamSpec &spec, QWidget *parent = nullptr);
    void setValue(float value);

signals:
    void valueChanged(uint8_t paramId, float newValue);

private slots:
    void onSliderMoved(int pos);

private:
    void updateLabel(float value);

    uint8_t m_paramId;
    Host::ParamSpec m_spec;
    QLabel *m_label = nullptr;
    QSlider *m_slider = nullptr;
};

} // namespace UI

#endif // EMBEDDEDDSP_HOST_PARAMCONTROL_H
