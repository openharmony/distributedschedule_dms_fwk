/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "mock_distributed_sched.h"
#include "dtbschedmgr_log.h"
#include "string_ex.h"

using namespace std;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
int32_t MockDistributedSched::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode, uint32_t accessToken)
{
    return 0;
}

int32_t MockDistributedSched::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
    const AccountInfo& accountInfo)
{
    return 0;
}

int32_t MockDistributedSched::SendResultFromRemote(OHOS::AAFwk::Want& want, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo, int32_t resultCode)
{
    return 0;
}

int32_t MockDistributedSched::ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    return 0;
}

int32_t MockDistributedSched::StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId,
    int32_t callerUid, int32_t status, uint32_t accessToken)
{
    return 0;
}

void MockDistributedSched::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
    (void)isSuccess;
}

int32_t MockDistributedSched::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    (void)isSuccess;
    return 0;
}

int32_t MockDistributedSched::ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
    int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    return 0;
}

int32_t MockDistributedSched::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid,
    uint32_t accessToken)
{
    return 0;
}

int32_t MockDistributedSched::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    return 0;
}

int32_t MockDistributedSched::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    return 0;
}

int32_t MockDistributedSched::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    return 0;
}

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
int32_t MockDistributedSched::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::GetRemoteMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    std::unique_ptr<MissionSnapshot>& missionSnapshot)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
    const CallerInfo& callerInfo)
{
    expectedTrue_ = false;
    return ERR_NONE;
}

int32_t MockDistributedSched::GetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<MissionInfo>& missionInfos)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
    std::vector<DstbMissionInfo>& missionInfos)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StopSyncMissionsFromRemote(const CallerInfo& callerInfo)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StopSyncRemoteMissions(const std::string& devId)
{
    return ERR_NONE;
}
#endif

int32_t MockDistributedSched::StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::ReleaseRemoteAbility(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StartAbilityByCallFromRemote(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::ReleaseAbilityFromRemote(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element, const CallerInfo& callerInfo)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::RegisterDistributedComponentListener(const sptr<IRemoteObject>& callback)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::GetDistributedComponentList(std::vector<std::string>& distributedComponents)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StartRemoteFreeInstall(const OHOS::AAFwk::Want &want, int32_t callerUid,
    int32_t requestCode, uint32_t accessToken, const sptr<IRemoteObject> &callback)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::StartFreeInstallFromRemote(const IDistributedSched::FreeInstallInfo &info, int64_t taskId)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::NotifyCompleteFreeInstallFromRemote(int64_t taskId, int32_t resultCode)
{
    return ERR_NONE;
}

int32_t MockDistributedSched::NotifyCompleteFreeInstall(
    const IDistributedSched::FreeInstallInfo &info, int64_t taskId, int32_t resultCode)
{
    return ERR_NONE;
}
} // namespace DistributedSchedule
} // namespace OHOS