/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "continuation_manager_test.h"

#include "distributed_ability_manager_client.h"
#include "dtbschedmgr_log.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace testing;
using namespace testing::ext;
namespace {
constexpr int32_t DISTRIBUTED_SCHED_SA_ID = 1401;
constexpr int32_t UNREGISTER_TOKEN = 10000;
constexpr int32_t TEST_TOKEN = 1000;
constexpr int32_t TEST_ACCESS_TOKEN = 10000000;
constexpr int32_t ERROR_CONNECT_STATUS = 10;
constexpr int32_t ERROR_CONTINUATION_MODE = 10;
constexpr uint32_t SELECTED_DEVICE_SIZE = 2;
constexpr uint32_t UNSELECTED_DEVICE_SIZE = 3;
const std::string TEST_DEVICE_ID = "test deviceId";
const std::string EMPTY_DEVICE_ID = "";
const std::string CALLBACK_TYPE1 = "deviceConnect";
const std::string CALLBACK_TYPE2 = "deviceDisconnect";
const std::string INVALID_CALLBACK_TYPE = "deviceCancel";
const std::string SELECTED_DEVICE_ID1 = "selected deviceId1";
const std::string SELECTED_DEVICE_ID2 = "selected deviceId2";
const std::string SELECTED_DEVICE_TYPE1 = "selected deviceType1";
const std::string SELECTED_DEVICE_TYPE2 = "selected deviceType2";
const std::string SELECTED_DEVICE_NAME1 = "selected deviceName1";
const std::string SELECTED_DEVICE_NAME2 = "selected deviceName2";
const std::string UNSELECTED_DEVICE_ID1 = "unselected deviceId1";
const std::string UNSELECTED_DEVICE_ID2 = "unselected deviceId2";
const std::string UNSELECTED_DEVICE_ID3 = "unselected deviceId3";
}

void DeviceSelectionNotifierTest::OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults)
{
    EXPECT_EQ(SELECTED_DEVICE_SIZE, continuationResults.size());
    for (size_t i = 0; i < continuationResults.size(); ++i) {
        DTEST_LOG << "DeviceSelectionNotifierTest::OnDeviceConnect selected deviceId:"<<
            continuationResults[i].GetDeviceId() << std::endl;
        DTEST_LOG << "DeviceSelectionNotifierTest::OnDeviceConnect selected deviceType:" <<
            continuationResults[i].GetDeviceType() << std::endl;
        DTEST_LOG << "DeviceSelectionNotifierTest::OnDeviceConnect selected deviceNane:" <<
            continuationResults[i].GetDeviceName() << std::endl;
    }
}

void DeviceSelectionNotifierTest::OnDeviceDisconnect(const std::vector<std::string>& deviceIds)
{
    EXPECT_EQ(UNSELECTED_DEVICE_SIZE, deviceIds.size());
    for (size_t i = 0; i < deviceIds.size(); ++i) {
        DTEST_LOG << "DeviceSelectionNotifierTest::OnDeviceDisconnect unselected deviceId:"<<
            deviceIds[i] << std::endl;
    }
}

void ContinuationManagerTest::SetUpTestCase()
{
    DTEST_LOG << "ContinuationManagerTest::SetUpTestCase" << std::endl;
}

void ContinuationManagerTest::TearDownTestCase()
{
    DTEST_LOG << "ContinuationManagerTest::TearDownTestCase" << std::endl;
}

