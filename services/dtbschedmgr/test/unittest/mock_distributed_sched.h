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

#ifndef MOCK_DISTRIBUTED_SCHED_H
#define MOCK_DISTRIBUTED_SCHED_H

#include "distributed_sched_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class MockDistributedSched : public DistributedSchedStub {
public:
    explicit MockDistributedSched(bool expectedTrue = false)
    {
        expectedTrue_ = expectedTrue;
    }
    ~MockDistributedSched() = default;

    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode) override;
    int32_t StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo) override;
    int32_t ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams) override;
    int32_t StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status) override;
    void NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess) override;
    int32_t NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess) override;
    int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
        const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid) override;
    int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect) override;
    int32_t ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo) override;
    int32_t DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId) override;
    int32_t NotifyProcessDiedFromRemote(const CallerInfo& callerInfo) override;
    int32_t StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    int32_t RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) override;
    int32_t UnRegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) override;
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions,
        std::vector<DstbMissionInfo>& missionInfos) override;
    int32_t StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
        const uint8_t* byteStream, size_t len) override;
    int32_t RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId) override;
    int32_t NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
        const CallerInfo& callerInfo) override;
    int32_t CheckSupportOsd(const std::string& deviceId) override;
    void GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values) override;
    int32_t GetOsdSwitchValueFromRemote() override;
    int32_t UpdateOsdSwitchValueFromRemote(int32_t switchVal, const std::string& sourceDeviceId) override;
    int32_t StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
        std::vector<DstbMissionInfo>& missionInfos) override;
    int32_t StopSyncMissionsFromRemote(const CallerInfo& callerInfo) override;
    int32_t StopSyncRemoteMissions(const std::string& devId) override;
private:
    bool expectedTrue_ = false;
};
} // namespace DistributedSchedule
} // namespace OHOS

#endif // MOCK_DISTRIBUTED_SCHED_H
