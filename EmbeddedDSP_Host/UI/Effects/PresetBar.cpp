#include "UI/Effects/PresetBar.h"
#include "Core/AppController.h"
#include "UI/UiObjectNames.h"

#include <QHBoxLayout>
#include <QPushButton>

namespace Host::UI::Effects {

PresetBar::PresetBar(Host::Core::AppController& controller, QWidget* parent)
    : QWidget(parent)
    , m_appController(controller)
{
    setObjectName(QLatin1String(UiObjectNames::kPresetBar));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 12);
    layout->setSpacing(4);

    for (int i = 0; i < Protocol::kUserPresetCount; ++i) {
        // UI label 1..N maps to wire preset id i (0..N-1).
        auto* btn = new QPushButton(QString::number(i + 1), this);
        btn->setObjectName(QLatin1String(UiObjectNames::kPresetButton));
        btn->setCheckable(true);
        btn->setFixedWidth(28);
        btn->setEnabled(false);
        m_presetButtons[static_cast<std::size_t>(i)] = btn;
        layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            onPresetClicked(i);
        });
    }

    m_saveButton = new QPushButton(tr("Save"), this);
    m_saveButton->setObjectName(QLatin1String(UiObjectNames::kPresetSaveButton));
    m_saveButton->setEnabled(false);
    layout->addWidget(m_saveButton);
    layout->addStretch();

    connect(m_saveButton, &QPushButton::clicked, this, &PresetBar::onSaveClicked);
    connect(&m_appController, &Host::Core::AppController::activePresetUpdated,
            this, &PresetBar::onActivePresetUpdated);
    connect(&m_appController, &Host::Core::AppController::connectionChanged,
            this, &PresetBar::onConnectionChanged);

    setSelectedPreset(0);
}

void PresetBar::setSelectedPreset(int presetId) {
    if (presetId < 0 || presetId >= Protocol::kUserPresetCount) {
        return;
    }
    m_selectedPreset = presetId;
    for (int i = 0; i < Protocol::kUserPresetCount; ++i) {
        m_presetButtons[static_cast<std::size_t>(i)]->setChecked(i == presetId);
    }
}

void PresetBar::setEnabledState(bool enabled) {
    for (auto* btn : m_presetButtons) {
        btn->setEnabled(enabled);
    }
    m_saveButton->setEnabled(enabled);
}

void PresetBar::onActivePresetUpdated(int presetId) {
    setSelectedPreset(presetId);
}

void PresetBar::onConnectionChanged(bool connected, const QString& /*portName*/) {
    m_connected = connected;
    setEnabledState(connected);
    if (!connected) {
        setSelectedPreset(0);
    }
}

void PresetBar::onPresetClicked(int presetId) {
    if (!m_connected) {
        return;
    }
    setSelectedPreset(presetId);
    m_appController.loadPreset(presetId);
}

void PresetBar::onSaveClicked() {
    if (!m_connected) {
        return;
    }
    m_appController.savePreset(m_selectedPreset);
}

} // namespace Host::UI::Effects
