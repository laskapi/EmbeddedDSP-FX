#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include <QEventLoop>
#include <chrono>
#include <future>
#include "Core/AppController.h"
#include "Device/IDevice.h"
#include "Device/IDeviceFactory.h"
#include "Device/IConnectionScanner.h"
#include "Device/DeviceSession.h"
#include "Processing/AudioFrameWorker.h"
#include "Protocol/ControlPacket.h"

namespace {

/// @brief Blocks until future is ready while pumping the Qt event loop (QueuedConnection / onTxIdle).
template <typename T>
bool waitForFuture(std::future<T>& future, std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        if (std::chrono::steady_clock::now() >= deadline) {
            return false;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    return true;
}

} // namespace

using namespace Host::Core;
using namespace Host::Device;
using namespace Host::Processing;
using namespace Host::Models;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::NiceMock;
using ::testing::_;
using ::testing::Field;
/// @brief Mock IDevice for AppController tests.
class MockDevice : public IDevice {
public:
    MOCK_METHOD(bool, open, (const QString& connectionId), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, sendControlPacket, (const Protocol::ControlPacket& pkt), (override));
};

class MockDeviceFactory : public IDeviceFactory {
public:
    MOCK_METHOD(std::unique_ptr<IDevice>, create, (const QString& connectionId), (override));
};

class MockConnectionScanner : public IConnectionScanner {
public:
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
};

class AppControllerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!qApp) {
            static int argc = 1;
            static char* argv[] = {(char*)"test"};
            static QCoreApplication app(argc, argv);
        }
    }

    void SetUp() override {
        auto connectionScanner = std::make_unique<NiceMock<MockConnectionScanner>>();
        mockScanner = connectionScanner.get();

        auto factory = std::make_unique<NiceMock<MockDeviceFactory>>();
        mockFactory = factory.get();

        auto deviceSession = std::make_unique<DeviceSession>(std::move(factory));
        auto audioWorker = std::make_unique<AudioFrameWorker>();
        controller = std::make_unique<AppController>(
            std::move(connectionScanner), std::move(deviceSession), std::move(audioWorker));
    }

    /// Creates a mock device, wires factory expectation, connects, returns raw mock pointer.
    MockDevice* connectWithMock(const QString& connectionId) {
        auto mock = std::make_unique<NiceMock<MockDevice>>();
        MockDevice* raw = mock.get();

        EXPECT_CALL(*mockFactory, create(connectionId))
            .WillOnce(Return(ByMove(std::unique_ptr<IDevice>(std::move(mock)))));

        std::promise<void> openedPromise;
        auto openedFuture = openedPromise.get_future();

        ON_CALL(*raw, sendControlPacket(_)).WillByDefault(Return(true));
        EXPECT_CALL(*raw, open(connectionId))
            .WillOnce([&openedPromise](const QString&) {
                openedPromise.set_value();
                return true;
            });

        controller->connectDevice(connectionId);

        auto status = openedFuture.wait_for(std::chrono::seconds(1));
        EXPECT_EQ(status, std::future_status::ready) << "Device open was not called within timeout";
        return raw;
    }

    void TearDown() override {
        controller.reset();
        QCoreApplication::processEvents();
    }

    MockConnectionScanner* mockScanner{nullptr};
    MockDeviceFactory* mockFactory{nullptr};
    std::unique_ptr<AppController> controller;
};

TEST_F(AppControllerTest, SetParameterSendsCorrectPacket) {
    const int slot = 2;
    const int param = 1;
    const float value = 0.75f;

    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    std::promise<void> sentPromise;
    auto sentFuture = sentPromise.get_future();

    EXPECT_CALL(*mockDevice, sendControlPacket(_))
        .WillOnce([&](const Protocol::ControlPacket& pkt) {
            EXPECT_EQ(pkt.command, Protocol::Command::SetParam);
            EXPECT_EQ(pkt.slotId, slot);
            EXPECT_EQ(pkt.paramId, param);
            EXPECT_FLOAT_EQ(pkt.value(), value);
            sentPromise.set_value();
            return true;
        });

    controller->setParameter(slot, param, value);

    ASSERT_TRUE(waitForFuture(sentFuture, std::chrono::seconds(1)))
        << "Control packet was not sent within timeout";
}

