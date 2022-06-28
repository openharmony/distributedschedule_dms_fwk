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

#define private public
#include "distributed_sched_service.h"
#undef private
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace testing;
using namespace testing::ext;
namespace {
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
    DTEST_LOG << "ContinuationManagerTest::SetUp" << std::endl;
}

void ContinuationManagerTest::TearDown()
{
    DTEST_LOG << "ContinuationManagerTest::TearDown" << std::endl;
}

sptr<IDistributedSched> ContinuationManagerTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(samgr != nullptr);
    if (samgr == nullptr) {
        DTEST_LOG << "ContinuationManagerTest samgr is nullptr" << std::endl;
        return nullptr;
    }
    DTEST_LOG << "ContinuationManagerTest samgr is not nullptr" << std::endl;
    auto distributedObject = samgr->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(distributedObject != nullptr);
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "ContinuationManagerTest dmsProxy is nullptr" << std::endl;
    }
    DTEST_LOG << "ContinuationManagerTest dmsProxy is not nullptr" << std::endl;
    return proxy_;
}

void ContinuationManagerTest::InitContinuationHandler()
{
    if (DistributedSchedService::GetInstance().continuationHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("continuationManager");
        DistributedSchedService::GetInstance().continuationHandler_ =
            std::make_shared<AppExecFwk::EventHandler>(runner);
    }
}

