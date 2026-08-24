#include "EffectsRack.h"
#include <QHBoxLayout>
#include <algorithm>

EffectsRack::EffectsRack(QWidget *parent) : QWidget(parent) {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 1. Find the maximum number of parameters across all effects using metadata
    uint8_t maxParams = 0;
    for (const auto &meta : EffectParams::kMetadata) {
        if (meta.paramCount > maxParams) {
            maxParams = meta.paramCount;
        }
    }

    // 2. Estimate required height based on UI elements
    // Header/Title + Combo (~60px) + Bypass Button (~40px) + Layout Spacing
    // Plus ~50px per parameter (Label + Slider pair)
    int estimatedHeight = 110 + (maxParams * 50);

    // 3. Create slots based on global configuration
    for (uint8_t i = 0; i < static_cast<uint8_t>(EffectParams::kMaxSlots); ++i) {
        auto *slot = new EffectWidget(i, this);
        
        // Ensure consistent height regardless of the currently selected effect
        slot->setMinimumHeight(estimatedHeight);
        
        m_slots.push_back(slot);
        
        // Add widget with stretch factor 1 to ensure equal width for all slots
        layout->addWidget(slot, 1);
        
        // Connect widget signal to rack signal for central packet handling
        connect(slot, &EffectWidget::controlPacketReady, 
                this, &EffectsRack::controlPacketReady);
    }
}