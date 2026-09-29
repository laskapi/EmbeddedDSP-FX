#include "SpectrumPanel.h"
#include "SpectrumView.h"
#include "AppController.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QResizeEvent>

namespace Host::UI {

SpectrumPanel::SpectrumPanel(std::shared_ptr<AppController> controller, QWidget *parent)
    : QFrame(parent)
    , m_appController(controller) 
{
    setObjectName("SpectrumPanel");
    
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(0);

    m_leftColumn = new QWidget(this);
    m_leftColumn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    auto* leftLayout = new QVBoxLayout(m_leftColumn);
    leftLayout->setContentsMargins(0, 0, 0, SpectrumView::AXIS_BOTTOM_MARGIN); 
    leftLayout->setSpacing(4);
    
    m_zoomIcon = new QLabel(QString(QChar(0x2315)), this);
    m_zoomIcon->setObjectName("SpectrumZoomIcon");
    m_zoomIcon->setAlignment(Qt::AlignCenter);
    
    m_dbSlider = new QSlider(Qt::Vertical, this);
    m_dbSlider->setObjectName("SpectrumSensitivitySlider");
    m_dbSlider->setRange(-120, -40);
    m_dbSlider->setValue(-100);

    leftLayout->addWidget(m_zoomIcon);
    leftLayout->addWidget(m_dbSlider, 1);
    
    m_spectrumView = new SpectrumView(m_appController, this);
    
    m_rightColumn = new QWidget(this);
    m_rightColumn->setAttribute(Qt::WA_TransparentForMouseEvents);

    mainLayout->addWidget(m_leftColumn, 0);
    mainLayout->addWidget(m_spectrumView, 1);
    mainLayout->addWidget(m_rightColumn, 0);
    
    connect(m_dbSlider, &QSlider::valueChanged, m_spectrumView, [this](int val){
        m_spectrumView->setDbRange(static_cast<float>(val), 0.0f);
    });
}

void SpectrumPanel::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    if (m_leftColumn && m_rightColumn) {
        m_rightColumn->setFixedWidth(m_leftColumn->width());
    }
}

} // namespace Host::UI
