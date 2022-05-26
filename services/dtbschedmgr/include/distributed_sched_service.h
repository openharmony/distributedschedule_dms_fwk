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

#ifndef OHOS_DISTRIBUTED_SCHED_SERVICE_H
#define OHOS_DISTRIBUTED_SCHED_SERVICE_H

#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>

#include "distributed_sched_stub.h"
#include "distributed_sched_continuation.h"
#include "dms_callback_task.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
#include "mission/distributed_mission_info.h"
#include "nocopyable.h"
#endif
#include "single_instance.h"
#include "system_ability.h"

namespace OHOS {
namespace DistributedSchedule {
class ConnectAbilitySession;

enum class ServiceRunningState {
    STATE_NO_START,
    STATE_RUNNING
};

enum class TargetComponent {
    HARMONY_COMPONENT,
};

struct ConnectInfo {
    CallerInfo callerInfo;
    sptr<IRemoteObject> callbackWrapper;
    AppExecFwk::ElementName element;
};

struct ProcessDiedNotifyInfo {
    std::string remoteDeviceId;
    CallerInfo callerInfo;
    TargetComponent targetComponent;
};

class DistributedSchedService : public SystemAbility, public DistributedSchedStub {
DECLARE_SYSTEM_ABILITY(DistributedSchedService);

DECLARE_SINGLE_INSTANCE_BASE(DistributedSchedService);

public:
    ~DistributedSchedService() = default;
    void OnStart() override;
    void OnStop() override;
    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken) override;
    int32_t StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo) override;
    int32_t ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams) override;
    int32_t StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status, uint32_t accessToken) override;
    void NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess) override;
    int32_t NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess) override;
    int32_t NotifyFreeInstallResult(const CallbackTaskItem item, int32_t resultCode);
    int32_t ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken) override;
    int32_t DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid,
        uint32_t accessToken) override;
    int32_t ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo) override;
    int32_t DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId) override;
    int32_t NotifyProcessDiedFromRemote(const CallerInfo& callerInfo) override;
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions,
        std::vector<AAFwk::MissionInfo>& missionInfos) override;
    int32_t StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
        const uint8_t* byteStream, size_t len) override;
    int32_t RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId) override;
    int32_t NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
        const CallerInfo& callerInfo) override;
#endif
    void ProcessConnectDied(const sptr<IRemoteObject>& connect);
    void ProcessDeviceOffline(const std::string& deviceId);
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void DumpConnectInfo(std::string& info);
    void DumpSessionsLocked(const std::list<ConnectAbilitySession>& sessionsList, std::string& info);
    void DumpElementLocked(const std::list<AppExecFwk::ElementName>& elementsList, std::string& info);
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    int32_t CheckSupportOsd(const std::string& deviceId) override;
    void GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values) override;
    int32_t GetOsdSwitchValueFromRemote() override;
    int32_t UpdateOsdSwitchValueFromRemote(int32_t switchVal, const std::string& sourceDeviceId) override;
    std::unique_ptr<Snapshot> GetRemoteSnapshotInfo(const std::u16string& deviceId, int32_t missionId) override;
    int32_t GetRemoteMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
        std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot) override;
    int32_t StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    int32_t StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
        std::vector<DstbMissionInfo>& missionInfos) override;
    int32_t StopSyncRemoteMissions(const std::string& devId) override;
    int32_t StopSyncMissionsFromRemote(const CallerInfo& callerInfo) override;
    int32_t RegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) override;
    int32_t UnRegisterMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj) override;
#endif
    int32_t StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken) override;
    int32_t ReleaseRemoteAbility(const sptr<IRemoteObject>& connect,
        const AppExecFwk::ElementName &element) override;
    int32_t StartAbilityByCallFromRemote(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo) override;
    int32_t ReleaseAbilityFromRemote(const sptr<IRemoteObject>& connect, const AppExecFwk::ElementName &element,
        const CallerInfo& callerInfo) override;
    void ProcessCallerDied(const sptr<IRemoteObject>& connect, int32_t deviceType);
    void ProcessCalleeDied(const sptr<IRemoteObject>& connect);
    int32_t StartRemoteFreeInstall(const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, const sptr<IRemoteObject>& callback) override;
    int32_t StartFreeInstallFromRemote(const FreeInstallInfo& info, int64_t taskId) override;
    int32_t NotifyCompleteFreeInstallFromRemote(int64_t taskId, int32_t resultCode) override;
    int32_t NotifyCompleteFreeInstall(const FreeInstallInfo& info, int64_t taskId, int32_t resultCode);
    int32_t RegisterDistributedComponentListener(const sptr<IRemoteObject>& callback) override;
    int32_t GetDistributedComponentList(std::vector<std::string>& distributedComponents) override;
