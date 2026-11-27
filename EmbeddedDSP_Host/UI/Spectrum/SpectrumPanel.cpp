#include "UI/Spectrum/SpectrumPanel.h"
#include "UI/Spectrum/SpectrumPlotHost.h"
#include "UI/Spectrum/SpectrumPlotView.h"
#include "UI/Spectrum/SpectrumAxisX.h"
#include "UI/Spectrum/SpectrumAxisY.h"
#include "UI/UiObjectNames.h"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <algorithm>

namespace Host::UI::Spectrum {

SpectrumPanel::SpectrumPanel(Core::AppController& controller, QWidget *parent)
    : QWidget(parent)
{
    auto* mainGrid = new QGridLayout(this);
    mainGrid->setContentsMargins(0, 0, 0, 0);
    mainGrid->setSpacing(0);

    m_zoomIcon = new QLabel(QString(QChar(0x2315)), this);
    m_zoomIcon->setObjectName(Host::UI::UiObjectNames::kSpectrumZoomIcon);
    m_zoomIcon->setAlignment(Qt::AlignCenter);

    m_dbSlider = new QSlider(Qt::Vertical, this);
    m_dbSlider->setObjectName(Host::UI::UiObjectNames::kSpectrumSensitivitySlider);
    m_dbSlider->setRange(-120, -40);
    m_dbSlider->setValue(-100);
    m_dbSlider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_dbUnitLabel = new QLabel(QStringLiteral("dB"), this);
    m_dbUnitLabel->setObjectName(Host::UI::UiObjectNames::kSpectrumUnitLabel);
    m_dbUnitLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_dbUnitLabel->ensurePolished();
    m_dbUnitLabel->setFixedWidth(
        m_dbUnitLabel->fontMetrics().horizontalAdvance(QStringLiteral("EQ dB")) + 4);

    m_spectrumPlotHost = new SpectrumPlotHost(controller, this);

    m_axisY = new SpectrumAxisY(this);

    m_axisX = new SpectrumAxisX(this);
    m_axisX->setObjectName(Host::UI::UiObjectNames::kSpectrumAxisX);

    mainGrid->addWidget(m_zoomIcon,    0, 0);
    mainGrid->addWidget(m_dbUnitLabel, 0, 1);

    mainGrid->addWidget(m_dbSlider,         1, 0);
    mainGrid->addWidget(m_axisY,            1, 1);
    mainGrid->addWidget(m_spectrumPlotHost, 1, 2);

    mainGrid->addWidget(m_axisX, 2, 2);

    mainGrid->setColumnMinimumWidth(
        1, std::max(m_dbUnitLabel->width(), m_axisY->sizeHint().width()));
    mainGrid->setColumnStretch(2, 1);
    mainGrid->setRowStretch(1, 1);

    connect(m_dbSlider, &QSlider::valueChanged, this, [this](int val) {
        const float minDb = static_cast<float>(val);
        m_spectrumPlotHost->plotView()->setDbRange(minDb, 0.0f);
        m_axisY->setFftDbRange(minDb, 0.0f);
    });
    m_axisY->setFftDbRange(static_cast<float>(m_dbSlider->value()), 0.0f);
}

void SpectrumPanel::setEqCurves(const QVector<Host::Models::EqCurveState>& curves) {
    m_spectrumPlotHost->plotView()->setEqCurves(curves);

    const bool eqMode = !curves.isEmpty();
    m_axisY->setEqMode(eqMode);
    m_dbUnitLabel->setText(eqMode ? QStringLiteral("EQ dB") : QStringLiteral("dB"));
    m_dbSlider->setEnabled(!eqMode);

    if (!eqMode) {
        const float minDb = static_cast<float>(m_dbSlider->value());
        m_spectrumPlotHost->plotView()->setDbRange(minDb, 0.0f);
        m_axisY->setFftDbRange(minDb, 0.0f);
    }
}

} // namespace Host::UI::Spectrum
