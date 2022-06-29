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
#include "gtest/gtest.h"

#define private public
#define protected public
#include "ability_connect_callback_stub.h"
#include "distributed_sched_adapter.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_device_info_storage.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "test_log.h"
#undef private
#undef protected

namespace OHOS {
namespace DistributedSchedule {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr int32_t INVALID_PARAMETERS_ERR_CODE = 29360128;
constexpr int32_t INVALID_REMOTE_PARAMETERS_ERR_CODE = 29360131;
constexpr int32_t MOCK_UID = 1000;
constexpr int32_t MOCK_PID = 1000;
const std::string MOCK_DEVICE_ID = "1234567890";
}

class AbilityCallCallbackTest : public AAFwk::AbilityConnectionStub {
public:
    AbilityCallCallbackTest() = default;
    ~AbilityCallCallbackTest() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
        int32_t resultCode) override;
};

class AbilityCallWrapperStubTest : public AAFwk::AbilityConnectionStub {
public:
    explicit AbilityCallWrapperStubTest(sptr<IRemoteObject> connection) : distributedCall_(connection) {}
    ~AbilityCallWrapperStubTest() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
        int32_t resultCode) override;

private:
    sptr<IRemoteObject> distributedCall_;
};

class DistributedSchedCallTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    void AddSession(const sptr<IRemoteObject>& connect, const std::string& localDeviceId,
        const std::string& remoteDeviceId, const AAFwk::Want& want) const;
    void RemoveSession(const sptr<IRemoteObject>& connect) const;

    void AddConnectInfo(const sptr<IRemoteObject>& connect, const std::string& localDeviceId,
        const std::string& remoteDeviceId) const;
    void RemoveConnectInfo(const sptr<IRemoteObject>& connect) const;

    void AddConnectCount(int32_t uid) const;
    void DecreaseConnectCount(int32_t uid) const;
};

void AbilityCallCallbackTest::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    DTEST_LOG << "DistributedSchedServiceTest call OnAbilityConnectDone " << std::endl;
}

void AbilityCallCallbackTest::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
    DTEST_LOG << "DistributedSchedServiceTest call OnAbilityDisconnectDone " << std::endl;
}

void AbilityCallWrapperStubTest::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int32_t resultCode)
{
    DTEST_LOG << "DistributedSchedServiceTest call OnAbilityConnectDone " << std::endl;
}

void AbilityCallWrapperStubTest::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int32_t resultCode)
{
    DTEST_LOG << "DistributedSchedServiceTest call OnAbilityDisconnectDone " << std::endl;
}

void DistributedSchedCallTest::SetUpTestCase()
{
    DTEST_LOG << "DistributedSchedServiceTest call SetUpTestCase " << std::endl;
}

void DistributedSchedCallTest::TearDownTestCase()
{
    DTEST_LOG << "DistributedSchedServiceTest call TearDownTestCase " << std::endl;
}

void DistributedSchedCallTest::SetUp()
{
    DTEST_LOG << "DistributedSchedServiceTest call SetUp " << std::endl;
}

void DistributedSchedCallTest::TearDown()
{
    DTEST_LOG << "DistributedSchedServiceTest call TearDown " << std::endl;
}

