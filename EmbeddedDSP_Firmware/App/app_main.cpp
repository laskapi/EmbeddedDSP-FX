#include "app_main.h"
#include "DSP/FxChain.h"
#include "Audio/I2sDma.h"
#include "Audio/ProcessHalf.h"
#include "Comm/UsbRx.h"
#include "Comm/UsbTx.h"
#include "Comm/CommandDispatcher.h"

#include <cstdint>

namespace
{
    using namespace App;

    constexpr std::size_t kMaxControlPacketsPerLoop = 8;

    Audio::I2sDma i2sDma{};
    Dsp::FxChain fxChain{Protocol::SAMPLE_RATE_HZ};

    Comm::UsbRx usbRx{};
    Comm::UsbTx usbTx{};
    Comm::CommandDispatcher commandDispatcher{fxChain, usbTx};
}

extern "C" {
void on_cdc_bytes(const uint8_t* Buf, uint32_t Len)
{
    usbRx.onBytesReceived(Buf, Len);
}

void on_usb_host_gone(void)
{
    usbTx.setAudioStreamEnabled(false);
}

void on_i2s_dma_half(void)
{
    i2sDma.onHalfComplete();
}

void on_i2s_dma_full(void)
{
    i2sDma.onTransferComplete();
}

void app_main(void)
{
    APP_LOG("Firmware starting...");

    i2sDma.start();

    uint32_t lastTick = HAL_GetTick();

    while (true)
    {
        if (auto buffers = i2sDma.takeReadyBuffers())
        {
            if (usbTx.isAudioStreamEnabled()) {
                Protocol::AudioFrame& frame = usbTx.acquireAudioFrame();
                Audio::processHalf(buffers->rx, fxChain, buffers->tx, frame.samples);
                usbTx.commitAudioFrame(frame);
            } else {
                Audio::processHalf(buffers->rx, fxChain, buffers->tx);
            }
        }

        for (std::size_t n = 0; n < kMaxControlPacketsPerLoop; ++n) {
            auto pkt = usbRx.popPacket();
            if (!pkt) break;
            commandDispatcher.dispatch(*pkt);
        }

        usbTx.sendNext();

        if (HAL_GetTick() - lastTick >= 200)
        {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
            lastTick = HAL_GetTick();
        }
    }
}
}
