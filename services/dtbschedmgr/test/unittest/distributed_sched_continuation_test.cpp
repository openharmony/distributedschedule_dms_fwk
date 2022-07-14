/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "distributed_sched_continuation_test.h"
#include "dtbschedmgr_device_info_storage.h"
#include "mock_distributed_sched.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using string = std::string;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::u16string MOCK_DEVICE_ID = u"MOCK_DEVICE_ID";
constexpr int32_t MOCK_SESSION_ID = 123;
constexpr int32_t MOCK_TASK_ID = 456;
const string LOCAL_DEVICE_ID = "192.168.43.100";
}

void DSchedContinuationTest::SetUpTestCase()
{
}

void DSchedContinuationTest::TearDownTestCase()
{
}

void DSchedContinuationTest::SetUp()
{
    dschedContinuation_ = std::make_shared<DSchedContinuation>();
}

void DSchedContinuationTest::TearDown()
{
    dschedContinuation_ = nullptr;
}

sptr<IRemoteObject> DSchedContinuationTest::GetDSchedService() const
{
    sptr<IRemoteObject> dsched = new MockDistributedSched();
    return dsched;
}

int32_t DSchedContinuationTest::PushAbilityToken()
{
    FuncContinuationCallback continuationCallback = [this] (int32_t missionId) {
        timeoutFlag_ = true;
    };
    dschedContinuation_->Init(continuationCallback);
    int32_t sessionId = dschedContinuation_->GenerateSessionId();
    dschedContinuation_->PushAbilityToken(sessionId, GetDSchedService());
    return sessionId;
}

std::shared_ptr<Want> DSchedContinuationTest::MockWant(const string& bundleName, const string& ability, int32_t flags)
{
    ElementName element("", bundleName, ability);
    std::shared_ptr<Want> spWant = std::make_shared<Want>();
    spWant->SetElement(element);
    spWant->SetFlags(flags);
    return spWant;
}

sptr<IDistributedSched> DSchedContinuationTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    if (sm == nullptr) {
        DTEST_LOG << "DSchedContinuationTest sm is nullptr" << std::endl;
        return nullptr;
    }
    DTEST_LOG << "DSchedContinuationTest sm is not nullptr" << std::endl;
    auto distributedObject = sm->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(distributedObject != nullptr);
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "DSchedContinuationTest DistributedSched is nullptr" << std::endl;
    }
    DTEST_LOG << "DSchedContinuationTest DistributedSched is not nullptr" << std::endl;
    return proxy_;
}

int32_t DSchedContinuationTest::StartContinuation(int32_t missionId, int32_t flags)
{
    string bundleName = "bundleName";
    string abilityName = "abilityName";
    string devId = "devId";
    std::shared_ptr<Want> spWant = MockWant(bundleName, abilityName, flags);
    int callerUid = 0;
    return DistributedSchedService::GetInstance().StartContinuation(*spWant, missionId, callerUid, 0, 0);
}

int32_t DSchedContinuationTest::StartRemoteFreeInstall(int32_t flags, const sptr<IRemoteObject>& callback)
{
    string bundleName = "bundleName";
    string abilityName = "abilityName";
    string devId = "devId";
    std::shared_ptr<Want> spWant = MockWant(bundleName, abilityName, flags);
    int callerUid = 0;
    return DistributedSchedService::GetInstance().StartRemoteFreeInstall(*spWant, callerUid, 0, 0, callback);
}

/**
 * @tc.name: StartContinuation_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartContinuation_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartContinuation_001 start" << std::endl;
    /**
     * @tc.steps: step1. want not set continuation flags.
     * @tc.expected: step1. return false.
     */
    int32_t ret = StartContinuation(0, 0);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest StartContinuation002 end" << std::endl;
}

