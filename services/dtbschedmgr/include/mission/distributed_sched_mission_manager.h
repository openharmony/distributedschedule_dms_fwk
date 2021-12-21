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

#ifndef DISTRIBUTEDSCHED_MISSION_MANAGER_H
#define DISTRIBUTEDSCHED_MISSION_MANAGER_H

#include <set>
#include <string>
#include <vector>

#include "common_event_manager.h"
#include "distributed_sched_interface.h"
#include "event_handler.h"
#include "single_instance.h"
#include "mission/distributed_data_storage.h"
#include "mission/snapshot.h"

namespace OHOS {
namespace DistributedSchedule {
struct ListenerInfo {
    bool called = false;
    std::set<sptr<IRemoteObject>> listenerSet;

    bool Emplace(sptr<IRemoteObject> listener)
    {
        auto pairRet = listenerSet.emplace(listener);
        if (!pairRet.second) {
            return false;
        }
        return true;
    }

    bool Find(sptr<IRemoteObject> listener)
    {
        auto iter = listenerSet.find(listener);
        if (iter == listenerSet.end()) {
            return false;
        }
        return true;
    }

    void Erase(sptr<IRemoteObject> listener)
    {
        listenerSet.erase(listener);
    }

    int32_t Size() const
    {
        return listenerSet.size();
    }

    bool Empty() const
    {
        return listenerSet.empty();
    }
};
class DistributedSchedMissionManager {
    DECLARE_SINGLE_INSTANCE(DistributedSchedMissionManager);

public:
    void Init();
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMissions, std::vector<MissionInfo>& missionInfos);
    int32_t InitDataStorage();
    int32_t StopDataStorage();
    int32_t StoreSnapshotInfo(const std::string& deviceId,
                              int32_t missionId,
                              const uint8_t* byteStream,
                              size_t len);
    int32_t RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId);
    std::unique_ptr<Snapshot> GetRemoteSnapshotInfo(const std::string& deviceId, int32_t missionId);
    void DeviceOnlineNotify(const std::string& deviceId);
    void DeviceOfflineNotify(const std::string& deviceId);
    void DeleteDataStorage(const std::string& deviceId, bool isDelayed);
    int32_t RegisterRemoteMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj);
    int32_t UnRegisterRemoteMissionListener(const std::u16string& devId, const sptr<IRemoteObject>& obj);
    int32_t PrepareAndSyncMissionsFromRemote(const CallerInfo& callerInfo, std::vector<MissionInfo>& missionInfos);
    void UnRegisterMissionListenerFromRemote(const std::string& deviceId);
    bool needSyncDevice(const std::string& deviceId);

    void NotifySnapshotChanged(const std::string& devId, int32_t missionId);
    void OnRemoteDied(const wptr<IRemoteObject>& remote);

    void EnqueueCachedSnapshotInfo(const std::string& deviceId, int32_t missionId, std::unique_ptr<Snapshot> snapshot);
    std::unique_ptr<Snapshot> DequeueCachedSnapshotInfo(const std::string& deviceId, int32_t missionId);
    int32_t NotifyMissionsChangedToRemote(const std::vector<MissionInfo>& missionInfos);
    int32_t NotifyMissionsChangedFromRemote(const CallerInfo& callerInfo, const std::vector<MissionInfo>& missionInfos);
    int32_t CheckSupportOsd(const std::string& deviceId);
    int32_t CheckOsdSwitch(const std::string& deviceId);
    void GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values);
    int32_t GetOsdSwitchValueFromRemote();
    int32_t UpdateOsdSwitchValueFromRemote(int32_t switchVal, const std::string& deviceId);
    int32_t UpdateSwitchValueToRemote();
    void UpdateConnCapSupportOsd(const std::string& deviceId);
    void NotifyOsdSwitchChanged(bool needNotifyChanged);
    int32_t PrepareAndSyncMissions(const std::u16string& devId, bool fixConflict, int64_t tag);
    void OnRemoteDmsDied(const wptr<IRemoteObject>& remote);
    void NotifyDmsProxyProcessDied();
    void OnDnetDied();
