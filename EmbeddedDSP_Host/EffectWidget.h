#ifndef EMBEDDEDDSP_HOST_EFFECTWIDGET_H
#define EMBEDDEDDSP_HOST_EFFECTWIDGET_H

#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include "../EmbeddedDSP_Firmware/App/Protocol/EffectParams.h"
#include "../EmbeddedDSP_Firmware/App/Protocol/ControlPacket.h"

class EffectWidget : public QGroupBox {
    Q_OBJECT
public:
    explicit EffectWidget(uint8_t slotId, QWidget *parent = nullptr);

signals:
    void controlPacketReady(const ControlPacket::ControlPacket &packet);

private slots:
    void onTypeChanged(int index);
    void onSliderMoved(int value);
    void onBypassToggled();

private:
    void setupUi();
    void buildParamsUi(const EffectParams::EffectMetadata &meta);

    uint8_t m_slotId;
    QComboBox *m_typeCombo;
    QWidget *m_paramsContainer;
    QVBoxLayout *m_paramsLayout;
    QPushButton *m_bypassBtn;

    struct SliderInfo {
        uint8_t paramId;
        EffectParams::ParamDesc desc;
    };
    QMap<QSlider*, SliderInfo> m_sliderMap;
};

#endif // EMBEDDEDDSP_HOST_EFFECTWIDGET_H