/**
 * @tc.name: StartContinuation_002
 * @tc.desc: get remote dms failed.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartContinuation_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartContinuation_002 start" << std::endl;
    /**
     * @tc.steps: step1. get remote dms failed.
     * @tc.expected: step1. return false.
     */
    int32_t ret = StartContinuation(0, Want::FLAG_ABILITY_CONTINUATION);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest StartContinuation003 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteContinuation_001
 * @tc.desc: input invalid session.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteContinuation_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteContinuation_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid session.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyCompleteContinuation(MOCK_DEVICE_ID, -1, true);
    EXPECT_TRUE(!timeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteContinuation_001 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteContinuation_002
 * @tc.desc: get remote dms failed.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteContinuation_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteContinuation_002 start" << std::endl;
    /**
     * @tc.steps: step1. get remote dms failed.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyCompleteContinuation(MOCK_DEVICE_ID, MOCK_SESSION_ID, true);
    EXPECT_TRUE(!timeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteContinuation_002 end" << std::endl;
}

/**
 * @tc.name: NotifyContinuationResultFromRemote_001
 * @tc.desc: input invalid session.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyContinuationResultFromRemote_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyContinuationResultFromRemote_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid session.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyContinuationResultFromRemote(-1, true);
    EXPECT_TRUE(!timeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyContinuationResultFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: NotifyContinuationResultFromRemote_002
 * @tc.desc: get remote dms failed.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyContinuationResultFromRemote_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyContinuationResultFromRemote_002 start" << std::endl;
    /**
     * @tc.steps: step1. get remote dms failed.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyContinuationResultFromRemote(MOCK_SESSION_ID, true);
    EXPECT_TRUE(!timeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyContinuationResultFromRemote_002 end" << std::endl;
}

/**
 * @tc.name: SetWantForContinuation_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, SetWantForContinuation_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest SetWantForContinuation_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid bundleName.
     * @tc.expected: step1. return err.
     */
    string bundleName = "bundleName";
    string abilityName = "abilityName";
    std::shared_ptr<Want> spWant = MockWant(bundleName, abilityName, 0);
    int32_t missionId = 0;
    int32_t ret = DistributedSchedService::GetInstance().SetWantForContinuation(*spWant, missionId);
    EXPECT_TRUE(INVALID_PARAMETERS_ERR == ret);
    DTEST_LOG << "DSchedContinuationTest SetWantForContinuation_001 end" << std::endl;
}

/**
 * @tc.name: SetWantForContinuation_002
 * @tc.desc: input valid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, SetWantForContinuation_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest SetWantForContinuation_002 start" << std::endl;
    /**
     * @tc.steps: step1. input valid bundleName.
     * @tc.expected: step1. return OK.
     */
    string bundleName = "ohos.samples.distributedcalc";
    string abilityName = "MainAbility";
    std::shared_ptr<Want> spWant = MockWant(bundleName, abilityName, 0);
    int32_t missionId = 0;
    int32_t ret = DistributedSchedService::GetInstance().SetWantForContinuation(*spWant, missionId);
    EXPECT_TRUE(ERR_OK == ret);
    DTEST_LOG << "DSchedContinuationTest SetWantForContinuation_002 end" << std::endl;
}

/**
 * @tc.name: ContinueLocalMission_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, ContinueLocalMission_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueLocalMission_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid missionId.
     * @tc.expected: step1. return err.
     */
    string deviceId = "123456";
    int32_t missionId = 0;
    auto callback = GetDSchedService();
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueLocalMission(deviceId,
        missionId, callback, wantParams);
    EXPECT_TRUE(INVALID_PARAMETERS_ERR == ret);
    DTEST_LOG << "DSchedContinuationTest ContinueLocalMission_001 end" << std::endl;
}

/**
 * @tc.name: ContinueLocalMission_002
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, ContinueLocalMission_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueLocalMission_002 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid mission.
     * @tc.expected: step1. return err.
     */
    string deviceId = "123456";
    int32_t missionId = 0;
    auto callback = GetDSchedService();
    WantParams wantParams;
    DistributedSchedService::GetInstance().OnStart();
    if (DistributedSchedService::GetInstance().dschedContinuation_ == nullptr) {
        return;
    }
    DistributedSchedService::GetInstance().dschedContinuation_->PushCallback(missionId, callback, deviceId, false);
    int32_t ret = DistributedSchedService::GetInstance().ContinueLocalMission(
        deviceId, missionId, callback, wantParams);
    EXPECT_TRUE(INVALID_PARAMETERS_ERR == ret);
    DTEST_LOG << "DSchedContinuationTest ContinueLocalMission_002 end" << std::endl;
}

/**
 * @tc.name: ContinueRemoteMission_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, ContinueRemoteMission_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueRemoteMission_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid deviceId.
     * @tc.expected: step1. return err.
     */
    string srcDeviceId = "123456";
    string dstDeviceid = "123456";
    int32_t missionId = 0;
    auto callback = GetDSchedService();
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueRemoteMission(
        srcDeviceId, dstDeviceid, missionId, callback, wantParams);
    EXPECT_TRUE(INVALID_REMOTE_PARAMETERS_ERR == ret);
    DTEST_LOG << "DSchedContinuationTest ContinueRemoteMission_001 end" << std::endl;
}

