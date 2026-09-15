#include "EffectWidget.h"
#include <QDebug>

EffectWidget::EffectWidget(uint8_t slotId, QWidget *parent)
    : QGroupBox(parent), m_slotId(slotId) {
    setTitle(tr("Slot %1").arg(slotId + 1));
    setupUi();
}

void EffectWidget::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    m_typeCombo = new QComboBox(this);
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectWidget::onTypeChanged);

    m_paramsContainer = new QWidget(this);
    m_paramsLayout = new QVBoxLayout(m_paramsContainer);
    m_paramsLayout->setContentsMargins(0, 0, 0, 0);

    m_bypassBtn = new QPushButton(tr("Bypass"), this);
    m_bypassBtn->setCheckable(true);
    connect(m_bypassBtn, &QPushButton::toggled, this, &EffectWidget::onBypassToggled);

    mainLayout->addWidget(m_typeCombo);
    mainLayout->addWidget(m_paramsContainer, 1);
    mainLayout->addWidget(m_bypassBtn);
}

void EffectWidget::setAvailableEffects(const QMap<int, EffectMetadata> &effects) {
    m_availableEffects = effects;
    
    m_typeCombo->blockSignals(true);
    m_typeCombo->clear();
    for (const auto &meta : m_availableEffects) {
        m_typeCombo->addItem(meta.name, meta.id);
    }
    m_typeCombo->blockSignals(false);
}

void EffectWidget::onTypeChanged(int index) {
    uint8_t effectId = static_cast<uint8_t>(m_typeCombo->itemData(index).toInt());
    
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetEffectType;
    pkt.slotId = m_slotId;
    pkt.paramId = effectId;
    pkt.applyCRC();
    
    emit controlPacketReady(pkt);
    buildParamsUi(effectId);
}

void EffectWidget::buildParamsUi(uint8_t effectId) {
    m_sliderMap.clear();
    QLayoutItem *child;
    while ((child = m_paramsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    if (!m_availableEffects.contains(effectId)) return;
    const auto &meta = m_availableEffects[effectId];

    for (int i = 0; i < meta.params.size(); ++i) {
        const auto &p = meta.params[i];
        auto *label = new QLabel(QString("%1: %2").arg(p.name).arg(p.defaultValue, 0, 'f', 2), this);
        auto *slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 1000);

        float range = p.max - p.min;
        int sliderVal = (range > 0) ? static_cast<int>((p.defaultValue - p.min) / range * 1000.0f) : 0;
        slider->setValue(sliderVal);

        m_sliderMap[slider] = { static_cast<uint8_t>(i), p, label };
        
        connect(slider, &QSlider::valueChanged, this, &EffectWidget::onSliderMoved);

        m_paramsLayout->addWidget(label);
        m_paramsLayout->addWidget(slider);
    }
    m_paramsLayout->addStretch(1);
}

void EffectWidget::updateFromPacket(const Protocol::ControlPacket &packet) {
    if (packet.command == Protocol::Command::SetEffectType) {
        int index = m_typeCombo->findData(packet.paramId);
        if (index != -1) {
            m_typeCombo->blockSignals(true);
            m_typeCombo->setCurrentIndex(index);
            m_typeCombo->blockSignals(false);
            buildParamsUi(packet.paramId);
        }
    } else if (packet.command == Protocol::Command::BypassToggle) {
        m_bypassBtn->blockSignals(true);
        m_bypassBtn->setChecked(packet.getValue() > 0.5f);
        m_bypassBtn->blockSignals(false);
    } else if (packet.command == Protocol::Command::SetParam) {
        for (auto it = m_sliderMap.begin(); it != m_sliderMap.end(); ++it) {
            if (it.value().paramId == packet.paramId) {
                auto &info = it.value();
                float range = info.desc.max - info.desc.min;
                float val = packet.getValue();
                int sliderVal = (range > 0) ? static_cast<int>((val - info.desc.min) / range * 1000.0f) : 0;
                
                it.key()->blockSignals(true);
                it.key()->setValue(sliderVal);
                it.key()->blockSignals(false);

                if (info.valueLabel) {
                    info.valueLabel->setText(QString("%1: %2").arg(info.desc.name).arg(val, 0, 'f', 2));
                }
                break;
            }
        }
    }
}

void EffectWidget::onSliderMoved(int value) {
    auto *slider = qobject_cast<QSlider*>(sender());
    if (!slider || !m_sliderMap.contains(slider)) return;
    
    auto &info = m_sliderMap[slider];
    float realVal = info.desc.min + (static_cast<float>(value) / 1000.0f) * (info.desc.max - info.desc.min);

    if (info.valueLabel) {
        info.valueLabel->setText(QString("%1: %2").arg(info.desc.name).arg(realVal, 0, 'f', 2));
    }

    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetParam;
    pkt.slotId = m_slotId; 
    pkt.paramId = info.paramId; 
    pkt.setValue(realVal);
    pkt.applyCRC();
    
    emit controlPacketReady(pkt);
}

void EffectWidget::onBypassToggled() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::BypassToggle;
    pkt.slotId = m_slotId;
    pkt.setValue(m_bypassBtn->isChecked() ? 1.0f : 0.0f);
    pkt.applyCRC();
    emit controlPacketReady(pkt);
}
