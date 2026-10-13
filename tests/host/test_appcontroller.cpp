#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include <future>
#include "AppController.h"
#include "IDeviceBackend.h"
#include "AudioFrameWorker.h"
#include <Protocol/ControlPacket.h>

using namespace Host::Core;
using namespace Host::Backend;
using namespace Host::Processing;
using namespace Host::Models;
using ::testing::Return;

/// @brief Mock implementation of IDeviceBackend for testing AppController logic.
class MockDeviceBackend : public IDeviceBackend {
public:
    MOCK_METHOD(bool, connectToDevice, (const QString &target), (override));
    MOCK_METHOD(void, disconnectFromDevice, (), (override));
    MOCK_METHOD(bool, sendControlPacket, (const Protocol::ControlPacket &pkt), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, requestAvailableConnections, (), (override));
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
        auto mock = std::make_unique<MockDeviceBackend>();
        mockBackend = mock.get();
        
        // Ignore the automatic scan calls from constructor to keep logs clean
        EXPECT_CALL(*mockBackend, requestAvailableConnections())
            .Times(::testing::AnyNumber());

        auto worker = std::make_unique<AudioFrameWorker>();
        controller = std::make_unique<AppController>(std::move(mock), std::move(worker));
    }

    void TearDown() override {
        controller.reset();
        QCoreApplication::processEvents();
    }

    MockDeviceBackend* mockBackend{nullptr};
    std::unique_ptr<AppController> controller;
};

TEST_F(AppControllerTest, ConnectToDeviceCallsBackendWithCorrectPort) {
    const QString testPort = "COM3";
    
    std::promise<void> calledPromise;
    auto calledFuture = calledPromise.get_future();

    EXPECT_CALL(*mockBackend, connectToDevice(testPort))
        .WillOnce([&calledPromise](const QString&) {
            calledPromise.set_value();
            return true;
        });

    controller->connectToDevice(testPort);
    
    auto status = calledFuture.wait_for(std::chrono::seconds(1));
    ASSERT_EQ(status, std::future_status::ready) << "Backend method was not called within timeout";
}

TEST_F(AppControllerTest, SetParameterSendsCorrectPacket) {
    const int slot = 2;
    const int param = 1;
    const float value = 0.75f;

    std::promise<void> sentPromise;
    auto sentFuture = sentPromise.get_future();

    // Verify that the controller sends a packet with correct command, slot and value
    EXPECT_CALL(*mockBackend, sendControlPacket(::testing::_))
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

    std::promise<void> sentPromise;
    auto sentFuture = sentPromise.get_future();

    EXPECT_CALL(*mockBackend, sendControlPacket(::testing::_))
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

TEST_F(AppControllerTest, EmitsManifestReadyWhenBackendSignalsIt) {
    QSignalSpy spy(controller.get(), &AppController::manifestReady);
    
    DeviceManifest manifest;
    manifest.slotCount = 8;

    emit mockBackend->manifestReady(manifest);

    ASSERT_EQ(spy.count(), 1);
    auto receivedManifest = spy.at(0).at(0).value<DeviceManifest>();
    EXPECT_EQ(receivedManifest.slotCount, 8);
}

TEST_F(AppControllerTest, HandlesConnectionErrorFromBackend) {
    QSignalSpy errorSpy(controller.get(), &AppController::errorOccurred);
    QString errorMsg = "Device disconnected unexpectedly";

    emit mockBackend->errorOccurred(errorMsg);

    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), errorMsg);
}