/**
 * @tc.name: ContinueRemoteMission_002
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, ContinueRemoteMission_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueRemoteMission_002 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid param.
     * @tc.expected: step1. return err.
     */
    string srcDeviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(srcDeviceId);
    string dstDeviceid = "123456";
    int32_t missionId = 0;
    auto callback = GetDSchedService();
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueRemoteMission(
        srcDeviceId, dstDeviceid, missionId, callback, wantParams);
    EXPECT_TRUE(ERR_OK != ret);
    DTEST_LOG << "DSchedContinuationTest ContinueRemoteMission_002 end" << std::endl;
}

/**
 * @tc.name: PushAbilityToken_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PushAbilityToken_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid abilityToken.
     * @tc.expected: step1. return false.
     */
    auto sessionId = dschedContinuation_->GenerateSessionId();
    bool ret = dschedContinuation_->PushAbilityToken(sessionId, nullptr);
    EXPECT_TRUE(!ret);
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_001 end" << std::endl;
}

/**
 * @tc.name: PushAbilityToken_002
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PushAbilityToken_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_002 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid sessionId.
     * @tc.expected: step1. return false.
     */
    bool ret = dschedContinuation_->PushAbilityToken(-1, GetDSchedService());
    EXPECT_TRUE(!ret);
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_002 end" << std::endl;
}

/**
 * @tc.name: PushAbilityToken_003
 * @tc.desc: init not call.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PushAbilityToken_003, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_003 start" << std::endl;
    /**
     * @tc.steps: step1. input valid abilityToken and valid sessionId.
     * @tc.expected: step1. return false.
     */
    auto sessionId = dschedContinuation_->GenerateSessionId();
    bool ret = dschedContinuation_->PushAbilityToken(sessionId, GetDSchedService());
    EXPECT_TRUE(!ret);
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_003 end" << std::endl;
}

/**
 * @tc.name: PushAbilityToken_004
 * @tc.desc: Push AbilityToken OK.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PushAbilityToken_004, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_004 start" << std::endl;
    /**
     * @tc.steps: step1. input valid params and init.
     * @tc.expected: step1. return true.
     */
    dschedContinuation_->Init(nullptr);
    auto sessionId = dschedContinuation_->GenerateSessionId();
    bool ret = dschedContinuation_->PushAbilityToken(sessionId, GetDSchedService());
    EXPECT_TRUE(ret);
    DTEST_LOG << "DSchedContinuationTest PushAbilityToken_004 end" << std::endl;
}

/**
 * @tc.name: PopAbilityToken_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PopAbilityToken_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid sessionId.
     * @tc.expected: step1. return false.
     */
    sptr<IRemoteObject> abilityToken = dschedContinuation_->PopAbilityToken(-1);
    EXPECT_TRUE(abilityToken == nullptr);
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_001 end" << std::endl;
}

/**
 * @tc.name: PopAbilityToken_002
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PopAbilityToken_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_002 start" << std::endl;
    /**
     * @tc.steps: step1. pop not exist sessionId.
     * @tc.expected: step1. return false.
     */
    int32_t sessionId = dschedContinuation_->GenerateSessionId() + 1;
    sptr<IRemoteObject> abilityToken = dschedContinuation_->PopAbilityToken(sessionId);
    EXPECT_TRUE(abilityToken == nullptr);
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_002 end" << std::endl;
}

/**
 * @tc.name: PopAbilityToken_003
 * @tc.desc: pop abilityToken success.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, PopAbilityToken_003, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_003 start" << std::endl;
    /**
     * @tc.steps: step1. pop exist sessionId.
     * @tc.expected: step1. return true.
     */
    int32_t sessionId =PushAbilityToken();
    sptr<IRemoteObject> abilityToken = dschedContinuation_->PopAbilityToken(sessionId);
    EXPECT_TRUE(abilityToken != nullptr);

    /**
     * @tc.steps: step2. duplicate pop abilityToken.
     * @tc.expected: step1. return false.
     */
    abilityToken = dschedContinuation_->PopAbilityToken(sessionId);
    EXPECT_TRUE(abilityToken == nullptr);
    DTEST_LOG << "DSchedContinuationTest PopAbilityToken_003 end" << std::endl;
}

