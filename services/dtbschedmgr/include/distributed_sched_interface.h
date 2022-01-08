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

#ifndef OHOS_DISTRIBUTED_SCHED_INTERFACE_H
#define OHOS_DISTRIBUTED_SCHED_INTERFACE_H

#include <vector>
#include "ability_info.h"
#include "caller_info.h"
#include "iremote_broker.h"
#include "mission/distributed_mission_info.h"
#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace DistributedSchedule {
class IDistributedSched : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedSchedule.IDistributedSched");
    enum {
        SAME_ACCOUNT_TYPE = 0,
        DIFF_ACCOUNT_TYPE,
    };
    struct AccountInfo {
        int32_t accountType = SAME_ACCOUNT_TYPE;
        std::vector<std::string> groupIdList;
    };

    virtual int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode) = 0;
    virtual int32_t StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo) = 0;
    virtual int32_t ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams) = 0;
    virtual int32_t StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status) = 0;
    virtual void NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess) = 0;
    virtual int32_t NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess) = 0;
    virtual int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
        const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid) = 0;
    virtual int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect) = 0;
    virtual int32_t ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo) = 0;
    virtual int32_t DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId) = 0;
    virtual int32_t NotifyProcessDiedFromRemote(const CallerInfo& callerInfo) = 0;
    virtual int32_t StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) = 0;
    virtual int32_t StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
        std::vector<DstbMissionInfo>& missionInfos) = 0;
    virtual int32_t StopSyncRemoteMissions(const std::string& devId) = 0;
    virtual int32_t StopSyncMissionsFromRemote(const CallerInfo& callerInfo) = 0;
    virtual int32_t RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) = 0;
    virtual int32_t UnRegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) = 0;
    virtual int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions,
        std::vector<DstbMissionInfo>& missionInfos) = 0;
    virtual int32_t StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
        const uint8_t* byteStream, size_t len) = 0;
    virtual int32_t RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId) = 0;
    virtual int32_t NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
         const CallerInfo& callerInfo) = 0;
    virtual int32_t CheckSupportOsd(const std::string& deviceId) = 0;
    virtual void GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values) = 0;
    virtual int32_t GetOsdSwitchValueFromRemote() = 0;
    virtual int32_t UpdateOsdSwitchValueFromRemote(int32_t switchVal, const std::string& sourceDeviceId) = 0;
    enum {
        START_REMOTE_ABILITY = 1,
        STOP_REMOTE_ABILITY = 3,
        START_ABILITY_FROM_REMOTE = 4,
        STOP_ABILITY_FROM_REMOTE = 5,
        CONNECT_REMOTE_ABILITY = 6,
        DISCONNECT_REMOTE_ABILITY = 7,
        CONNECT_ABILITY_FROM_REMOTE = 8,
        DISCONNECT_ABILITY_FROM_REMOTE = 9,
        START_CONTINUATION = 11,
        NOTIFY_COMPLETE_CONTINUATION = 12,
        NOTIFY_CONTINUATION_RESULT_FROM_REMOTE = 13,
        REGISTER_ABILITY_TOKEN = 14,
        UNREGISTER_ABILITY_TOKEN = 15,
        CONTINUE_ABILITY = 16,
        NOTIFY_PROCESS_DIED_FROM_REMOTE = 17,
        GET_REMOTE_APPTHREAD = 35,
        CONTINUE_MISSION = 36,

        // request code for mission
        GET_MISSION_INFOS = 80,
        STORE_SNAPSHOT_INFO = 81,
        GET_REMOTE_SNAPSHOT_INFO = 82,
        REMOVE_SNAPSHOT_INFO = 83,
        REGISTER_MISSION_LISTENER = 84,
        UNREGISTER_MISSION_LISTENER = 85,
        START_SYNC_MISSIONS_FROM_REMOTE = 86,
        STOP_SYNC_MISSIONS_FROM_REMOTE = 87,
        CHECK_SUPPORTED_OSD = 88,
        CHECK_SUPPORT_OSD_FROM_REMOTE = 89,
        MISSION_CHANGED = 90,
        NOTIFY_MISSIONS_CHANGED_FROM_REMOTE = 91,
        START_SYNC_MISSIONS = 92,
        TRY_OPENP2PSESSION_FROM_REMOTE = 93,
        SWITCH_CHANGED = 94,
        NOTIFY_SWITCH_CHANGED_FROM_REMOTE = 95,
        GET_CACHED_SUPPORTED_OSD = 96,
        ALL_CONNECT_TO_DMS = 97,
        STOP_SYNC_MISSIONS = 98,
    };
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_INTERFACE_H
