#include "SpectrumPanel.h"
#include "SpectrumPlotHost.h"
#include "SpectrumPlotView.h"
#include "SpectrumAxisX.h"
#include "SpectrumAxisY.h"
#include "SpectrumCommon.h"
#include <QGridLayout>
#include <QSlider>
#include <QLabel>

namespace Host::UI::Spectrum {

SpectrumPanel::SpectrumPanel(Core::AppController& controller, QWidget *parent)
    : QFrame(parent)
{
    auto* mainGrid = new QGridLayout(this);
    mainGrid->setContentsMargins(0, 0, 0, 0);
    mainGrid->setSpacing(0);

    m_zoomIcon = new QLabel(QString(QChar(0x2315)), this);
    m_zoomIcon->setObjectName("SpectrumZoomIcon");
    m_zoomIcon->setAlignment(Qt::AlignCenter);

    m_dbSlider = new QSlider(Qt::Vertical, this);
    m_dbSlider->setObjectName("SpectrumSensitivitySlider");
    m_dbSlider->setRange(-120, -40);
    m_dbSlider->setValue(-100);
    m_dbSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_dbUnitLabel = new QLabel("dB", this);
    m_dbUnitLabel->setObjectName("SpectrumUnitLabel");
    m_dbUnitLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    m_spectrumPlotHost = new SpectrumPlotHost(controller, this);

    m_axisY = new SpectrumAxisY(this);

    m_axisX = new SpectrumAxisX(this);
    m_axisX->setObjectName("SpectrumAxisX");

    mainGrid->addWidget(m_zoomIcon,    0, 0);
    mainGrid->addWidget(m_dbUnitLabel, 0, 1);

    mainGrid->addWidget(m_dbSlider,         1, 0);
    mainGrid->addWidget(m_axisY,            1, 1);
    mainGrid->addWidget(m_spectrumPlotHost, 1, 2);

    mainGrid->addWidget(m_axisX, 2, 2);

    mainGrid->setColumnStretch(2, 1);
    mainGrid->setRowStretch(1, 1);

    connect(m_dbSlider, &QSlider::valueChanged, this, [this](int val) {
        const float minDb = static_cast<float>(val);
        m_spectrumPlotHost->plotView()->setDbRange(minDb, 0.0f);
        m_axisY->setDbRange(minDb, 0.0f);
    });
}

} // namespace Host::UI::Spectrum