/**
 * @tc.name: CallAbility_001
 * @tc.desc: Call StartRemoteAbilityByCall with illegal want
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_001, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_001 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    int32_t callerUid = MOCK_UID;
    int32_t callerPid = MOCK_PID;
    uint32_t accessToken = 0;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal want " << std::endl;
    OHOS::AAFwk::Want illegalWant;
    illegalWant.SetElementName("", "ohos.demo.test", "abilityTest");
    int32_t result = DistributedSchedService::GetInstance().StartRemoteAbilityByCall(illegalWant,
        callback, callerUid, callerPid, accessToken);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_001 end " << std::endl;
}

/**
 * @tc.name: CallAbility_002
 * @tc.desc: Call TryStartRemoteAbilityByCall with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_002, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_002 start " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.dmsVersion = 0;
    callerInfo.accessToken = 0;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal want " << std::endl;
    int32_t result = DistributedSchedService::GetInstance().TryStartRemoteAbilityByCall(mockWant,
        callback, callerInfo);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_002 end " << std::endl;
}

/**
 * @tc.name: CallAbility_003
 * @tc.desc: Call StartAbilityByCallFromRemote with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_003, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_003 start " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.dmsVersion = 1000;
    callerInfo.accessToken = 0;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    IDistributedSched::AccountInfo accountInfo;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal want " << std::endl;
    int32_t result = DistributedSchedService::GetInstance().StartAbilityByCallFromRemote(mockWant,
        callback, callerInfo, accountInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_003 end " << std::endl;
}

/**
 * @tc.name: CallAbility_004
 * @tc.desc: Call StartAbilityByCall with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_004, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_004 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    sptr<IRemoteObject> callbackWrapper = new AbilityCallWrapperStubTest(callback);
    sptr<IRemoteObject> callerToken = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal want " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.xxx", "xxx");
    int32_t result = DistributedSchedAdapter::GetInstance().StartAbilityByCall(mockWant,
        callbackWrapper, new AbilityCallCallbackTest());
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_004 end " << std::endl;
}

/**
 * @tc.name: CallAbility_005
 * @tc.desc: Call ReleaseRemoteAbility with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_005, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_005 start " << std::endl;
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal element " << std::endl;
    AppExecFwk::ElementName mockElement("", "ohos.demo.test", "abilityTest");
    int32_t result = DistributedSchedService::GetInstance().ReleaseRemoteAbility(callback,
        mockElement);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_005 end " << std::endl;
}

/**
 * @tc.name: CallAbility_006
 * @tc.desc: Call ReleaseAbilityFromRemote with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_006, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_006 start " << std::endl;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 1000;
    callerInfo.dmsVersion = 1000;
    callerInfo.uid = MOCK_UID;
    callerInfo.accessToken = 1000;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal element " << std::endl;
    AppExecFwk::ElementName mockElement("", "ohos.demo.test", "abilityTest");
    int32_t result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        mockElement, callerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest get illegal callerInfo" << std::endl;
    CallerInfo mockCallerInfo = callerInfo;
    mockCallerInfo.uid = -1;
    result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_006 end " << std::endl;
}

/**
 * @tc.name: CallAbility_007
 * @tc.desc: Call ReleaseAbilityFromRemote with illegal callerInfo
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_007, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_007 start " << std::endl;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.dmsVersion = 0;
    callerInfo.accessToken = 0;

    DTEST_LOG << "DistributedSchedServiceTest get illegal uid" << std::endl;
    CallerInfo mockCallerInfo = callerInfo;
    mockCallerInfo.uid = -1;
    int32_t result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest get illegal sourceDeviceId" << std::endl;
    mockCallerInfo = callerInfo;
    mockCallerInfo.sourceDeviceId = "";
    result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest get illegal duid" << std::endl;
    mockCallerInfo = callerInfo;
    mockCallerInfo.duid = -1;
    result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest get illegal accessToken" << std::endl;
    mockCallerInfo = callerInfo;
    mockCallerInfo.accessToken = -1;
    result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

        DTEST_LOG << "DistributedSchedServiceTest get illegal dmsVersion" << std::endl;
    mockCallerInfo = callerInfo;
    mockCallerInfo.dmsVersion = -1;
    result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(callback,
        element, mockCallerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_007 end " << std::endl;
}

/**
 * @tc.name: CallAbility_008
 * @tc.desc: Call StartAbilityByCall with illegal parameter
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_008, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_008 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    sptr<IRemoteObject> callbackWrapper = new AbilityCallWrapperStubTest(callback);

    DTEST_LOG << "DistributedSchedServiceTest mock illegal ElementName " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "", "abilityTest");
    int32_t result = DistributedSchedAdapter::GetInstance().ReleaseAbility(callbackWrapper,
        mockWant.GetElement());
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    sptr<IRemoteObject> mockCallbackWrapper = nullptr;
    result = DistributedSchedAdapter::GetInstance().ReleaseAbility(mockCallbackWrapper,
        want.GetElement());
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest mock illegal ElementName " << std::endl;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "");
    result = DistributedSchedAdapter::GetInstance().ReleaseAbility(callbackWrapper,
        mockWant.GetElement());
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest mock illegal ElementName " << std::endl;
    mockWant.SetElementName("", "ohos.demo.test", "abilityTest");
    result = DistributedSchedAdapter::GetInstance().ReleaseAbility(callbackWrapper,
        mockWant.GetElement());
    EXPECT_NE(result, 0);
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_008 end " << std::endl;
}

/**
 * @tc.name: CallAbility_011
 * @tc.desc: Call StartRemoteAbilityByCall with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_011, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_011 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    int32_t callerUid = MOCK_UID;
    int32_t callerPid = MOCK_PID;
    uint32_t accessToken = 0;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    sptr<IRemoteObject> illegalCallback = nullptr;
    int32_t result = DistributedSchedService::GetInstance().StartRemoteAbilityByCall(want,
        illegalCallback, callerUid, callerPid, accessToken);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_011 end " << std::endl;
}

/**
 * @tc.name: CallAbility_012
 * @tc.desc: Call StartRemoteAbilityByCall with illegal uid
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_012, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_012 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    int32_t callerPid = MOCK_PID;
    uint32_t accessToken = 0;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal uid " << std::endl;
    int32_t illegalUid = -1;
    int32_t result = DistributedSchedService::GetInstance().StartRemoteAbilityByCall(want,
        callback, illegalUid, callerPid, accessToken);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_012 end " << std::endl;
}

/**
 * @tc.name: CallAbility_013
 * @tc.desc: Call TryStartRemoteAbilityByCall with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_013, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_002 start " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.dmsVersion = 0;
    callerInfo.accessToken = 0;
    DTEST_LOG << "DistributedSchedServiceTest mock illegal want " << std::endl;
    int32_t result = DistributedSchedService::GetInstance().TryStartRemoteAbilityByCall(mockWant,
        nullptr, callerInfo);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_013 end " << std::endl;
}

/**
 * @tc.name: CallAbility_014
 * @tc.desc: Call TryStartRemoteAbilityByCall with illegal callerInfo
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_014, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_002 start " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    CallerInfo callerInfo;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.dmsVersion = 0;
    callerInfo.accessToken = 0;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal uid " << std::endl;
    callerInfo.uid = -1;
    int32_t result = DistributedSchedService::GetInstance().TryStartRemoteAbilityByCall(mockWant,
        callback, callerInfo);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    callerInfo.uid = MOCK_UID;
    DTEST_LOG << "DistributedSchedServiceTest mock illegal sourceDeviceId " << std::endl;
    callerInfo.sourceDeviceId = "";
    result = DistributedSchedService::GetInstance().TryStartRemoteAbilityByCall(mockWant,
        callback, callerInfo);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    DTEST_LOG << "DistributedSchedServiceTest mock illegal duid " << std::endl;
    callerInfo.duid = -1;
    result = DistributedSchedService::GetInstance().TryStartRemoteAbilityByCall(mockWant,
        callback, callerInfo);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_014 end " << std::endl;
}

/**
 * @tc.name: CallAbility_015
 * @tc.desc: Call StartAbilityByCallFromRemote with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_015, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_015 start " << std::endl;
    OHOS::AAFwk::Want mockWant;
    mockWant.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 0;
    callerInfo.uid = MOCK_UID;
    callerInfo.pid = MOCK_PID;
    callerInfo.dmsVersion = 1000;
    callerInfo.accessToken = 0;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    IDistributedSched::AccountInfo accountInfo;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    std::string srcDeviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(srcDeviceId);
    OHOS::AAFwk::Want want;
    want.SetElementName(srcDeviceId, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> mockCallback = nullptr;
    
    int32_t result = DistributedSchedService::GetInstance().StartAbilityByCallFromRemote(want,
        mockCallback, callerInfo, accountInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_015 end " << std::endl;
}

/**
 * @tc.name: CallAbility_016
 * @tc.desc: Call StartAbilityByCall with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_016, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_016 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    sptr<IRemoteObject> callbackWrapper = new AbilityCallWrapperStubTest(callback);
    sptr<IRemoteObject> callerToken = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    sptr<IRemoteObject> mockCallbackWrapper = nullptr;
    int32_t result = DistributedSchedAdapter::GetInstance().StartAbilityByCall(want,
        mockCallbackWrapper, new AbilityCallCallbackTest());
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest mock illegal token " << std::endl;
    sptr<IRemoteObject> mockToken = nullptr;
    result = DistributedSchedAdapter::GetInstance().StartAbilityByCall(want,
        callbackWrapper, mockToken);
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_016 end " << std::endl;
}

/**
 * @tc.name: CallAbility_017
 * @tc.desc: Call StartAbilityByCall with illegal token
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_017, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_017 start " << std::endl;
    OHOS::AAFwk::Want want;
    want.SetElementName(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    sptr<IRemoteObject> callbackWrapper = new AbilityCallWrapperStubTest(callback);
    sptr<IRemoteObject> callerToken = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal token " << std::endl;
    sptr<IRemoteObject> mockToken = nullptr;
    int32_t result = DistributedSchedAdapter::GetInstance().StartAbilityByCall(want,
        callbackWrapper, mockToken);
    EXPECT_NE(result, 0);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_017 end " << std::endl;
}

/**
 * @tc.name: CallAbility_018
 * @tc.desc: Call ReleaseRemoteAbility with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_018, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_005 start " << std::endl;
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    sptr<IRemoteObject> mockCallbackWrapper = nullptr;
    int32_t result = DistributedSchedService::GetInstance().ReleaseRemoteAbility(mockCallbackWrapper,
        element);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_018 end " << std::endl;
}

/**
 * @tc.name: CallAbility_019
 * @tc.desc: Call ReleaseRemoteAbility with illegal remote deviceId
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_019, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_005 start " << std::endl;
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();

    DTEST_LOG << "DistributedSchedServiceTest get remote dms failed " << std::endl;
    int32_t result = DistributedSchedService::GetInstance().ReleaseRemoteAbility(callback,
        element);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_019 end " << std::endl;
}

/**
 * @tc.name: CallAbility_020
 * @tc.desc: Call ReleaseAbilityFromRemote with illegal callback
 * @tc.type: FUNC
 * @tc.require: AR000GK59A
 */
