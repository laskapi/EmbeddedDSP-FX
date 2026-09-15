#ifndef EMBEDDEDDSP_HOST_EFFECTWIDGET_H
#define EMBEDDEDDSP_HOST_EFFECTWIDGET_H

#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QList>
#include <Protocol/ControlPacket.h>

struct ParamInfo {
    QString name;
    float min;
    float max;
    float defaultValue;
};

struct EffectMetadata {
    uint8_t id;
    QString name;
    QList<ParamInfo> params;
};

class EffectWidget : public QGroupBox {
    Q_OBJECT
public:
    explicit EffectWidget(uint8_t slotId, QWidget *parent = nullptr);
    void updateFromPacket(const Protocol::ControlPacket &packet);
    void setAvailableEffects(const QMap<int, EffectMetadata> &effects);

signals:
    void controlPacketReady(const Protocol::ControlPacket &packet);

private slots:
    void onTypeChanged(int index);
    void onSliderMoved(int value);
    void onBypassToggled();

private:
    void setupUi();
    void buildParamsUi(uint8_t effectId);

    uint8_t m_slotId;
    QComboBox *m_typeCombo;
    QWidget *m_paramsContainer;
    QVBoxLayout *m_paramsLayout;
    QPushButton *m_bypassBtn;

    QMap<int, EffectMetadata> m_availableEffects;
    
    struct SliderInfo {
        uint8_t paramId;
        ParamInfo desc;
        QLabel* valueLabel;
    };
    QMap<QSlider*, SliderInfo> m_sliderMap;
};

#endif // EMBEDDEDDSP_HOST_EFFECTWIDGET_H