/**
 * @tc.name: ContinueMission_001
 * @tc.desc: test ContinueMission when srcDeviceId is empty.
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(DSchedContinuationTest, ContinueMission_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueMission_001 start" << std::endl;
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueMission("",
        "string", 1, GetDSchedService(), wantParams);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest ContinueMission_001 end" << std::endl;
}

/**
 * @tc.name: ContinueMission_002
 * @tc.desc: test ContinueMission when dstDeviceId is empty.
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(DSchedContinuationTest, ContinueMission_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueMission_002 start" << std::endl;
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueMission("string",
        "", 1, GetDSchedService(), wantParams);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest ContinueMission_002 end" << std::endl;
}

/**
 * @tc.name: ContinueMission_003
 * @tc.desc: test ContinueMission when callback is nullptr.
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(DSchedContinuationTest, ContinueMission_003, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueMission_003 start" << std::endl;
    WantParams wantParams;
    int32_t ret = DistributedSchedService::GetInstance().ContinueMission("string", "string", 1, nullptr, wantParams);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest ContinueMission_003 end" << std::endl;
}

/**
 * @tc.name: ContinueMission_004
 * @tc.desc: test ContinueMission when srcDeviceId == localDevId.
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(DSchedContinuationTest, ContinueMission_004, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest ContinueMission_004 start" << std::endl;
    WantParams wantParams;

    std::string srcDeviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(srcDeviceId);
    int32_t ret = DistributedSchedService::GetInstance().ContinueMission(srcDeviceId,
        "string", 1, GetDSchedService(), wantParams);
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest ContinueMission_004 end" << std::endl;
}

/**
 * @tc.name: StartRemoteFreeInstall_001
 * @tc.desc: input invalid params.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartRemoteFreeInstall_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartRemoteFreeInstall_001 start" << std::endl;
    /**
     * @tc.steps: step1. want not set continuation flags.
     * @tc.expected: step1. return false.
     */
    int32_t ret = StartRemoteFreeInstall(0, GetDSchedService());
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest StartRemoteFreeInstall_001 end" << std::endl;
}

/**
 * @tc.name: StartRemoteFreeInstall_002
 * @tc.desc: get remote dms failed.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartRemoteFreeInstall_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartRemoteFreeInstall_002 start" << std::endl;
    /**
     * @tc.steps: step1. get remote dms failed.
     * @tc.expected: step1. return false.
     */
    int32_t ret = StartRemoteFreeInstall(Want::FLAG_ABILITY_CONTINUATION, GetDSchedService());
    EXPECT_TRUE(ret != ERR_OK);
    DTEST_LOG << "DSchedContinuationTest StartRemoteFreeInstall_002 end" << std::endl;
}

/**
 * @tc.name: StartFreeInstallFromRemote_001
 * @tc.desc: call StartFreeInstallFromRemote with illegal param
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartFreeInstallFromRemote_001, TestSize.Level0)
{
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_001 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    AAFwk::Want want;
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    IDistributedSched::FreeInstallInfo info = {.want = want,
        .requestCode = 0,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo
    };
    /**
     * @tc.steps: step1. StartFreeInstallFromRemote with uninitialized params
     * @tc.expected: step1. StartFreeInstallFromRemote return INVALID_REMOTE_PARAMETERS_ERR
     */
    int result1 = proxy->StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result1:" << result1 << std::endl;
    /**
     * @tc.steps: step1. StartFreeInstallFromRemote with with empty deviceId
     * @tc.expected: step1. StartFreeInstallFromRemote return INVALID_REMOTE_PARAMETERS_ERR
     */
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    int result2 = proxy->StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result2:" << result2 << std::endl;
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: StartFreeInstallFromRemote_002
 * @tc.desc: call StartFreeInstallFromRemote
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartFreeInstallFromRemote_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_002 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }

    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    IDistributedSched::FreeInstallInfo info = {.want = want,
        .requestCode = 0,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo
    };

    int result1 = proxy->StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result1 is" << result1 << std::endl;

    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    int result2 = proxy->StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result2:" << result2 << std::endl;
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_002 end" << std::endl;
}

/**
 * @tc.name: StartFreeInstallFromRemote_003
 * @tc.desc: call StartFreeInstallFromRemote for pressure test
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartFreeInstallFromRemote_003, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_003 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    /**
     * @tc.steps: step1. set want and abilityInfo
     */
    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    IDistributedSched::FreeInstallInfo info = {.want = want,
        .requestCode = 0,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo
    };
    /**
     * @tc.steps: step2. StartFreeInstallFromRemote for pressure test
     * @tc.expected: step2. StartFreeInstallFromRemote for result
     */
    for (int index = 0; index < static_cast<int32_t>(LoopTime::LOOP_TIME); index++) {
        int result = proxy->StartFreeInstallFromRemote(info, 0);
        DTEST_LOG << "pressure" + std::to_string(index) + " result is " << result << std::endl;
    }
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_003 end" << std::endl;
}

