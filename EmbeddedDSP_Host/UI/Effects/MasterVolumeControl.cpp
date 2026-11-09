#include "UI/Effects/MasterVolumeControl.h"
#include "Core/AppController.h"
#include "UI/UiObjectNames.h"

#include <QLabel>
#include <QSizePolicy>
#include <QSlider>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace Host::UI::Effects {

MasterVolumeControl::MasterVolumeControl(Host::Core::AppController& controller, QWidget* parent)
    : QWidget(parent)
    , m_appController(controller)
{
    setObjectName(Host::UI::UiObjectNames::kMasterVolumeBox);
    setAttribute(Qt::WA_StyledBackground, true);

    m_titleLabel = new QLabel(tr("MASTER"), this);
    m_titleLabel->setObjectName(Host::UI::UiObjectNames::kMasterVolumeTitle);
    m_titleLabel->setAlignment(Qt::AlignHCenter);

    m_slider = new QSlider(Qt::Vertical, this);
    m_slider->setObjectName(Host::UI::UiObjectNames::kMasterVolumeSlider);
    m_slider->setRange(0, kSliderResolution);
    m_slider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_valueLabel = new QLabel(QStringLiteral("--"), this);
    m_valueLabel->setObjectName(Host::UI::UiObjectNames::kMasterVolumeValue);
    m_valueLabel->setAlignment(Qt::AlignHCenter);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 10, 8, 10);
    layout->setSpacing(6);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_slider, 1, Qt::AlignHCenter);
    layout->addWidget(m_valueLabel);

    setFixedWidth(72);

    connect(m_slider, &QSlider::valueChanged, this, &MasterVolumeControl::onSliderMoved);
    connect(&m_appController, &Host::Core::AppController::masterVolumeUpdated,
            this, &MasterVolumeControl::applyMasterVolume);
}

void MasterVolumeControl::applyMasterVolume(float gain) {
    m_slider->blockSignals(true);
    setSliderFromGain(gain);
    m_slider->blockSignals(false);
}

void MasterVolumeControl::onSliderMoved(int pos) {
    const float gain = static_cast<float>(pos) / static_cast<float>(kSliderResolution);
    m_valueLabel->setText(QString::number(gain, 'f', 2));
    m_appController.setMasterVolume(gain);
}

void MasterVolumeControl::setSliderFromGain(float gain) {
    const float clamped = std::clamp(gain, 0.0f, 1.0f);
    const int pos = static_cast<int>(
        std::lround(clamped * static_cast<float>(kSliderResolution)));
    m_slider->setValue(pos);
    m_valueLabel->setText(QString::number(clamped, 'f', 2));
}

} // namespace Host::UI::Effects