private:
    std::map<std::string, std::shared_ptr<AppExecFwk::EventHandler>> deviceHandle_;
    mutable std::mutex remoteMissionInfosLock_;
    std::map<std::string, std::vector<MissionInfo>> deviceMissionInfos_;
    sptr<IDistributedSched> GetRemoteDms(const std::string& deviceId);
    bool IsDeviceIdValidated(const std::string& deviceId);
    std::shared_ptr<AppExecFwk::EventHandler> FetchDeviceHandler(const std::string& deviceId);
    bool GenerateCallerInfo(CallerInfo& callerInfo);
    void NotifyMissionsChangedToRemoteInner(const std::string& remoteUuid,
        const std::vector<MissionInfo>& missionInfos, const CallerInfo& callerInfo);
    std::string GenerateKeyInfo(const std::string& devId, int32_t missionId)
    {
        return devId + "_" + std::to_string(missionId);
    }
    bool AllowMissionUid(int32_t uid);
    int32_t PrepareAndSyncMissions(const std::string& dstDevId, const std::string& localDevId);
    int32_t PrepareAndSyncMissions(const std::string& dstDevId, const sptr<IDistributedSched>& remoteDms);
    int32_t UnRegisterRemoteMissionListenerInner(const std::string& dstDevId, bool offline, bool exit = false);
    void CleanMissionResources(const std::string& dstDevId);
    void RetryStartRemoteSyncMission(const std::string& dstDeviceId, const std::string& localDevId, int32_t retryTimes);
    bool HasSyncListener(const std::string& dstDeviceId);
    void DeleteCachedSnapshotInfo(const std::string& networkId);
    int32_t FetchCachedRemoteMissions(const std::string& srcId, int32_t numMissions,
        std::vector<MissionInfo>& missionInfos);
    void RebornMissionCache(const std::string& deviceId, const std::vector<MissionInfo>& missionInfos);
    void CleanMissionCache(const std::string& deviceId);
    void UpdateSwitchValueToRemoteInner(std::set<std::string>& remoteSyncDeviceSet,
        const std::string& localNetworkId);
    void TryUpdateSwitchValueToRemote(const std::string& localNetworkId,
        const std::string& destUuid, int32_t retryTime);
    bool IsValidOsdSwitchValue(int32_t osdSwitchVal);
    bool IsConnCapSupportOsd(const std::string& deviceId);
    bool GetConnCapSupportOsd(const std::string& deviceId);
    bool GetConnCapSupportOsdInnerLocked(const std::string& deviceId);
    bool PreCheckSupportOsd(const std::string& deviceId);
    void NotifyOsdSwitchChanged(bool needNotifyChanged, const std::string& deviceId, int32_t switchVal);
    void OnMissionListenerDied(const sptr<IRemoteObject>& remote);
    void OnRemoteDmsDied(const sptr<IRemoteObject>& remote);
    void RetryRegisterMissionChange(int32_t retryTimes);

    class ListenerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    };
    sptr<ListenerDeathRecipient> listenerDeath_;

    std::set<std::string> remoteSyncDeviceSet_;
    std::mutex remoteSyncDeviceLock_;

    std::map<std::string, std::unique_ptr<Snapshot>> cachedSnapshotInfos_;
    std::map<std::u16string, ListenerInfo> listenDeviceMap_;
    std::mutex listenDeviceLock_;
    std::shared_ptr<DistributedDataStorage> distributedDataStorage_;

    std::set<int32_t> allowMissionUids_;
    std::mutex allowMissionUidsLock_;
    bool isRegMissionChange_ = false;

    class RemoteDmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    };
    sptr<RemoteDmsDeathRecipient> remoteDmsRecipient_;
    std::map<std::string, sptr<IDistributedSched>> remoteDmsMap_;
    std::mutex remoteDmsLock_;
    std::shared_ptr<AppExecFwk::EventHandler> missionHandler_;
    std::shared_ptr<AppExecFwk::EventHandler> updateHandler_;
    std::mutex osdSwitchLock_;
    std::map<std::string, int32_t> osdSwitchValueMap_; // key is uuid
    std::map<std::string, bool> connCapSupportOsdMap_; // key is networkId
};
}
}
#endif // DISTRIBUTEDSCHED_MISSION_MANAGER_H
