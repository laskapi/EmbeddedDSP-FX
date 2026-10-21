#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include <future>
#include "AppController.h"
#include "IDevice.h"
#include "IDeviceFactory.h"
#include "IConnectionScanner.h"
#include "AudioFrameWorker.h"
#include <Protocol/ControlPacket.h>

using namespace Host::Core;
using namespace Host::Device;
using namespace Host::Processing;
using namespace Host::Models;
using ::testing::Return;
using ::testing::ByMove;
using ::testing::NiceMock;
using ::testing::_;

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
        auto scanner = std::make_unique<NiceMock<MockConnectionScanner>>();
        mockScanner = scanner.get();

        auto factory = std::make_unique<NiceMock<MockDeviceFactory>>();
        mockFactory = factory.get();

        auto worker = std::make_unique<AudioFrameWorker>();
        controller = std::make_unique<AppController>(
            std::move(scanner), std::move(factory), std::move(worker));
    }

    /// Creates a mock device, wires factory expectation, connects, returns raw mock pointer.
    MockDevice* connectWithMock(const QString& connectionId) {
        auto mock = std::make_unique<NiceMock<MockDevice>>();
        MockDevice* raw = mock.get();

        EXPECT_CALL(*mockFactory, create(connectionId))
            .WillOnce(Return(ByMove(std::unique_ptr<IDevice>(std::move(mock)))));

        ON_CALL(*raw, open(_)).WillByDefault(Return(true));
        ON_CALL(*raw, sendControlPacket(_)).WillByDefault(Return(true));

        controller->connectDevice(connectionId);
        QCoreApplication::processEvents();
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

TEST_F(AppControllerTest, ConnectDeviceCallsDeviceOpenWithConnectionId) {
    const QString connectionId = QStringLiteral("COM3");

    auto mock = std::make_unique<NiceMock<MockDevice>>();
    MockDevice* raw = mock.get();

    std::promise<void> calledPromise;
    auto calledFuture = calledPromise.get_future();

    EXPECT_CALL(*mockFactory, create(connectionId))
        .WillOnce(Return(ByMove(std::unique_ptr<IDevice>(std::move(mock)))));

    EXPECT_CALL(*raw, open(connectionId))
        .WillOnce([&calledPromise](const QString&) {
            calledPromise.set_value();
            return true;
        });

    controller->connectDevice(connectionId);

    auto status = calledFuture.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready) << "Device method was not called within timeout";
}

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
            EXPECT_FLOAT_EQ(pkt.getValue(), value);
            sentPromise.set_value();
            return true;
        });

    controller->setParameter(slot, param, value);

    auto status = sentFuture.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready) << "Control packet was not sent within timeout";
}

TEST_F(AppControllerTest, SetEffectTypeSendsCorrectPacket) {
    const int slot = 0;
    const int typeId = 4;

    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    std::promise<void> sentPromise;
    auto sentFuture = sentPromise.get_future();

    EXPECT_CALL(*mockDevice, sendControlPacket(_))
        .WillOnce([&](const Protocol::ControlPacket& pkt) {
            EXPECT_EQ(pkt.command, Protocol::Command::SetEffectType);
            EXPECT_EQ(pkt.slotId, slot);
            EXPECT_EQ(pkt.effectTypeId, typeId);
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

    ASSERT_EQ(spy.count(), 1);
    auto receivedManifest = spy.at(0).at(0).value<DeviceManifest>();
    EXPECT_EQ(receivedManifest.slotCount, 8);
}

TEST_F(AppControllerTest, HandlesConnectionErrorFromDevice) {
    MockDevice* mockDevice = connectWithMock(QStringLiteral("COM3"));

    QSignalSpy errorSpy(controller.get(), &AppController::errorOccurred);
    QString errorMsg = "Device disconnected unexpectedly";

    emit mockDevice->errorOccurred(errorMsg);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), errorMsg);
}