/**
 * @tc.name: RegisterTest_001
 * @tc.desc: test register token
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, RegisterTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterTest_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token1 = -1;
    int32_t result1 = proxy->Register(nullptr, token1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t token2 = -1;
    int32_t result2 = proxy->Register(nullptr, token2);
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
HWTEST_F(ContinuationManagerTest, RegisterTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterTest_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t token1 = -1;
    int32_t result1 = proxy->Register(continuationExtraParams, token1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t token2 = -1;
    int32_t result2 = proxy->Register(continuationExtraParams, token2);
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
HWTEST_F(ContinuationManagerTest, UnregisterTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->Unregister(token);
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
HWTEST_F(ContinuationManagerTest, UnregisterTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterTest_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t result = proxy->Unregister(UNREGISTER_TOKEN);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = proxy->RegisterDeviceSelectionCallback(token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = proxy->RegisterDeviceSelectionCallback(token, CALLBACK_TYPE2, notifier);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result1 = proxy->RegisterDeviceSelectionCallback(UNREGISTER_TOKEN, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->RegisterDeviceSelectionCallback(UNREGISTER_TOKEN, CALLBACK_TYPE2, notifier);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_003, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = proxy->RegisterDeviceSelectionCallback(token, INVALID_CALLBACK_TYPE, notifier);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_004, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_004 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedSchedService::GetInstance().RegisterDeviceSelectionCallback(
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
    int32_t result3 = DistributedSchedService::GetInstance().OnDeviceConnect(token, continuationResults);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_005, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_005 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedSchedService::GetInstance().RegisterDeviceSelectionCallback(
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
    int32_t result3 = DistributedSchedService::GetInstance().OnDeviceConnect(token, continuationResults);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_006, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_006 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedSchedService::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = DistributedSchedService::GetInstance().OnDeviceDisconnect(token, deviceIds);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_007, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_007 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = DistributedSchedService::GetInstance().RegisterDeviceSelectionCallback(
        token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = DistributedSchedService::GetInstance().OnDeviceDisconnect(token, deviceIds);
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
HWTEST_F(ContinuationManagerTest, RegisterDeviceSelectionCallbackTest_008, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_008 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
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
    int32_t result2 = DistributedSchedService::GetInstance().OnDeviceConnect(token, continuationResults);
    DTEST_LOG << "result2:" << result2 << std::endl;
    std::vector<std::string> deviceIds;
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID1);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID2);
    deviceIds.emplace_back(UNSELECTED_DEVICE_ID3);
    int32_t result3 = DistributedSchedService::GetInstance().OnDeviceDisconnect(token, deviceIds);
    DTEST_LOG << "result3:" << result3 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result2);
    EXPECT_EQ(CALLBACK_HAS_NOT_REGISTERED, result3);
    DTEST_LOG << "ContinuationManagerTest RegisterDeviceSelectionCallbackTest_008 end" << std::endl;
}

/**
 * @tc.name: UnregisterDeviceSelectionCallbackTest_001
 * @tc.desc: test unregister device selection callback
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    int32_t result2 = proxy->RegisterDeviceSelectionCallback(token, CALLBACK_TYPE1, notifier);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = proxy->RegisterDeviceSelectionCallback(token, CALLBACK_TYPE2, notifier);
    DTEST_LOG << "result3:" << result3 << std::endl;
    int32_t result4 = proxy->UnregisterDeviceSelectionCallback(token, CALLBACK_TYPE1);
    DTEST_LOG << "result4:" << result4 << std::endl;
    int32_t result5 = proxy->UnregisterDeviceSelectionCallback(token, CALLBACK_TYPE2);
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
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->UnregisterDeviceSelectionCallback(token, CALLBACK_TYPE1);
    DTEST_LOG << "result2:" << result2 << std::endl;
    int32_t result3 = proxy->UnregisterDeviceSelectionCallback(token, CALLBACK_TYPE2);
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
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_003, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t result1 = proxy->UnregisterDeviceSelectionCallback(UNREGISTER_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->UnregisterDeviceSelectionCallback(UNREGISTER_TOKEN, CALLBACK_TYPE2);
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
HWTEST_F(ContinuationManagerTest, UnregisterDeviceSelectionCallbackTest_004, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UnregisterDeviceSelectionCallbackTest_004 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->UnregisterDeviceSelectionCallback(token, INVALID_CALLBACK_TYPE);
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
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = proxy->StartDeviceManager(token);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
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
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t result2 = proxy->StartDeviceManager(token, continuationExtraParams);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_002 end" << std::endl;
}

/**
 * @tc.name: StartDeviceManagerTest_003
 * @tc.desc: test start device manager with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, StartDeviceManagerTest_003, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t result1 = proxy->StartDeviceManager(UNREGISTER_TOKEN);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams =
        std::make_shared<ContinuationExtraParams>();
    int32_t result2 = proxy->StartDeviceManager(UNREGISTER_TOKEN, continuationExtraParams);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result1);
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result2);
    DTEST_LOG << "ContinuationManagerTest StartDeviceManagerTest_003 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_001
 * @tc.desc: test update connect status
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result2 = proxy->UpdateConnectStatus(token, deviceId, deviceConnectStatus);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_OK, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_001 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_002
 * @tc.desc: test update connect status with invalid token
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    std::string deviceId = TEST_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result = proxy->UpdateConnectStatus(UNREGISTER_TOKEN, deviceId, deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(TOKEN_HAS_NOT_REGISTERED, result);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_002 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_003
 * @tc.desc: test update connect status with empty deviceId
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_003, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = EMPTY_DEVICE_ID;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    int32_t result2 = proxy->UpdateConnectStatus(token, deviceId, deviceConnectStatus);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(ERR_NULL_OBJECT, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_003 end" << std::endl;
}

/**
 * @tc.name: UpdateConnectStatusTest_004
 * @tc.desc: test update connect status with invalid status
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, UpdateConnectStatusTest_004, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_004 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    int32_t token = -1;
    int32_t result1 = proxy->Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    std::string deviceId = TEST_DEVICE_ID;
    int32_t result2 = proxy->UpdateConnectStatus(token, deviceId,
        static_cast<DeviceConnectStatus>(ERROR_CONNECT_STATUS));
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(INVALID_CONNECT_STATUS, result2);
    DTEST_LOG << "ContinuationManagerTest UpdateConnectStatusTest_004 end" << std::endl;
}

/**
 * @tc.name: OnDeviceCancelTest_001
 * @tc.desc: test on device cancel when token has registered.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, OnDeviceCancelTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest OnDeviceCancelTest_001 start" << std::endl;
    InitContinuationHandler();
    int32_t token = -1;
    int32_t result1 = DistributedSchedService::GetInstance().Register(nullptr, token);
    DTEST_LOG << "result1:" << result1 << std::endl;
    int32_t result2 = DistributedSchedService::GetInstance().OnDeviceCancel(token);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(ERR_OK, result1);
    EXPECT_EQ(INVALID_PARAMETERS_ERR, result2);
    DTEST_LOG << "ContinuationManagerTest OnDeviceCancelTest_001 end" << std::endl;
}

/**
 * @tc.name: OnDeviceCancelTest_002
 * @tc.desc: test on device cancel when token has not registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, OnDeviceCancelTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest OnDeviceCancelTest_002 start" << std::endl;
    InitContinuationHandler();
    int32_t result = DistributedSchedService::GetInstance().OnDeviceCancel(UNREGISTER_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(INVALID_PARAMETERS_ERR, result);
    DTEST_LOG << "ContinuationManagerTest OnDeviceCancelTest_002 end" << std::endl;
}

/**
 * @tc.name: IsAccessTokenRegisterMaxTimesTest_001
 * @tc.desc: test is accessToken register max times.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsAccessTokenRegisterMaxTimesTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsAccessTokenRegisterMaxTimesTest_001 start" << std::endl;
    std::vector<int32_t> tokenVec;
    tokenVec.emplace_back(TEST_TOKEN);
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    DistributedSchedService::GetInstance().tokenMap_[accessToken] = tokenVec;
    bool result = DistributedSchedService::GetInstance().IsAccessTokenRegisterMaxTimes(TEST_ACCESS_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsAccessTokenRegisterMaxTimesTest_001 end" << std::endl;
}

/**
 * @tc.name: IsContinuationModeValidTest_001
 * @tc.desc: test is continuation mode valid.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsContinuationModeValidTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_001 start" << std::endl;
    ContinuationMode continuationMode = static_cast<ContinuationMode>(ERROR_CONTINUATION_MODE);
    bool result = DistributedSchedService::GetInstance().IsContinuationModeValid(continuationMode);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_001 end" << std::endl;
}

/**
 * @tc.name: IsContinuationModeValidTest_002
 * @tc.desc: test is continuation mode valid.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsContinuationModeValidTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_002 start" << std::endl;
    ContinuationMode continuationMode = ContinuationMode::COLLABORATION_MUTIPLE;
    bool result = DistributedSchedService::GetInstance().IsContinuationModeValid(continuationMode);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsContinuationModeValidTest_002 end" << std::endl;
}

/**
 * @tc.name: IsConnectStatusValidTest_001
 * @tc.desc: test is connect status valid.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsConnectStatusValidTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_001 start" << std::endl;
    DeviceConnectStatus deviceConnectStatus = static_cast<DeviceConnectStatus>(ERROR_CONNECT_STATUS);
    bool result = DistributedSchedService::GetInstance().IsConnectStatusValid(deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_001 end" << std::endl;
}

/**
 * @tc.name: IsConnectStatusValidTest_002
 * @tc.desc: test is connect status valid.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsConnectStatusValidTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_002 start" << std::endl;
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::CONNECTING;
    bool result = DistributedSchedService::GetInstance().IsConnectStatusValid(deviceConnectStatus);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsConnectStatusValidTest_002 end" << std::endl;
}

/**
 * @tc.name: IsTokenRegisteredTest_001
 * @tc.desc: test is token registered.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IsTokenRegisteredTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 start" << std::endl;
    std::vector<int32_t> tokenVec;
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    DistributedSchedService::GetInstance().tokenMap_[accessToken] = tokenVec;
    bool result = DistributedSchedService::GetInstance().IsTokenRegistered(TEST_ACCESS_TOKEN, TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IsTokenRegisteredTest_002
 * @tc.desc: test is token registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IsTokenRegisteredTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 start" << std::endl;
    std::vector<int32_t> tokenVec;
    tokenVec.emplace_back(TEST_TOKEN);
    uint32_t accessToken = TEST_ACCESS_TOKEN;
    DistributedSchedService::GetInstance().tokenMap_[accessToken] = tokenVec;
    bool result = DistributedSchedService::GetInstance().IsTokenRegistered(TEST_ACCESS_TOKEN, TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IsTokenRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_001
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_001, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_001 start" << std::endl;
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_001 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_002
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_002, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_002 start" << std::endl;
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_002 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_003
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: SR000H34KJ
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_003, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_003 start" << std::endl;
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    DistributedSchedService::GetInstance().callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(TEST_TOKEN);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_003 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_004
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_004, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_004 start" << std::endl;
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    DistributedSchedService::GetInstance().callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE2);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_004 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_005
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_005, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_005 start" << std::endl;
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    DistributedSchedService::GetInstance().callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(UNREGISTER_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(false, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_005 end" << std::endl;
}

/**
 * @tc.name: IfNotifierRegisteredTest_006
 * @tc.desc: test is notifier registered.
 * @tc.type: FUNC
 * @tc.require: AR000H34KK
 */
HWTEST_F(ContinuationManagerTest, IfNotifierRegisteredTest_006, TestSize.Level0)
{
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_006 start" << std::endl;
    std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
    sptr<IRemoteObject> notifier = new DeviceSelectionNotifierTest();
    notifierInfo->SetNotifier(CALLBACK_TYPE1, notifier);
    DistributedSchedService::GetInstance().callbackMap_[TEST_TOKEN] = std::move(notifierInfo);
    bool result = DistributedSchedService::GetInstance().IfNotifierRegistered(TEST_TOKEN, CALLBACK_TYPE1);
    DTEST_LOG << "result:" << result << std::endl;
    EXPECT_EQ(true, result);
    DTEST_LOG << "ContinuationManagerTest IfNotifierRegisteredTest_006 end" << std::endl;
}
} // namespace DistributedSchedule
} // namespace OHOS