HWTEST_F(DistributedSchedCallTest, CallAbility_020, TestSize.Level1)
{
    DTEST_LOG << "DistributedSchedServiceTest CallAbility_020 start " << std::endl;
    sptr<IRemoteObject> callback = new AbilityCallCallbackTest();
    AppExecFwk::ElementName element(MOCK_DEVICE_ID, "ohos.demo.test", "abilityTest");
    CallerInfo callerInfo;
    callerInfo.callerType = CALLER_TYPE_NONE;
    callerInfo.duid = 1000;
    callerInfo.dmsVersion = 1000;
    callerInfo.uid = MOCK_UID;
    callerInfo.accessToken = 1000;
    callerInfo.pid = MOCK_PID;
    callerInfo.sourceDeviceId = MOCK_DEVICE_ID;

    DTEST_LOG << "DistributedSchedServiceTest mock illegal callback " << std::endl;
    sptr<IRemoteObject> mockCallbackWrapper = nullptr;
    int32_t result = DistributedSchedService::GetInstance().ReleaseAbilityFromRemote(mockCallbackWrapper,
        element, callerInfo);
    EXPECT_EQ(result, INVALID_REMOTE_PARAMETERS_ERR_CODE);

    DTEST_LOG << "DistributedSchedServiceTest CallAbility_020 end " << std::endl;
}