void ContinuationManagerTest::SetUp()
{
    dtbabilitymgrService_ = new DistributedAbilityManagerService(DISTRIBUTED_SCHED_SA_ID, true);
    if (dtbabilitymgrService_->continuationHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("continuationManagerTest");
        dtbabilitymgrService_->continuationHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    DTEST_LOG << "ContinuationManagerTest::SetUp" << std::endl;
}

void ContinuationManagerTest::TearDown()
{
    DTEST_LOG << "ContinuationManagerTest::TearDown" << std::endl;
}

/**
 * @tc.name: RegisterTest_001
 * @tc.desc: test register token
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, RegisterTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterTest_001 start" << std::endl;
    int32_t token1 = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t token2 = -1;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token2);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(1, token2 - token1);
    DTEST_LOG << "ContinuationManagerTest RegisterTest_001 end" << std::endl;
}

/**
 * @tc.name: RegisterTest_002
 * @tc.desc: test register token with extra param
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterTest_002 start" << std::endl;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t token1 = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(continuationExtraParams, token1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t token2 = -1;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().Register(continuationExtraParams, token2);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(1, token2 - token1);
    DTEST_LOG << "ContinuationManagerTest RegisterTest_002 end" << std::endl;
}

/**
 * @tc.name: UnregisterTest_001
 * @tc.desc: test unregister token
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, UnregisterTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_001 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().Unregister(token);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_001 end" << std::endl;
}

/**
 * @tc.name: UnregisterTest_002
 * @tc.desc: test unregister token with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UnregisterTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_002 start" << std::endl;
    int32_t result = DistributedAbilityManagerClient::GetInstance().Unregister(UNREGISTER_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result);
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_002 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_001
 * @tc.desc: test register device selection callback
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_001 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(ERR_OK, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_001 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_002
 * @tc.desc: test register device selection callback with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_002 start" << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_002 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_003
 * @tc.desc: test register device selection callback with invalid callback type
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_003, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_003 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, INVALID_CALLBACK_TYPE, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(UNKNOWN_CALLBACK_TYPE, result2);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_003 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_004
 * @tc.desc: test dms deviceConnect callback called when device selection callback has registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_004, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_004 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<ContinuationResult> continuationResults;
    ContinuationResult continuationResult1;
    continuationResult1.SetDeviceId(SELECTED_DEVICE_ID1);
    continuationResult1.SetDeviceType(SELECTED_DEVICE_TYPE1);
    continuationResult1.SetDeviceName(SELECTED_DEVICE_NAME1);
    ContinuationResult continuationResult2;
    continuationResult2.SetDeviceId(SELECTED_DEVICE_ID2);
    continuationResult2.SetDeviceType(SELECTED_DEVICE_TYPE2);
    continuationResult2.SetDeviceName(SELECTED_DEVICE_NAME2);
    continuationResults.emplace_back(continuationResult1);
    continuationResults.emplace_back(continuationResult2);
    int32_t result3 = dtbabilitymgrService_->OnDeviceConnect(token, continuationResults);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(ERR_OK, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_004 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_005
 * @tc.desc: test dms deviceConnect callback called when device selection callback has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_005, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_005 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<ContinuationResult> continuationResults;
    ContinuationResult continuationResult1;
    continuationResult1.SetDeviceId(SELECTED_DEVICE_ID1);
    continuationResult1.SetDeviceType(SELECTED_DEVICE_TYPE1);
    continuationResult1.SetDeviceName(SELECTED_DEVICE_NAME1);
    ContinuationResult continuationResult2;
    continuationResult2.SetDeviceId(SELECTED_DEVICE_ID2);
    continuationResult2.SetDeviceType(SELECTED_DEVICE_TYPE2);
    continuationResult2.SetDeviceName(SELECTED_DEVICE_NAME2);
    continuationResults.emplace_back(continuationResult1);
    continuationResults.emplace_back(continuationResult2);
    int32_t result3 = dtbabilitymgrService_->OnDeviceConnect(token, continuationResults);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_005 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_006
 * @tc.desc: test dms deviceDisconnect callback called when device selection callback has registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_006, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_006 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = dtbabilitymgrService_->OnDeviceDisconnect(token, deviceIds);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(ERR_OK, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_006 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_007
 * @tc.desc: test dms deviceDisconnect callback called when device selection callback has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_007, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_007 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = dtbabilitymgrService_->OnDeviceDisconnect(token, deviceIds);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_007 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_008
 * @tc.desc: test dms callback called when device selection callback has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_008, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_008 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::vector<ContinuationResult> continuationResults;
    ContinuationResult continuationResult1;
    continuationResult1.SetDeviceId(SELECTED_DEVICE_ID1);
    continuationResult1.SetDeviceType(SELECTED_DEVICE_TYPE1);
    continuationResult1.SetDeviceName(SELECTED_DEVICE_NAME1);
    ContinuationResult continuationResult2;
    continuationResult2.SetDeviceId(SELECTED_DEVICE_ID2);
    continuationResult2.SetDeviceType(SELECTED_DEVICE_TYPE2);
    continuationResult2.SetDeviceName(SELECTED_DEVICE_NAME2);
    continuationResults.emplace_back(continuationResult1);
    continuationResults.emplace_back(continuationResult2);
    int32_t result2 = dtbabilitymgrService_->OnDeviceConnect(token, continuationResults);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = dtbabilitymgrService_->OnDeviceDisconnect(token, deviceIds);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_008 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_009
 * @tc.desc: test dms callback called when token has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_009, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_009 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result1 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = dtbabilitymgrService_->RegisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<ContinuationResult> continuationResults;
    ContinuationResult continuationResult1;
    continuationResult1.SetDeviceId(SELECTED_DEVICE_ID1);
    continuationResult1.SetDeviceType(SELECTED_DEVICE_TYPE1);
    continuationResult1.SetDeviceName(SELECTED_DEVICE_NAME1);
    ContinuationResult continuationResult2;
    continuationResult2.SetDeviceId(SELECTED_DEVICE_ID2);
    continuationResult2.SetDeviceType(SELECTED_DEVICE_TYPE2);
    continuationResult2.SetDeviceName(SELECTED_DEVICE_NAME2);
    continuationResults.emplace_back(continuationResult1);
    continuationResults.emplace_back(continuationResult2);
    int32_t result3 = dtbabilitymgrService_->OnDeviceConnect(
        UNREGISTER_TOKEN, continuationResults);
    DTEST_LOG << "result3:" << result3 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result4 = dtbabilitymgrService_->OnDeviceDisconnect(UNREGISTER_TOKEN, deviceIds);
    DTEST_LOG << "result4:" << result4 << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result4);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_009 end" << std::endl;
}

/**
 * @tc.name: RegisterDeviceSelectionCallbackTest_010
 * @tc.desc: test dms callback called when token and device selection callback has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_010, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_010 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::vector<ContinuationResult> continuationResults;
    ContinuationResult continuationResult1;
    continuationResult1.SetDeviceId(SELECTED_DEVICE_ID1);
    continuationResult1.SetDeviceType(SELECTED_DEVICE_TYPE1);
    continuationResult1.SetDeviceName(SELECTED_DEVICE_NAME1);
    ContinuationResult continuationResult2;
    continuationResult2.SetDeviceId(SELECTED_DEVICE_ID2);
    continuationResult2.SetDeviceType(SELECTED_DEVICE_TYPE2);
    continuationResult2.SetDeviceName(SELECTED_DEVICE_NAME2);
    continuationResults.emplace_back(continuationResult1);
    continuationResults.emplace_back(continuationResult2);
    int32_t result1 = dtbabilitymgrService_->OnDeviceConnect(
        UNREGISTER_TOKEN, continuationResults);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result2 = dtbabilitymgrService_->OnDeviceDisconnect(UNREGISTER_TOKEN, deviceIds);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_010 end" << std::endl;
}

/**
 * @tc.name: UnregisterDeviceSelectionCallbackTest_001
 * @tc.desc: test unregister device selection callback
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_001 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result3:" << result3 << std::endl;
    int32_t result4 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1);
    DTEST_LOG << "result4:" << result4 << std::endl;
    int32_t result5 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2);
    DTEST_LOG << "result5:" << result5 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(ERR_OK, result3);
    EXPECT_EQ(ERR_OK, result4);
    EXPECT_EQ(ERR_OK, result5);
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_001 end" << std::endl;
}

/**
 * @tc.name: UnregisterDeviceSelectionCallbackTest_002
 * @tc.desc: test unregister device selection callback that has not been registered
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_002 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_002 end" << std::endl;
}

/**
 * @tc.name: UnregisterDeviceSelectionCallbackTest_003
 * @tc.desc: test register device selection callback with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_003, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_003 start" << std::endl;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        UNREGISTER_TOKEN, CALLBACK_TYPE2);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_003 end" << std::endl;
}

/**
 * @tc.name: UnregisterDeviceSelectionCallbackTest_004
 * @tc.desc: test register device selection callback with invalid callback type
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_004, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_004 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(
        token, INVALID_CALLBACK_TYPE);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(UNKNOWN_CALLBACK_TYPE, result2);
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_004 end" << std::endl;
}

/**
 * @tc.name: StartDeviceManagerTest_001
 * @tc.desc: test start device manager
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, StartDeviceManagerTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = dtbabilitymgrService_->StartDeviceManager(token);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_001 end" << std::endl;
}

/**
 * @tc.name: StartDeviceManagerTest_002
 * @tc.desc: test start device manager with extra param
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, StartDeviceManagerTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    int32_t token = -1;
    int32_t result1 = dtbabilitymgrService_->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t result2 = dtbabilitymgrService_->StartDeviceManager(
        token, continuationExtraParams);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_002 end" << std::endl;
}

/**
 * @tc.name: StartDeviceManagerTest_003
 * @tc.desc: test start device manager with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, StartDeviceManagerTest_003, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_003 start" << std::endl;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().StartDeviceManager(UNREGISTER_TOKEN);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().StartDeviceManager(
        UNREGISTER_TOKEN, continuationExtraParams);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_003 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_001
 * @tc.desc: test update connect status when device selection callback has registered.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_001 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result3 = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
        token, deviceId, deviceConnectStatus);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    EXPECT_EQ(ERR_OK, result3);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_001 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_002
 * @tc.desc: test update connect status when device selection callback has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_002 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
        token, deviceId, deviceConnectStatus);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_002 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_003
 * @tc.desc: test update connect status with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_003, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_003 start" << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
        UNREGISTER_TOKEN, deviceId, deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_003 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_004
 * @tc.desc: test update connect status with empty deviceId
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_004, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_004 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = EMPTY_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
        token, deviceId, deviceConnectStatus);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_NULL_OBJECT, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_004 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_005
 * @tc.desc: test update connect status with invalid status
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_005, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_005 start" << std::endl;
    int32_t token = -1;
    int32_t result1 = DistributedAbilityManagerClient::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    int32_t result2 = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
        token, deviceId, static_cast<DeviceConnectStatus>(ERROR_CONNECT_STATUS));
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(INVALID_CONNECT_STATUS, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_005 end" << std::endl;
}

/**
 * @tc.name: IsExceededRegisterMaxNumTest_001
 * @tc.desc: test IsExceededRegisterMaxNum function.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsExceededRegisterMaxNumTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsExceededRegisterMaxNumTest_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::vector<int32_t> tokenVec;
    tokenVec.emplace_back(TEST_TOKEN);
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    dtbabilitymgrService_->tokenMap_[accessToken] = tokenVec;
    bool result = dtbabilitymgrService_->IsExceededRegisterMaxNum(TEST_ACCESS_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsExceededRegisterMaxNumTest_001 end" << std::endl;
}

/**
 * @tc.name: IsContinuationModeValidTest_001
 * @tc.desc: test IsContinuationModeValid function with invalid continuation mode.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsContinuationModeValidTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    ContinuationMode continuationMode = static_cast<ContinuationMode>(ERROR_CONTINUATION_MODE);
    bool result = dtbabilitymgrService_->IsContinuationModeValid(continuationMode);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_001 end" << std::endl;
}

/**
 * @tc.name: IsContinuationModeValidTest_002
 * @tc.desc: test IsContinuationModeValid function with correct continuation mode.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsContinuationModeValidTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    ContinuationMode continuationMode = ContinuationMode::COLLABORATION_MUTIPLE;
    bool result = dtbabilitymgrService_->IsContinuationModeValid(continuationMode);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_002 end" << std::endl;
}

/**
 * @tc.name: IsConnectStatusValidTest_001
 * @tc.desc: test IsConnectStatusValid function with invalid connect status.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsConnectStatusValidTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    DeviceConnectStatus deviceConnectStatus = static_cast<DeviceConnectStatus>(ERROR_CONNECT_STATUS);
    bool result = dtbabilitymgrService_->IsConnectStatusValid(deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_001 end" << std::endl;
}

/**
 * @tc.name: IsConnectStatusValidTest_002
 * @tc.desc: test IsConnectStatusValid function with correct connect status.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsConnectStatusValidTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    bool result = dtbabilitymgrService_->IsConnectStatusValid(deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_002 end" << std::endl;
}

/**
 * @tc.name: IsTokenRegisteredTest_001
 * @tc.desc: test IsTokenRegistered function with invalid token.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsTokenRegisteredTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::vector<int32_t> tokenVec;
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    dtbabilitymgrService_->tokenMap_[accessToken] = tokenVec;
    bool result = dtbabilitymgrService_->IsTokenRegistered(TEST_ACCESS_TOKEN, TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IsTokenRegisteredTest_002
 * @tc.desc: test IsTokenRegistered function with correct token.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsTokenRegisteredTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::vector<int32_t> tokenVec;
    tokenVec.emplace_back(TEST_TOKEN);
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    dtbabilitymgrService_->tokenMap_[accessToken] = tokenVec;
    bool result = dtbabilitymgrService_->IsTokenRegistered(TEST_ACCESS_TOKEN, TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_001
 * @tc.desc: test IfNotifierRegistered function with invalid token.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_001 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_002
 * @tc.desc: test IfNotifierRegistered function with invalid token.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_003
 * @tc.desc: test IfNotifierRegistered function with invalid callback type.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_003, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_003 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN, INVALID_CALLBACK_TYPE);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_003 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_004
 * @tc.desc: test IfNotifierRegistered function with correct token.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_004, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_004 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_004 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_005
 * @tc.desc: test IfNotifierRegistered function with incorrect callback type.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_005, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_005 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE2);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_005 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_006
 * @tc.desc: test IfNotifierRegistered function with incorrect token.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_006, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_006 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = dtbabilitymgrService_->IfNotifierRegistered(UNREGISTER_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_006 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_007
 * @tc.desc: test IfNotifierRegistered function with correct token and callback type.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_007, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_007 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = dtbabilitymgrService_->IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_007 end" << std::endl;
}

/**
 * @tc.name: QueryTokenByNotifier_001
 * @tc.desc: test QueryTokenByNotifier function with incorrect notifier.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, QueryTokenByNotifier_001, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest QueryTokenByNotifier_001 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    int32_t token = -1;
    bool result = dtbabilitymgrService_->QueryTokenByNotifier(nullptr, token);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest QueryTokenByNotifier_001 end" << std::endl;
}

/**
 * @tc.name: QueryTokenByNotifier_002
 * @tc.desc: test QueryTokenByNotifier function with correct notifier.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, QueryTokenByNotifier_002, TestSize.Level1)
{
    DTEST_LOG << "ContinuationManagerTest QueryTokenByNotifier_002 start" << std::endl;
    if (dtbabilitymgrService_ == nullptr) {
        DTEST_LOG << "dtbabilitymgrService_ is nullptr" << std::endl;
        return;
    }
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<DeviceSelectionNotifierTest> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    dtbabilitymgrService_->callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    int32_t token = -1;
    bool result = dtbabilitymgrService_->QueryTokenByNotifier(notifier, token);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    EXPECT_EQ(TEST_TOKEN, token);
    DTEST_LOG << "ContinuationManagerTest QueryTokenByNotifier_002 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS