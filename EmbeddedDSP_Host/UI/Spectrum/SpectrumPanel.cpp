#include "SpectrumPanel.h"
#include "SpectrumView.h"
#include "SpectrumAxisX.h"
#include "SpectrumAxisY.h"
#include "SpectrumCommon.h"
#include "AppController.h"
#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>

namespace Host::UI::Spectrum {

SpectrumPanel::SpectrumPanel(std::shared_ptr<AppController> controller, QWidget *parent)
    : QFrame(parent)
    , m_appController(controller) 
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

    m_axisY = new SpectrumAxisY(this);
    
    m_spectrumView = new SpectrumView(m_appController, this);
    m_axisX = new SpectrumAxisX(this);
    m_axisX->setObjectName("SpectrumAxisX");

    // Welcome label as a child of spectrum view for floating effect
    m_welcomeLabel = new QLabel(tr("Please connect your EmbeddedDSP device..."), m_spectrumView);
    m_welcomeLabel->setObjectName("welcomeLabel");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    
    // Simple layout to center the label inside the view
    auto* viewLayout = new QVBoxLayout(m_spectrumView);
    viewLayout->addWidget(m_welcomeLabel);
    m_welcomeLabel->show();

    mainGrid->addWidget(m_zoomIcon,    0, 0);
    mainGrid->addWidget(m_dbUnitLabel, 0, 1);

    mainGrid->addWidget(m_dbSlider,     1, 0);
    mainGrid->addWidget(m_axisY,        1, 1);
    mainGrid->addWidget(m_spectrumView, 1, 2);

    mainGrid->addWidget(m_axisX,        2, 2);

    mainGrid->setColumnStretch(2, 1); 
    mainGrid->setRowStretch(1, 1);    

    connect(m_dbSlider, &QSlider::valueChanged, this, [this](int val){
        float minDb = static_cast<float>(val);
        m_spectrumView->setDbRange(minDb, 0.0f);
        m_axisY->setDbRange(minDb, 0.0f);
    });
}

void SpectrumPanel::setWelcomeVisible(bool visible) {
    if (m_welcomeLabel) {
        m_welcomeLabel->setVisible(visible);
    }
}

} // namespace Host::UI::Spectrum
