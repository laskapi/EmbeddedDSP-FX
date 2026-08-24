#include "EffectWidget.h"

EffectWidget::EffectWidget(uint8_t slotId, QWidget *parent)
    : QGroupBox(parent), m_slotId(slotId) {
    setTitle(tr("Slot %1").arg(slotId + 1));
    setupUi();
}

void EffectWidget::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    m_typeCombo = new QComboBox(this);
    for (const auto &meta : EffectParams::kMetadata) {
        m_typeCombo->addItem(meta.name, static_cast<int>(meta.type));
    }
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

void EffectWidget::onTypeChanged(int index) {
    const auto &meta = EffectParams::kMetadata[index];
    ControlPacket::ControlPacket pkt;
    pkt.command = ControlPacket::Command::SetEffectType;
    pkt.slotId = m_slotId;
    pkt.paramId = static_cast<uint8_t>(meta.type);
    pkt.crc = ControlPacket::ControlPacket::calculateCRC(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt) - 1);
    emit controlPacketReady(pkt);
    buildParamsUi(meta);
}

void EffectWidget::buildParamsUi(const EffectParams::EffectMetadata &meta) {
    m_sliderMap.clear();
    QLayoutItem *item;
    while ((item = m_paramsLayout->takeAt(0)) != nullptr) {
        delete item->widget(); delete item;
    }
    for (uint8_t i = 0; i < meta.paramCount; ++i) {
        const auto &desc = meta.params[i];
        auto *label = new QLabel(QString("%1: %2 %3").arg(desc.name).arg(desc.defaultValue).arg(desc.unit), this);
        auto *slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 1000);
        float range = desc.maxValue - desc.minValue;
        slider->setValue(static_cast<int>((desc.defaultValue - desc.minValue) / range * 1000.0f));
        m_sliderMap[slider] = { desc.paramId, desc };
        connect(slider, &QSlider::valueChanged, this, &EffectWidget::onSliderMoved);
        m_paramsLayout->addWidget(label);
        m_paramsLayout->addWidget(slider);
    }
}

void EffectWidget::onSliderMoved(int value) {
    auto *slider = qobject_cast<QSlider*>(sender());
    if (!slider || !m_sliderMap.contains(slider)) return;
    const auto &info = m_sliderMap[slider];
    float realVal = info.desc.minValue + (value / 1000.0f) * (info.desc.maxValue - info.desc.minValue);
    int idx = m_paramsLayout->indexOf(slider);
    if (idx > 0) {
        auto *label = qobject_cast<QLabel*>(m_paramsLayout->itemAt(idx - 1)->widget());
        if (label) label->setText(QString("%1: %2 %3").arg(info.desc.name).arg(realVal, 0, 'f', 2).arg(info.desc.unit));
    }
    ControlPacket::ControlPacket pkt;
    pkt.command = ControlPacket::Command::SetParam;
    pkt.slotId = m_slotId; pkt.paramId = info.paramId; pkt.setValue(realVal);
    pkt.crc = ControlPacket::ControlPacket::calculateCRC(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt) - 1);
    emit controlPacketReady(pkt);
}

void EffectWidget::onBypassToggled() {
    ControlPacket::ControlPacket pkt;
    pkt.command = ControlPacket::Command::BypassToggle;
    pkt.slotId = m_slotId;
    pkt.crc = ControlPacket::ControlPacket::calculateCRC(reinterpret_cast<uint8_t*>(&pkt), sizeof(pkt) - 1);
    emit controlPacketReady(pkt);
}