TEST_F(AppControllerTest, SetParameterCoalescesToLatestValue) {
    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    std::promise<void> enteredFirstSend;
    auto enteredFirstSendFuture = enteredFirstSend.get_future();
    std::promise<void> releaseFirstSend;
    auto releaseFirstSendFuture = releaseFirstSend.get_future();

    std::promise<void> donePromise;
    auto doneFuture = donePromise.get_future();

    /// First send blocks until later SetParams are queued, so they coalesce while tx is busy.
    EXPECT_CALL(*mockDevice, sendControlPacket(_))
        .WillOnce([&](const Protocol::ControlPacket& pkt) {
            EXPECT_EQ(pkt.command, Protocol::Command::SetParam);
            EXPECT_EQ(pkt.slotId, 1);
            EXPECT_EQ(pkt.paramId, 0);
            EXPECT_FLOAT_EQ(pkt.value(), 0.1f);
            enteredFirstSend.set_value();
            releaseFirstSendFuture.wait();
            return true;
        })
        .WillOnce([&](const Protocol::ControlPacket& pkt) {
            EXPECT_EQ(pkt.command, Protocol::Command::SetParam);
            EXPECT_EQ(pkt.slotId, 1);
            EXPECT_EQ(pkt.paramId, 0);
            EXPECT_FLOAT_EQ(pkt.value(), 0.9f);
            donePromise.set_value();
            return true;
        });

    controller->setParameter(1, 0, 0.1f);

    ASSERT_TRUE(waitForFuture(enteredFirstSendFuture, std::chrono::seconds(1)))
        << "First SetParam did not reach the device";

    controller->setParameter(1, 0, 0.5f);
    controller->setParameter(1, 0, 0.9f);
    releaseFirstSend.set_value();

    ASSERT_TRUE(waitForFuture(doneFuture, std::chrono::seconds(1)))
        << "Coalesced SetParam was not sent within timeout";
}

TEST_F(AppControllerTest, SetEffectTypeSendsCorrectPacket) {
    const int slot = 0;
    const int typeId = 4;

    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    std::promise<void> sentPromise;
    auto sentFuture = sentPromise.get_future();

    EXPECT_CALL(*mockDevice, sendControlPacket(AllOf(
                                 Field(&Protocol::ControlPacket::command,      Protocol::Command::SetEffectType),
                                 Field(&Protocol::ControlPacket::slotId,       slot),
                                 Field(&Protocol::ControlPacket::effectTypeId, typeId)
                                 )))
        .WillOnce([&]() {
            sentPromise.set_value();
            return true;
        });

    controller->setEffectType(slot, typeId);

    auto status = sentFuture.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready) << "Control packet was not sent within timeout";
}

TEST_F(AppControllerTest, EmitsManifestReadyWhenDeviceSignalsIt) {
    MockDevice* mockDevice = connectWithMock(QStringLiteral("DEMO_MODE"));

    QSignalSpy spy(controller.get(), &AppController::manifestReady);

    DeviceManifest manifest;
    manifest.slotCount = 8;
    emit mockDevice->manifestReady(manifest);

    ASSERT_TRUE(spy.wait(1000));
    ASSERT_EQ(spy.count(), 1);
    auto receivedManifest = spy.at(0).at(0).value<DeviceManifest>();
    EXPECT_EQ(receivedManifest.slotCount, 8);
}

TEST_F(AppControllerTest, HandlesConnectionErrorFromDevice) {
    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    QSignalSpy errorSpy(controller.get(), &AppController::errorOccurred);
    QString errorMsg = "Device disconnected unexpectedly";

    emit mockDevice->errorOccurred(errorMsg);

    ASSERT_TRUE(errorSpy.wait(1000));
    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), errorMsg);
}