void DistributedSchedCallTest::AddSession(const sptr<IRemoteObject>& connect,
    const std::string& localDeviceId, const std::string& remoteDeviceId, const AAFwk::Want& want) const
{
    DTEST_LOG << "DistributedSchedServiceTest call AddSession " << std::endl;
    if (connect == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    CallerInfo callerInfo;
    callerInfo.uid = IPCSkeleton::GetCallingUid();
    callerInfo.pid = IPCSkeleton::GetCallingPid();
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    DistributedSchedService::GetInstance().RemoteConnectAbilityMappingLocked(connect, localDeviceId,
        remoteDeviceId, want.GetElement(), callerInfo, TargetComponent::HARMONY_COMPONENT);
}

void DistributedSchedCallTest::RemoveSession(const sptr<IRemoteObject>& connect) const
{
    DTEST_LOG << "DistributedSchedServiceTest call RemoveSession " << std::endl;
    if (connect == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest RemoveSession connect nullptr " << std::endl;
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    DistributedSchedService::GetInstance().distributedConnectAbilityMap_.erase(connect);
}

void DistributedSchedCallTest::AddConnectInfo(const sptr<IRemoteObject>& connect,
    const std::string& localDeviceId, const std::string& remoteDeviceId) const
{
    DTEST_LOG << "DistributedSchedServiceTest call AddConnectInfo " << std::endl;
    if (connect == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest AddConnectInfo connect nullptr " << std::endl;
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    CallerInfo callerInfo;
    callerInfo.uid = IPCSkeleton::GetCallingUid();
    callerInfo.pid = IPCSkeleton::GetCallingPid();
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.callerType = CALLER_TYPE_HARMONY;

    sptr<IRemoteObject> callbackWrapper = new AbilityCallWrapperStubTest(connect);
    ConnectInfo connectInfo {callerInfo, callbackWrapper};
    DistributedSchedService::GetInstance().connectAbilityMap_.emplace(connect, connectInfo);
}

void DistributedSchedCallTest::RemoveConnectInfo(const sptr<IRemoteObject>& connect) const
{
    DTEST_LOG << "DistributedSchedServiceTest call RemoveConnectInfo " << std::endl;
    if (connect == nullptr) {
        DTEST_LOG << "DistributedSchedServiceTest RemoveConnectInfo connect nullptr " << std::endl;
        return;
    }

    std::lock_guard<std::mutex> autoLock(DistributedSchedService::GetInstance().distributedLock_);
    DistributedSchedService::GetInstance().connectAbilityMap_.erase(connect);
}

void DistributedSchedCallTest::AddConnectCount(int32_t uid) const
{
    DTEST_LOG << "DistributedSchedServiceTest call AddConnectCount " << std::endl;
    if (uid < 0) {
        DTEST_LOG << "DistributedSchedServiceTest AddConnectCount uid < 0 " << std::endl;
        return;
    }

    auto& trackingUidMap = DistributedSchedService::GetInstance().trackingUidMap_;
    ++trackingUidMap[uid];
}

void DistributedSchedCallTest::DecreaseConnectCount(int32_t uid) const
{
    DTEST_LOG << "DistributedSchedServiceTest call DecreaseConnectCount " << std::endl;
    if (uid < 0) {
        DTEST_LOG << "DistributedSchedServiceTest DecreaseConnectCount uid < 0 " << std::endl;
        return;
    }

    DistributedSchedService::GetInstance().DecreaseConnectLocked(uid);
}
}
}