private:
    DistributedSchedService();
    bool Init();
    void NotifyContinuationCallbackResult(int32_t missionId, int32_t isSuccess);
    void RemoteConnectAbilityMappingLocked(const sptr<IRemoteObject>& connect, const std::string& localDeviceId,
        const std::string& remoteDeviceId, const AppExecFwk::ElementName& element, const CallerInfo& callerInfo,
        TargetComponent targetComponent);
    int32_t DisconnectEachRemoteAbilityLocked(const std::string& localDeviceId,
        const std::string& remoteDeviceId, const sptr<IRemoteObject>& connect);
    sptr<IDistributedSched> GetRemoteDms(const std::string& remoteDeviceId);
    static bool GetLocalDeviceId(std::string& localDeviceId);
    bool CheckDeviceId(const std::string& localDeviceId, const std::string& remoteDeviceId);
    bool CheckDeviceIdFromRemote(const std::string& localDeviceId,
        const std::string& destinationDeviceId, const std::string& sourceDeviceId);
    void NotifyDeviceOfflineToAppLocked(const sptr<IRemoteObject>& connect, const ConnectAbilitySession& session);
    int32_t NotifyApp(const sptr<IRemoteObject>& connect, const AppExecFwk::ElementName& element, int32_t errCode);
    void NotifyProcessDiedAll(const std::list<ProcessDiedNotifyInfo>& notifyList);
    void NotifyProcessDied(const std::string& remoteDeviceId, const CallerInfo& callerInfo,
        TargetComponent targetComponent);
    int32_t CheckDistributedConnectLocked(const CallerInfo& callerInfo) const;
    void DecreaseConnectLocked(int32_t uid);
    static int32_t GetUidLocked(const std::list<ConnectAbilitySession>& sessionList);
    int32_t TryConnectRemoteAbility(const OHOS::AAFwk::Want& want,
        const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo);
    int32_t CleanMission(int32_t missionId);
    int32_t SetCallerInfo(int32_t callerUid, std::string localDeviceId, uint32_t accessToken, CallerInfo& callerInfo);
    int32_t SetWantForContinuation(AAFwk::Want& newWant, int32_t missionId);
    int32_t ContinueLocalMission(const std::string& dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams);
    int32_t ContinueRemoteMission(const std::string& srcDeviceId, const std::string& dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams);
    int32_t TryStartRemoteAbilityByCall(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo);
    int32_t StartLocalAbility(const FreeInstallInfo& info, int64_t taskId, int32_t resultCode);
    int32_t HandleRemoteNotify(const FreeInstallInfo& info, int64_t taskId, int32_t resultCode);
    bool HandleDistributedComponentChange(const std::string& componentInfo);
    void ReportDistributedComponentChange(const CallerInfo& callerInfo, int32_t changeType,
        int32_t componentType, int32_t deviceType);
    void ReportDistributedComponentChange(const ConnectInfo& connectInfo, int32_t changeType,
        int32_t componentType, int32_t deviceType);
    void HandleLocalCallerDied(const sptr<IRemoteObject>& connect);
    void SaveCallerComponent(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo);
    void RemoveCallerComponent(const sptr<IRemoteObject>& connect);
    void ProcessCalleeOffline(const std::string& deviceId);
    void GetConnectComponentList(std::vector<std::string>& distributedComponents);
    void GetCallComponentList(std::vector<std::string>& distributedComponents);
    void ProcessFreeInstallOffline(const std::string& deviceId);

    std::shared_ptr<DSchedContinuation> dschedContinuation_;
    std::map<sptr<IRemoteObject>, std::list<ConnectAbilitySession>> distributedConnectAbilityMap_;
    std::map<sptr<IRemoteObject>, ConnectInfo> connectAbilityMap_;
    std::unordered_map<int32_t, uint32_t> trackingUidMap_;
    std::mutex distributedLock_;
    std::mutex connectLock_;
    sptr<IRemoteObject::DeathRecipient> connectDeathRecipient_;
    std::mutex calleeLock_;
    std::map<sptr<IRemoteObject>, ConnectInfo> calleeMap_;
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient_;
    std::shared_ptr<DmsCallbackTask> dmsCallbackTask_;
    sptr<IRemoteObject> distributedComponentListener_;
    std::shared_ptr<AppExecFwk::EventHandler> componentChangeHandler_;
    std::mutex callerLock_;
    std::map<sptr<IRemoteObject>, std::list<ConnectAbilitySession>> callerMap_;
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipientForLocalDevice_;
};

class ConnectAbilitySession {
public:
    ConnectAbilitySession(const std::string& sourceDeviceId, const std::string& destinationDeviceId,
        const CallerInfo& callerInfo, TargetComponent targetComponent = TargetComponent::HARMONY_COMPONENT);
    ~ConnectAbilitySession() = default;

    const std::string& GetSourceDeviceId() const
    {
        return sourceDeviceId_;
    }

    const std::string& GetDestinationDeviceId() const
    {
        return destinationDeviceId_;
    }

    std::list<AppExecFwk::ElementName> GetElementsList() const
    {
        return elementsList_;
    }

    CallerInfo GetCallerInfo() const
    {
        return callerInfo_;
    }

    TargetComponent GetTargetComponent() const
    {
        return targetComponent_;
    }

    bool IsSameCaller(const CallerInfo& callerInfo);
    void AddElement(const AppExecFwk::ElementName& element);

private:
    std::string sourceDeviceId_;
    std::string destinationDeviceId_;
    std::list<AppExecFwk::ElementName> elementsList_;
    CallerInfo callerInfo_;
    TargetComponent targetComponent_;
};

class CallerDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    CallerDeathRecipient() = default;
    explicit CallerDeathRecipient(int32_t deviceType)
    {
        deviceType_ = deviceType;
    }
    ~CallerDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
private:
    int32_t deviceType_ = IDistributedSched::CALLEE;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_SERVICE_H