/**
 * @tc.name: StartFreeInstallFromRemote_004
 * @tc.desc: call StartFreeInstallFromRemote with dms
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, StartFreeInstallFromRemote_004, TestSize.Level0)
{
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_004 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();

    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    IDistributedSched::FreeInstallInfo info = {.want = want,
        .requestCode = 0,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo
    };

    int result1 = DistributedSchedService::GetInstance().StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result1:" << result1 << std::endl;

    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    int result2 = DistributedSchedService::GetInstance().StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result2:" << result2 << std::endl;
    EXPECT_EQ(static_cast<int>(INVALID_REMOTE_PARAMETERS_ERR), result1);
    EXPECT_EQ(static_cast<int>(INVALID_REMOTE_PARAMETERS_ERR), result2);
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_004 end" << std::endl;
}

/**
 * @tc.name: StartFreeInstallFromRemote_005
 * @tc.desc: call StartFreeInstallFromRemote with dms
 * @tc.type: FUNC
 * @tc.require: SR000GKT4A
 */
HWTEST_F(DSchedContinuationTest, StartFreeInstallFromRemote_005, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_005 start" << std::endl;
    sptr<IDistributedSched> proxy = GetDms();

    AAFwk::Want want;
    AppExecFwk::ElementName element("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbility");
    want.SetElement(element);
    CallerInfo callerInfo;
    callerInfo.uid = 0;
    callerInfo.sourceDeviceId = "255.255.255.255";
    IDistributedSched::AccountInfo accountInfo;
    accountInfo.accountType = 1;
    accountInfo.groupIdList.push_back("123456");
    IDistributedSched::FreeInstallInfo info = {.want = want,
        .requestCode = 0,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo
    };

    int result1 = DistributedSchedService::GetInstance().StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result1:" << result1 << std::endl;

    AppExecFwk::ElementName element2("", "com.ohos.distributedmusicplayer",
        "com.ohos.distributedmusicplayer.MainAbilityService");
    want.SetElement(element2);
    int result2 = DistributedSchedService::GetInstance().StartFreeInstallFromRemote(info, 0);
    DTEST_LOG << "result2:" << result2 << std::endl;
    DTEST_LOG << "DSchedContinuationTest StartFreeInstallFromRemote_005 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteFreeInstall_001
 * @tc.desc: input invalid taskId.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteFreeInstall_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstall_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid taskId.
     * @tc.expected: step1. return false.
     */
    IDistributedSched::FreeInstallInfo info;
    DistributedSchedService::GetInstance().NotifyCompleteFreeInstall(info, -1, 0);
    EXPECT_TRUE(!freeInstallTimeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstall_001 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteFreeInstall_002
 * @tc.desc: get remote dms failed.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteFreeInstall_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstall_002 start" << std::endl;
    /**
     * @tc.steps: step1. get remote dms failed.
     * @tc.expected: step1. return false.
     */
    IDistributedSched::FreeInstallInfo info;
    DistributedSchedService::GetInstance().NotifyCompleteFreeInstall(info, MOCK_TASK_ID, 0);
    EXPECT_TRUE(!freeInstallTimeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstall_002 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteFreeInstallFromRemote_001
 * @tc.desc: input invalid taskId.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteFreeInstallFromRemote_001, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstallFromRemote_001 start" << std::endl;
    /**
     * @tc.steps: step1. input invalid taskId.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyCompleteFreeInstallFromRemote(-1, 0);
    EXPECT_TRUE(!freeInstallTimeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstallFromRemote_001 end" << std::endl;
}

/**
 * @tc.name: NotifyCompleteFreeInstallFromRemote_002
 * @tc.desc: dmsCallbackTask_ or dschedContinuation_ is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(DSchedContinuationTest, NotifyCompleteFreeInstallFromRemote_002, TestSize.Level1)
{
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstallFromRemote_002 start" << std::endl;
    /**
     * @tc.steps: step1. dmsCallbackTask_ or dschedContinuation_ is nullptr.
     * @tc.expected: step1. return false.
     */
    DistributedSchedService::GetInstance().NotifyCompleteFreeInstallFromRemote(MOCK_TASK_ID, 0);
    EXPECT_TRUE(!freeInstallTimeoutFlag_);
    DTEST_LOG << "DSchedContinuationTest NotifyCompleteFreeInstallFromRemote_002 end" << std::endl;
}
} // DistributedSchedule
} // namespace OHOS
