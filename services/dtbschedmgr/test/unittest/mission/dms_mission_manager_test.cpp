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

#include "dms_mission_manager_test.h"

#include "distributed_sched_proxy.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_device_info_storage.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mission/mission_constant.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "test_log.h"

#define private public
#include "mission/distributed_sched_mission_manager.h"

using namespace std;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string DEVICE_NAME = "DEVICE_PHONE_001";
const std::string DEVICE_ID = "123456789ABCD";
const std::string BUNDLE_NAME = "ohos.test.test";
const int32_t NUM_MISSIONS = 100;
}
void DMSMissionManagerTest::SetUpTestCase()
{
}

void DMSMissionManagerTest::TearDownTestCase()
{
}

void DMSMissionManagerTest::SetUp()
{
    string localDeviceId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        DTEST_LOG << "getLocalDevicesId failed!" << std::endl;
        return;
    }
    localDeviceId_ = localDeviceId;
}

void DMSMissionManagerTest::TearDown()
{
}

sptr<IDistributedSched> DMSMissionManagerTest::GetDms()
{
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sm != nullptr);
    if (sm == nullptr) {
        DTEST_LOG << "DMSMissionManagerTest sm is nullptr" << std::endl;
        return nullptr;
    }
    DTEST_LOG << "DMSMissionManagerTest sm is not nullptr" << std::endl;
    auto distributedObject = sm->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    EXPECT_TRUE(distributedObject != nullptr);
    proxy_ = iface_cast<IDistributedSched>(distributedObject);
    if (proxy_ == nullptr) {
        DTEST_LOG << "DMSMissionManagerTest DistributedSched is nullptr" << std::endl;
    }
    DTEST_LOG << "DMSMissionManagerTest DistributedSched is not nullptr" << std::endl;
    return proxy_;
}

/**
 * @tc.name: testGetRemoteMissionInfo001
 * @tc.desc: get remote mission info
 * @tc.type: FUNC
 * @tc.require:AR000GK66R
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionInfo001, TestSize.Level1)
{
    std::vector<AAFwk::MissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(localDeviceId_, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos("", 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(DEVICE_ID, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(localDeviceId_, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testGetRemoteMissionInfo002
 * @tc.desc: get remote mission info
 * @tc.type: FUNC
 * @tc.require:AR000GK66R
 */
HWTEST_F(DMSMissionManagerTest, testGetRemoteMissionInfo002, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    std::vector<AAFwk::MissionInfo> infos;
    auto ret = proxy->GetMissionInfos(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(localDeviceId_, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos("", 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(DEVICE_ID, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->GetMissionInfos(localDeviceId_, -1, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testStartSyncRemoteMissions001
 * @tc.desc: prepare and sync missions from remote
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testStartSyncRemoteMissions001, TestSize.Level1)
{
    sptr<IDistributedSched> proxy = GetDms();
    if (proxy == nullptr) {
        return;
    }
    auto ret = proxy->StartSyncRemoteMissions(DEVICE_ID, false, 0);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->StartSyncRemoteMissions(localDeviceId_, false, 0);
    EXPECT_TRUE(ret != ERR_NONE);

    ret = proxy->StartSyncRemoteMissions("", false, 0);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testIsDeviceIdValidated001
 * @tc.desc: judging whether the deviceId is valid
 * @tc.type: FUNC
 * @tc.require:AR000GK66R
 */
HWTEST_F(DMSMissionManagerTest, testIsDeviceIdValidated001, TestSize.Level1)
{
    std::string emptyDeviceId = "";
    auto ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(emptyDeviceId);
    EXPECT_FALSE(ret);

    ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(DEVICE_ID);
    EXPECT_FALSE(ret);

    ret = DistributedSchedMissionManager::GetInstance().IsDeviceIdValidated(localDeviceId_);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: testGetMissionInfos003
 * @tc.desc: get remote missions
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testGetMissionInfos001, TestSize.Level1)
{
    std::string deviceid = DEVICE_ID;
    int32_t numMissions = NUM_MISSIONS;
    std::vector<AAFwk::MissionInfo> missionInfos;

    auto ret = DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceid, numMissions, missionInfos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testFetchCachedRemoteMissions001
 * @tc.desc: test stop fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testFetchCachedRemoteMissions001, TestSize.Level1)
{
    std::vector<DstbMissionInfo> infos;
    auto ret = DistributedSchedMissionManager::GetInstance().FetchCachedRemoteMissions(DEVICE_ID, 0, infos);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testFetchCachedRemoteMissions001
 * @tc.desc: test check support osd fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testCheckSupportOsd001, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().CheckSupportOsd(DEVICE_ID);
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testFetchCachedRemoteMissions002
 * @tc.desc: test check support osd fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testCheckSupportOsd002, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().CheckSupportOsd(DEVICE_ID);
    EXPECT_TRUE(ret != ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().CheckSupportOsd("");
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: GetOsdSwitchValueFromRemote001
 * @tc.desc: test GetOsdSwitchValueFromRemote, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, GetOsdSwitchValueFromRemote001, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().GetOsdSwitchValueFromRemote();
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testUpdateOsdSwitchValueFromRemote001
 * @tc.desc: test UpdateOsdSwitchValueFromRemote fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testUpdateOsdSwitchValueFromRemote001, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().UpdateOsdSwitchValueFromRemote(0, DEVICE_ID);
    EXPECT_TRUE(ret == ERR_NONE);
}

/**
 * @tc.name: testUpdateOsdSwitchValueFromRemote002
 * @tc.desc: test UpdateOsdSwitchValueFromRemote fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testUpdateOsdSwitchValueFromRemote002, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().UpdateOsdSwitchValueFromRemote(0, DEVICE_ID);
    EXPECT_TRUE(ret == ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().UpdateOsdSwitchValueFromRemote(1, DEVICE_ID);
    EXPECT_TRUE(ret == ERR_NONE);
}

/**
 * @tc.name: testUpdateOsdSwitchValueFromRemote003
 * @tc.desc: test UpdateOsdSwitchValueFromRemote fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testUpdateOsdSwitchValueFromRemote003, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().UpdateOsdSwitchValueFromRemote(0, "");
    EXPECT_TRUE(ret == ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().UpdateOsdSwitchValueFromRemote(1, "");
    EXPECT_TRUE(ret == ERR_NONE);
}

/**
 * @tc.name: testUpdateSwitchValueToRemote001
 * @tc.desc: test UpdateSwitchValueToRemote, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testUpdateSwitchValueToRemote001, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().UpdateSwitchValueToRemote();
    EXPECT_TRUE(ret != ERR_NONE);
}

/**
 * @tc.name: testUpdateSwitchValueToRemote002
 * @tc.desc: test check support osd fake device, return error code
 * @tc.type: FUNC
 * @tc.require:AR000GK67M
 */
HWTEST_F(DMSMissionManagerTest, testUpdateSwitchValueToRemote002, TestSize.Level1)
{
    auto ret = DistributedSchedMissionManager::GetInstance().UpdateSwitchValueToRemote();
    EXPECT_TRUE(ret != ERR_NONE);
    ret = DistributedSchedMissionManager::GetInstance().UpdateSwitchValueToRemote();
    EXPECT_TRUE(ret != ERR_NONE);
}
} // namespace DistributedSchedule
} // namespace OHOS
