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

#include "mission/distributed_sched_mission_manager.h"

#include <chrono>
#include <sys/time.h>
#include <unistd.h>

#include "datetime_ex.h"
#include "distributed_sched_adapter.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mission/mission_changed_notify.h"
#include "mission/mission_constant.h"
#include "mission/mission_info_converter.h"
#include "mission/snapshot_converter.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedSchedMissionManager";
constexpr size_t MAX_CACHED_ITEM = 10;
constexpr int32_t MAX_RETRY_TIMES = 15;
constexpr int32_t RETRY_DELAYED = 2000;
constexpr int32_t GET_FOREGROUND_SNAPSHOT_DELAY_TIME = 800; // ms
const std::string DELETE_DATA_STORAGE = "DeleteDataStorage";
constexpr int32_t DELETE_DATA_STORAGE_DELAYED = 60000; // ms
const std::string INVAILD_LOCAL_DEVICE_ID = "-1";
}
namespace Mission {
constexpr int32_t GET_MAX_MISSIONS = 20;
} // Mission
using namespace std::chrono;
using namespace Constants::Mission;
using namespace OHOS::DistributedKv;

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedMissionManager);

void DistributedSchedMissionManager::Init()
{
    listenerDeath_ = new ListenerDeathRecipient();
    remoteDmsRecipient_ = new RemoteDmsDeathRecipient();
    auto runner = AppExecFwk::EventRunner::Create("MissionManagerHandler");
    missionHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto updateRunner = AppExecFwk::EventRunner::Create("UpdateHandler");
    updateHandler_ = std::make_shared<AppExecFwk::EventHandler>(updateRunner);
    missonChangeListener_ = new DistributedMissionChangeListener();
    auto missionChangeRunner = AppExecFwk::EventRunner::Create("DistributedMissionChange");
    missionChangeHandler_ = std::make_shared<AppExecFwk::EventHandler>(missionChangeRunner);
}

int32_t DistributedSchedMissionManager::GetMissionInfos(const std::string& deviceId,
    int32_t numMissions, std::vector<AAFwk::MissionInfo>& missionInfos)
{
    HILOGI("start!");
    if (!IsDeviceIdValidated(deviceId)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (numMissions <= 0) {
        HILOGE("numMissions is illegal! numMissions:%{public}d", numMissions);
        return INVALID_PARAMETERS_ERR;
    }
    std::vector<DstbMissionInfo> dstbMissionInfos;
    int32_t ret = FetchCachedRemoteMissions(deviceId, numMissions, dstbMissionInfos);
    if (ret != ERR_OK) {
        HILOGE("FetchCachedRemoteMissions failed, ret = %{public}d", ret);
        return ret;
    }
    return MissionInfoConverter::ConvertToMissionInfos(dstbMissionInfos, missionInfos);
}

sptr<IDistributedSched> DistributedSchedMissionManager::GetRemoteDms(const std::string& deviceId)
{
    int64_t begin = GetTickCount();
    if (deviceId.empty()) {
        HILOGE("GetRemoteDms remoteDeviceId is empty");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(remoteDmsLock_);
        auto iter = remoteDmsMap_.find(deviceId);
        if (iter != remoteDmsMap_.end()) {
            auto object = iter->second;
            if (object != nullptr) {
                HILOGI("[PerformanceTest] GetRemoteDms from cache spend %{public}" PRId64 " ms",
                    GetTickCount() - begin);
                return object;
            }
        }
    }
    HILOGD("GetRemoteDms connect deviceid is %s", deviceId.c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("GetRemoteDms failed to connect to systemAbilityMgr!");
        return nullptr;
    }
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID, deviceId);
    if (object == nullptr) {
        HILOGE("GetRemoteDms failed to get dms for remote device:%{public}s!",
            DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
        return nullptr;
    }
    auto ret = object->AddDeathRecipient(remoteDmsRecipient_);
    HILOGD("GetRemoteDms AddDeathRecipient ret : %{public}d", ret);
    sptr<IDistributedSched> remoteDmsObj = iface_cast<IDistributedSched>(object);
    {
        std::lock_guard<std::mutex> autoLock(remoteDmsLock_);
        auto iter = remoteDmsMap_.find(deviceId);
        if (iter != remoteDmsMap_.end()) {
            iter->second->AsObject()->RemoveDeathRecipient(remoteDmsRecipient_);
        }
        remoteDmsMap_[deviceId] = remoteDmsObj;
    }
    HILOGI("[PerformanceTest] GetRemoteDms spend %{public}" PRId64 " ms", GetTickCount() - begin);
    return remoteDmsObj;
}

bool DistributedSchedMissionManager::IsDeviceIdValidated(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGE("IsDeviceIdValidated deviceId is empty!");
        return false;
    }
    if (DtbschedmgrDeviceInfoStorage::GetInstance().GetDeviceInfoById(deviceId) == nullptr) {
        HILOGW("IsDeviceIdValidated device offline.");
        return false;
    }

    return true;
}

void DistributedSchedMissionManager::NotifyRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (distributedDataStorage_ == nullptr) {
        HILOGE("DistributedDataStorage null!");
        return;
    }
    distributedDataStorage_->NotifyRemoteDied(remote);
}

int32_t DistributedSchedMissionManager::InitDataStorage()
{
    if (distributedDataStorage_ == nullptr) {
        distributedDataStorage_ = std::make_shared<DistributedDataStorage>();
    }
    if (!distributedDataStorage_->Init()) {
        HILOGE("InitDataStorage DistributedDataStorage init failed!");
        return ERR_NULL_OBJECT;
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::StopDataStorage()
{
    if (distributedDataStorage_ == nullptr) {
        HILOGE("StopDataStorage DistributedDataStorage null!");
        return ERR_NULL_OBJECT;
    }
    if (!distributedDataStorage_->Stop()) {
        HILOGE("StopDataStorage DistributedDataStorage stop failed!");
        return ERR_NULL_OBJECT;
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    if (distributedDataStorage_ == nullptr) {
        HILOGE("StoreSnapshotInfo DistributedDataStorage null!");
        return ERR_NULL_OBJECT;
    }
    if (!distributedDataStorage_->Insert(deviceId, missionId, byteStream, len)) {
        HILOGE("StoreSnapshotInfo DistributedDataStorage insert failed!");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId)
{
    if (distributedDataStorage_ == nullptr) {
        HILOGE("RemoveSnapshotInfo DistributedDataStorage null!");
        return ERR_NULL_OBJECT;
    }
    if (!distributedDataStorage_->Delete(deviceId, missionId)) {
        HILOGE("RemoveSnapshotInfo DistributedDataStorage delete failed!");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::GetRemoteMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot)
{
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        HILOGE("uuid is empty!");
        return INVALID_PARAMETERS_ERR;
    }
    std::unique_ptr<Snapshot> snapshotPtr = DequeueCachedSnapshotInfo(uuid, missionId);
    if (snapshotPtr != nullptr) {
        HILOGI("get uuid = %{public}s + missionId = %{public}d snapshot from cache successful!",
            DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
        SnapshotConverter::ConvertToMissionSnapshot(*snapshotPtr, missionSnapshot);
        return ERR_NONE;
    }
    if (distributedDataStorage_ == nullptr) {
        HILOGE("DistributedDataStorage null!");
        return ERR_NULL_OBJECT;
    }
    DistributedKv::Value value;
    bool ret = distributedDataStorage_->Query(networkId, missionId, value);
    if (!ret) {
        HILOGE("DistributedDataStorage query failed!");
        return INVALID_PARAMETERS_ERR;
    }
    snapshotPtr = Snapshot::Create(value.Data());
    if (snapshotPtr == nullptr) {
        HILOGE("snapshot create failed!");
        return ERR_NULL_OBJECT;
    }
    HILOGI("get uuid = %{public}s + missionId = %{public}d snapshot from DistributedDB successful!",
        DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
    SnapshotConverter::ConvertToMissionSnapshot(*snapshotPtr, missionSnapshot);
    return ERR_NONE;
}

void DistributedSchedMissionManager::DeviceOnlineNotify(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGW("DeviceOnlineNotify deviceId empty!");
        return;
    }

    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(deviceId);
    if (missionHandler_ != nullptr) {
        HILOGI("DeviceOnlineNotify RemoveTask");
        missionHandler_->RemoveTask(DELETE_DATA_STORAGE + uuid);
    }
}

void DistributedSchedMissionManager::DeviceOfflineNotify(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGW("DeviceOfflineNotify deviceId empty!");
        return;
    }
    StopSyncMissionsFromRemote(deviceId);
    CleanMissionResources(deviceId);
    {
        std::lock_guard<std::mutex> autoLock(remoteDmsLock_);
        auto iter = remoteDmsMap_.find(deviceId);
        if (iter != remoteDmsMap_.end()) {
            iter->second->AsObject()->RemoveDeathRecipient(remoteDmsRecipient_);
            remoteDmsMap_.erase(iter);
        }
    }
    HILOGI("DeviceOfflineNotify erase value for deviceId: %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
}

void DistributedSchedMissionManager::DeleteDataStorage(const std::string& deviceId, bool isDelayed)
{
    if (distributedDataStorage_ == nullptr) {
        HILOGE("DeleteDataStorage DistributedDataStorage null!");
        return;
    }
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(deviceId);
    auto callback = [this, uuid, deviceId]() {
        if (!distributedDataStorage_->FuzzyDelete(deviceId)) {
            HILOGE("DeleteDataStorage storage delete failed!");
        } else {
            HILOGI("DeleteDataStorage storage delete successfully!");
        }
    };
    if (isDelayed) {
        if (missionHandler_ != nullptr) {
            HILOGI("DeleteDataStorage PostTask");
            missionHandler_->PostTask(callback, DELETE_DATA_STORAGE + uuid, DELETE_DATA_STORAGE_DELAYED);
        }
    } else {
        if (missionHandler_ != nullptr) {
            HILOGI("DeleteDataStorage RemoveTask");
            missionHandler_->RemoveTask(DELETE_DATA_STORAGE + uuid);
        }
        callback();
    }
}

int32_t DistributedSchedMissionManager::RegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& listener)
{
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(Str16ToStr8(devId));
    if (missionHandler_ != nullptr) {
        HILOGI("RemoveTask");
        missionHandler_->RemoveTask(DELETE_DATA_STORAGE + uuid);
    }
    if (listener == nullptr) {
        return INVALID_PARAMETERS_ERR;
    }
    std::string localDeviceId;
    std::string remoteDeviceId = Str16ToStr8(devId);
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)
        || localDeviceId == remoteDeviceId) {
        HILOGE("check deviceId failed!");
        return INVALID_PARAMETERS_ERR;
    }
    {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto& listenerInfo = listenDeviceMap_[devId];
        if (!listenerInfo.Emplace(listener)) {
            HILOGW("RegisterSyncListener listener has already inserted!");
            return ERR_NONE;
        }
        bool ret = listener->AddDeathRecipient(listenerDeath_);
        if (!ret) {
            HILOGW("RegisterSyncListener AddDeathRecipient failed!");
        }
        if (listenerInfo.Size() > 1) {
            HILOGI("RegisterMissionListener not notify remote DMS!");
            return ERR_NONE;
        }
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::StartSyncRemoteMissions(const std::string& dstDevId,
    const std::string& localDevId)
{
    std::u16string devId = Str8ToStr16(dstDevId);
    {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iterItem = listenDeviceMap_.find(devId);
        if (iterItem == listenDeviceMap_.end()) {
            return ERR_NONE;
        }
        bool callFlag = iterItem->second.called;
        if (callFlag) {
            HILOGI("StartSyncRemoteMissions already called!");
            return ERR_NONE;
        }
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(dstDevId);
    if (remoteDms == nullptr) {
        HILOGE("get remoteDms failed!");
        RetryStartSyncRemoteMissions(dstDevId, localDevId, 0);
        return GET_REMOTE_DMS_FAIL;
    }
    int32_t ret = StartSyncRemoteMissions(dstDevId, remoteDms);
    if (ret == ERR_NONE) {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iterItem = listenDeviceMap_.find(devId);
        if (iterItem != listenDeviceMap_.end()) {
            iterItem->second.called = true;
        }
    }
    return ret;
}

int32_t DistributedSchedMissionManager::StartSyncRemoteMissions(const std::string& dstDevId,
    const sptr<IDistributedSched>& remoteDms)
{
    std::vector<DstbMissionInfo> missionInfos;
    CallerInfo callerInfo;
    if (!GenerateCallerInfo(callerInfo)) {
        return GET_LOCAL_DEVICE_ERR;
    }
    int64_t begin = GetTickCount();
    int32_t ret = remoteDms->StartSyncMissionsFromRemote(callerInfo, missionInfos);
    HILOGI("[PerformanceTest] StartSyncMissionsFromRemote ret:%{public}d, spend %{public}" PRId64 " ms",
        ret, GetTickCount() - begin);
    if (ret == ERR_NONE) {
        RebornMissionCache(dstDevId, missionInfos);
    }
    return ret;
}

int32_t DistributedSchedMissionManager::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& listener)
{
    if (listener == nullptr) {
        return INVALID_PARAMETERS_ERR;
    }
    std::string localDeviceId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)
        || localDeviceId == Str16ToStr8(devId)) {
        HILOGE("check deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }
    {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iterItem = listenDeviceMap_.find(devId);
        if (iterItem == listenDeviceMap_.end()) {
            return ERR_NONE;
        }
        auto& listenerInfo = iterItem->second;
        auto ret = listenerInfo.Find(listener);
        if (!ret) {
            HILOGI("listener not registered!");
            return ERR_NONE;
        }
        listener->RemoveDeathRecipient(listenerDeath_);
        listenerInfo.Erase(listener);
        if (!listenerInfo.Empty()) {
            return ERR_NONE;
        }
        listenDeviceMap_.erase(iterItem);
    }
    return ERR_NONE;
}

void DistributedSchedMissionManager::CleanMissionResources(const std::string& dstDevId)
{
    {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iterDevice = listenDeviceMap_.find(Str8ToStr16(dstDevId));
        if (iterDevice == listenDeviceMap_.end()) {
            return;
        }
        auto& listenerInfo = iterDevice->second;
        for (sptr<IRemoteObject> listener : listenerInfo.listenerSet) {
            if (listener != nullptr) {
                listener->RemoveDeathRecipient(listenerDeath_);
            }
        }
        listenDeviceMap_.erase(iterDevice);
    }
    StopSyncRemoteMissions(dstDevId, true);
}

int32_t DistributedSchedMissionManager::StopSyncRemoteMissions(const std::string& dstDevId,
    bool offline, bool exit)
{
    CleanMissionCache(dstDevId);
    DeleteCachedSnapshotInfo(dstDevId);
    DeleteDataStorage(dstDevId, true);

    if (offline) {
        return ERR_NONE;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(dstDevId);
    if (remoteDms == nullptr) {
        HILOGE("DMS get remoteDms failed");
        return GET_REMOTE_DMS_FAIL;
    }

    CallerInfo callerInfo;
    if (!GenerateCallerInfo(callerInfo)) {
        return GET_LOCAL_DEVICE_ERR;
    }
    int64_t begin = GetTickCount();
    int32_t ret = remoteDms->StopSyncMissionsFromRemote(callerInfo);
    HILOGI("[PerformanceTest] ret:%d, spend %{public}" PRId64 " ms", ret, GetTickCount() - begin);
    return ret;
}

int32_t DistributedSchedMissionManager::StartSyncRemoteMissions(const std::string& dstDevId, bool fixConflict,
    int64_t tag)
{
    std::string localDeviceId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)
        || (dstDevId == localDeviceId)) {
        HILOGE("check deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }
    HILOGI("begin, dstDevId is %{public}s, local deviceId is %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(dstDevId).c_str(),
        DnetworkAdapter::AnonymizeDeviceId(localDeviceId).c_str());
    auto ret = StartSyncRemoteMissions(dstDevId, localDeviceId);
    if (ret != ERR_NONE) {
        HILOGE("StartSyncRemoteMissions failed, %{public}d", ret);
        return ret;
    }
    return ERR_NONE;
}

int32_t DistributedSchedMissionManager::StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
    std::vector<DstbMissionInfo>& missionInfos)
{
    auto deviceId = callerInfo.sourceDeviceId;
    HILOGD("remote version is %{public}d!", callerInfo.dmsVersion);
    {
        std::lock_guard<std::mutex> autoLock(remoteSyncDeviceLock_);
        remoteSyncDeviceSet_.emplace(deviceId);
    }
    int32_t result = DistributedSchedAdapter::GetInstance().GetLocalMissionInfos(Mission::GET_MAX_MISSIONS,
        missionInfos);
    auto func = [this, missionInfos]() {
        HILOGD("RegisterMissionListener called.");
        if (!isRegMissionChange_) {
            int32_t ret = DistributedSchedAdapter::GetInstance().RegisterMissionListener(missonChangeListener_);
            if (ret == ERR_OK) {
                isRegMissionChange_ = true;
            }
            InitAllSnapshots(missionInfos);
        }
    };
    if (!missionHandler_->PostTask(func)) {
        HILOGE("post RegisterMissionListener and InitAllSnapshots Task failed");
    }
    return result;
}

void DistributedSchedMissionManager::StopSyncMissionsFromRemote(const std::string& deviceId)
{
    HILOGD(" %{private}s!", deviceId.c_str());
    {
        std::lock_guard<std::mutex> autoLock(remoteSyncDeviceLock_);
        remoteSyncDeviceSet_.erase(deviceId);
        if (remoteSyncDeviceSet_.empty()) {
            auto func = [this]() {
                int32_t ret = DistributedSchedAdapter::GetInstance().UnRegisterMissionListener(missonChangeListener_);
                if (ret == ERR_OK) {
                    isRegMissionChange_ = false;
                }
            };
            if (!missionHandler_->PostTask(func)) {
                HILOGE("post UnRegisterMissionListener Task failed");
            }
        }
    }
}

bool DistributedSchedMissionManager::needSyncDevice(const std::string& deviceId)
{
    std::lock_guard<std::mutex> autoLock(remoteSyncDeviceLock_);
    if (remoteSyncDeviceSet_.count(deviceId) == 0) {
        return false;
    }
    return true;
}

bool DistributedSchedMissionManager::HasSyncListener(const std::string& networkId)
{
    std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
    auto iter = listenDeviceMap_.find(Str8ToStr16(networkId));
    if (iter != listenDeviceMap_.end()) {
        return iter->second.called;
    }
    return false;
}

void DistributedSchedMissionManager::NotifySnapshotChanged(const std::string& networkId, int32_t missionId)
{
    std::u16string u16DevId = Str8ToStr16(networkId);
    std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
    auto iter = listenDeviceMap_.find(u16DevId);
    if (iter == listenDeviceMap_.end()) {
        return;
    }
    auto& listenerInfo = iter->second;
    for (auto& listener : listenerInfo.listenerSet) {
        MissionChangedNotify::NotifySnapshot(listener, u16DevId, missionId);
    }
}

void DistributedSchedMissionManager::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("OnRemoteDied!");
    sptr<IRemoteObject> listener = remote.promote();
    if (listener == nullptr) {
        return;
    }
    auto remoteDiedFunc = [this, listener]() {
        OnMissionListenerDied(listener);
    };
    if (missionHandler_ != nullptr) {
        missionHandler_->PostTask(remoteDiedFunc);
    }
}

void DistributedSchedMissionManager::ListenerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DistributedSchedMissionManager::GetInstance().OnRemoteDied(remote);
}

void DistributedSchedMissionManager::EnqueueCachedSnapshotInfo(const std::string& deviceId, int32_t missionId,
    std::unique_ptr<Snapshot> snapshot)
{
    if (deviceId.empty() || snapshot == nullptr) {
        HILOGW("EnqueueCachedSnapshotInfo invalid input param!");
        return;
    }
    std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
    std::string keyInfo = GenerateKeyInfo(deviceId, missionId);
    auto iter = cachedSnapshotInfos_.find(keyInfo);
    if (iter != cachedSnapshotInfos_.end()) {
        if (snapshot->GetCreatedTime() < iter->second->GetCreatedTime()) {
            return;
        }
    }

    if (cachedSnapshotInfos_.size() == MAX_CACHED_ITEM) {
        int64_t oldest = -1;
        auto iterOldest = cachedSnapshotInfos_.end();
        for (auto iterItem = cachedSnapshotInfos_.begin(); iterItem != cachedSnapshotInfos_.end(); ++iterItem) {
            if (oldest == -1 || iterItem->second->GetLastAccessTime() < oldest) {
                oldest = iterItem->second->GetLastAccessTime();
                iterOldest = iterItem;
            }
        }
        if (iterOldest != cachedSnapshotInfos_.end()) {
            cachedSnapshotInfos_.erase(iterOldest);
        }
    }
    cachedSnapshotInfos_[keyInfo] = std::move(snapshot);
}

std::unique_ptr<Snapshot> DistributedSchedMissionManager::DequeueCachedSnapshotInfo(const std::string& deviceId,
    int32_t missionId)
{
    if (deviceId.empty()) {
        HILOGW("DequeueCachedSnapshotInfo invalid input param!");
        return nullptr;
    }
    std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
    auto iter = cachedSnapshotInfos_.find(GenerateKeyInfo(deviceId, missionId));
    if (iter != cachedSnapshotInfos_.end()) {
        std::unique_ptr<Snapshot> snapshot = std::move(iter->second);
        snapshot->UpdateLastAccessTime(GetTickCount());
        iter->second = nullptr;
        cachedSnapshotInfos_.erase(iter);
        return snapshot;
    }
    return nullptr;
}

void DistributedSchedMissionManager::DeleteCachedSnapshotInfo(const std::string& networkId)
{
    if (networkId.empty()) {
        return;
    }
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        return;
    }
    std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
    auto iter = cachedSnapshotInfos_.begin();
    while (iter != cachedSnapshotInfos_.end()) {
        if (iter->first.find(uuid) != std::string::npos) {
            iter = cachedSnapshotInfos_.erase(iter);
        } else {
            ++iter;
        }
    }
}

int32_t DistributedSchedMissionManager::FetchCachedRemoteMissions(const std::string& srcId, int32_t numMissions,
    std::vector<DstbMissionInfo>& missionInfos)
{
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(srcId);
    if (uuid.empty()) {
        HILOGE("uuid empty!");
        return INVALID_PARAMETERS_ERR;
    }
    std::lock_guard<std::mutex> autoLock(remoteMissionInfosLock_);
    auto iter = deviceMissionInfos_.find(uuid);
    if (iter == deviceMissionInfos_.end()) {
        HILOGE("can not find uuid, deviceId: %{public}s!",
            DnetworkAdapter::AnonymizeDeviceId(srcId).c_str());
        return ERR_NULL_OBJECT;
    }

    // get at most numMissions missions
    int32_t actualNums = static_cast<int32_t>((iter->second).size());
    if (actualNums < 0) {
        HILOGE("invalid size!");
        return ERR_NULL_OBJECT;
    }
    missionInfos.assign((iter->second).begin(),
        (actualNums > numMissions) ? (iter->second).begin() + numMissions : (iter->second).end());
    return ERR_NONE;
}

void DistributedSchedMissionManager::RebornMissionCache(const std::string& deviceId,
    const std::vector<DstbMissionInfo>& missionInfos)
{
    HILOGI("start! deviceId is %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(deviceId);
    if (uuid.empty()) {
        HILOGE("uuid empty!");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(remoteMissionInfosLock_);
        deviceMissionInfos_[uuid] = missionInfos;
    }
    HILOGI("RebornMissionCache end!");
}

void DistributedSchedMissionManager::CleanMissionCache(const std::string& deviceId)
{
    HILOGI("CleanMissionCache start! deviceId is %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(deviceId);
    if (uuid.empty()) {
        HILOGE("CleanMissionCache uuid empty!");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(remoteMissionInfosLock_);
        deviceMissionInfos_.erase(uuid);
    }
    HILOGI("CleanMissionCache end!");
}

int32_t DistributedSchedMissionManager::NotifyMissionsChangedFromRemote(const CallerInfo& callerInfo,
    const std::vector<DstbMissionInfo>& missionInfos)
{
    HILOGI("NotifyMissionsChangedFromRemote version is %{public}d!", callerInfo.dmsVersion);
    std::u16string u16DevId = Str8ToStr16(callerInfo.sourceDeviceId);
    RebornMissionCache(callerInfo.sourceDeviceId, missionInfos);
    {
        HILOGI("NotifyMissionsChangedFromRemote notify mission start!");
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iter = listenDeviceMap_.find(u16DevId);
        if (iter == listenDeviceMap_.end()) {
            HILOGE("NotifyMissionsChangedFromRemote notify mission no listener!");
            return INVALID_PARAMETERS_ERR;
        }
        auto& listenerSet = iter->second.listenerSet;
        auto notifyChanged = [listenerSet, u16DevId] () {
            for (const auto& listener : listenerSet) {
                MissionChangedNotify::NotifyMissionsChanged(listener, u16DevId);
            }
        };
        if (missionHandler_ != nullptr) {
            missionHandler_->PostTask(notifyChanged);
            HILOGI("NotifyMissionsChangedFromRemote end!");
            return ERR_NONE;
        }
    }
    return INVALID_PARAMETERS_ERR;
}

void DistributedSchedMissionManager::NotifyLocalMissionsChanged()
{
    auto func = [this]() {
        HILOGI("NotifyLocalMissionsChanged");
        std::vector<DstbMissionInfo> missionInfos;
        int32_t ret = DistributedSchedAdapter::GetInstance().GetLocalMissionInfos(Mission::GET_MAX_MISSIONS,
            missionInfos);
        if (ret == ERR_OK) {
            int32_t result = NotifyMissionsChangedToRemote(missionInfos);
            HILOGI("NotifyMissionsChangedToRemote result = %{public}d", result);
        }
    };
    if (!missionChangeHandler_->PostTask(func)) {
        HILOGE("postTask failed");
    }
}

void DistributedSchedMissionManager::NotifyMissionSnapshotCreated(int32_t missionId)
{
    auto func = [this, missionId]() {
        HILOGD("called.");
        ErrCode errCode = MissionSnapshotChanged(missionId);
        if (errCode != ERR_OK) {
            HILOGE("mission snapshot changed failed, missionId=%{public}d, errCode=%{public}d", missionId, errCode);
        }
    };
    if (!missionChangeHandler_->PostTask(func, GET_FOREGROUND_SNAPSHOT_DELAY_TIME)) {
        HILOGE("post MissionSnapshotChanged delay Task failed");
    }
}

void DistributedSchedMissionManager::NotifyMissionSnapshotChanged(int32_t missionId)
{
    auto func = [this, missionId]() {
        HILOGD("called.");
        ErrCode errCode = MissionSnapshotChanged(missionId);
        if (errCode != ERR_OK) {
            HILOGE("mission snapshot changed failed, missionId=%{public}d, errCode=%{public}d", missionId, errCode);
        }
    };
    if (!missionChangeHandler_->PostTask(func)) {
        HILOGE("post MissionSnapshotChanged Task failed");
    }
}

void DistributedSchedMissionManager::NotifyMissionSnapshotDestroyed(int32_t missionId)
{
    auto func = [this, missionId]() {
        HILOGD("called.");
        ErrCode errCode = MissionSnapshotDestroyed(missionId);
        if (errCode != ERR_OK) {
            HILOGE("mission snapshot removed failed, missionId=%{public}d, errCode=%{public}d", missionId, errCode);
        }
    };
    if (!missionChangeHandler_->PostTask(func)) {
        HILOGE("post MissionSnapshotDestroyed Task failed");
    }
}

int32_t DistributedSchedMissionManager::NotifyMissionsChangedToRemote(const std::vector<DstbMissionInfo>& missionInfos)
{
    CallerInfo callerInfo;
    if (!GenerateCallerInfo(callerInfo)) {
        return GET_LOCAL_DEVICE_ERR;
    }
    std::lock_guard<std::mutex> autoLock(remoteSyncDeviceLock_);
    for (const auto& destDeviceId : remoteSyncDeviceSet_) {
        auto handler = FetchDeviceHandler(destDeviceId);
        if (handler == nullptr) {
            HILOGE("NotifyMissionsChangedToRemote fetch handler failed!");
            continue;
        }
        auto callback = [destDeviceId, missionInfos, callerInfo, this] () {
            NotifyMissionsChangedToRemoteInner(destDeviceId, missionInfos, callerInfo);
        };
        if (!handler->PostTask(callback)) {
            HILOGE("NotifyMissionsChangedToRemote PostTask failed!");
            return ERR_NULL_OBJECT;
        }
    }

    return ERR_NONE;
}

void DistributedSchedMissionManager::NotifyMissionsChangedToRemoteInner(const std::string& deviceId,
    const std::vector<DstbMissionInfo>& missionInfos, const CallerInfo& callerInfo)
{
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("NotifyMissionsChangedToRemote DMS get remoteDms failed");
        return;
    }
    int64_t begin = GetTickCount();
    int32_t result = remoteDms->NotifyMissionsChangedFromRemote(missionInfos, callerInfo);
    HILOGI("[PerformanceTest] NotifyMissionsChangedFromRemote ret:%{public}d, spend %{public}" PRId64 " ms",
        result, GetTickCount() - begin);
}

bool DistributedSchedMissionManager::GenerateCallerInfo(CallerInfo& callerInfo)
{
    std::string localUuid;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localUuid)) {
        HILOGE("get local uuid failed!");
        return false;
    }
    callerInfo.uid = IPCSkeleton::GetCallingUid();
    callerInfo.pid = IPCSkeleton::GetCallingPid();
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    callerInfo.sourceDeviceId = localUuid;
    callerInfo.dmsVersion = VERSION;
    return true;
}

std::shared_ptr<AppExecFwk::EventHandler> DistributedSchedMissionManager::FetchDeviceHandler(
    const std::string& deviceId)
{
    if (!IsDeviceIdValidated(deviceId)) {
        HILOGW("FetchDeviceHandler device:%{public}s offline.",
            DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
        return nullptr;
    }

    std::string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(deviceId);
    if (uuid.empty()) {
        HILOGE("FetchDeviceHandler uuid empty!");
        return nullptr;
    }

    auto iter = deviceHandle_.find(uuid);
    if (iter != deviceHandle_.end()) {
        return iter->second;
    }

    auto anonyUuid = DnetworkAdapter::AnonymizeDeviceId(uuid);
    auto runner = AppExecFwk::EventRunner::Create(anonyUuid + "_MissionN");
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (handler != nullptr) {
        deviceHandle_.emplace(uuid, handler);
    }
    return handler;
}

void DistributedSchedMissionManager::RemoteDmsDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("OnRemoteDied received died notify!");
    DistributedSchedMissionManager::GetInstance().OnRemoteDmsDied(remote);
}

void DistributedSchedMissionManager::OnRemoteDmsDied(const wptr<IRemoteObject>& remote)
{
    sptr<IRemoteObject> diedRemoted = remote.promote();
    if (diedRemoted == nullptr) {
        HILOGW("OnRemoteDmsDied promote failed!");
        return;
    }
    HILOGD("delete diedRemoted");
    auto remoteDmsDiedFunc = [this, diedRemoted]() {
        OnRemoteDmsDied(diedRemoted);
    };
    if (missionHandler_ != nullptr) {
        missionHandler_->PostTask(remoteDmsDiedFunc);
    }
}

void DistributedSchedMissionManager::RetryStartSyncRemoteMissions(const std::string& dstDeviceId,
    const std::string& localDevId, int32_t retryTimes)
{
    auto retryFunc = [this, dstDeviceId, localDevId, retryTimes]() {
        bool ret = HasSyncListener(dstDeviceId);
        if (!ret) {
            return;
        }
        sptr<IDistributedSched> remoteDms = GetRemoteDms(dstDeviceId);
        if (remoteDms == nullptr) {
            HILOGI("RetryStartSyncRemoteMissions DMS get remoteDms failed");
            RetryStartSyncRemoteMissions(dstDeviceId, localDevId, retryTimes + 1);
            return;
        }
        int32_t errNo = StartSyncRemoteMissions(dstDeviceId, remoteDms);
        HILOGI("RetryStartSyncRemoteMissions result:%{public}d", errNo);
    };
    if (missionHandler_ != nullptr && retryTimes < MAX_RETRY_TIMES) {
        missionHandler_->PostTask(retryFunc, RETRY_DELAYED);
    }
}

void DistributedSchedMissionManager::OnMissionListenerDied(const sptr<IRemoteObject>& remote)
{
    HILOGI("OnMissionListenerDied");
    std::set<std::string> deviceSet;
    {
        std::lock_guard<std::mutex> autoLock(listenDeviceLock_);
        auto iterItem = listenDeviceMap_.begin();
        while (iterItem != listenDeviceMap_.end()) {
            auto& listenerInfo = iterItem->second;
            auto ret = listenerInfo.Find(remote);
            if (!ret) {
                ++iterItem;
                continue;
            }
            remote->RemoveDeathRecipient(listenerDeath_);
            listenerInfo.Erase(remote);
            if (listenerInfo.Empty()) {
                if (listenerInfo.called) {
                    deviceSet.emplace(Str16ToStr8(iterItem->first));
                }
                iterItem = listenDeviceMap_.erase(iterItem);
            } else {
                ++iterItem;
            }
        }
    }
    if (deviceSet.empty()) {
        return;
    }
    for (auto& devId : deviceSet) {
        StopSyncRemoteMissions(devId, false);
    }
}

void DistributedSchedMissionManager::OnRemoteDmsDied(const sptr<IRemoteObject>& remote)
{
    HILOGI("OnRemoteDmsDied");
    std::string devId;
    {
        std::lock_guard<std::mutex> autoLock(remoteDmsLock_);
        for (auto iter = remoteDmsMap_.begin(); iter != remoteDmsMap_.end(); ++iter) {
            if (iter->second->AsObject() == remote) {
                iter->second->AsObject()->RemoveDeathRecipient(remoteDmsRecipient_);
                devId = iter->first;
                remoteDmsMap_.erase(iter);
                break;
            }
        }
    }
    if (devId.empty()) {
        return;
    }
    bool ret = HasSyncListener(devId);
    if (ret) {
        std::string localDeviceId;
        if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
            return;
        }
        RetryStartSyncRemoteMissions(devId, localDeviceId, 0);
    }
}

void DistributedSchedMissionManager::NotifyDmsProxyProcessDied()
{
    HILOGI("NotifyDmsProxyProcessDied!");
    if (!isRegMissionChange_) {
        return;
    }
    RetryRegisterMissionChange(0);
}

void DistributedSchedMissionManager::RetryRegisterMissionChange(int32_t retryTimes)
{
    auto remoteDiedFunc = [this, retryTimes]() {
        HILOGI("RetryRegisterMissionChange retryTimes:%{public}d begin", retryTimes);
        if (!isRegMissionChange_) {
            return;
        }
        int32_t ret = DistributedSchedAdapter::GetInstance().RegisterMissionListener(missonChangeListener_);
        if (ret == ERR_NULL_OBJECT) {
            RetryRegisterMissionChange(retryTimes + 1);
            HILOGI("RetryRegisterMissionChange dmsproxy null, retry!");
            return;
        }
        HILOGI("RetryRegisterMissionChange result:%{public}d", ret);
    };
    if (missionHandler_ != nullptr && retryTimes < MAX_RETRY_TIMES) {
        missionHandler_->PostTask(remoteDiedFunc, RETRY_DELAYED);
    }
}

void DistributedSchedMissionManager::InitAllSnapshots(const std::vector<DstbMissionInfo>& missionInfos)
{
    for (auto iter = missionInfos.begin(); iter != missionInfos.end(); iter++) {
        ErrCode errCode = MissionSnapshotChanged(iter->id);
        if (errCode != ERR_OK) {
            HILOGE("mission snapshot changed failed, missionId=%{public}d, errCode=%{public}d", iter->id, errCode);
        }
    }
}

int32_t DistributedSchedMissionManager::MissionSnapshotChanged(int32_t missionId)
{
    std::string networkId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(networkId)) {
        HILOGE("get local networkId failed!");
        return INVALID_PARAMETERS_ERR;
    }
    AAFwk::MissionSnapshot missionSnapshot;
    ErrCode errCode = DistributedSchedAdapter::GetInstance()
        .GetLocalMissionSnapshotInfo(networkId, missionId, missionSnapshot);
    if (errCode != ERR_OK) {
        HILOGE("get local mission snapshot failed, missionId=%{public}d, errCode=%{public}d", missionId, errCode);
        return errCode;
    }
    Snapshot snapshot;
    SnapshotConverter::ConvertToSnapshot(missionSnapshot, snapshot);
    MessageParcel data;
    errCode = MissionSnapshotSequence(snapshot, data);
    if (errCode != ERR_OK) {
        HILOGE("mission snapshot sequence failed, errCode=%{public}d", errCode);
        return errCode;
    }
    size_t len = data.GetReadableBytes();
    const uint8_t* byteStream = data.ReadBuffer(len);
    errCode = StoreSnapshotInfo(networkId, missionId, byteStream, len);
    return errCode;
}

int32_t DistributedSchedMissionManager::MissionSnapshotDestroyed(int32_t missionId)
{
    std::string networkId;
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(networkId)) {
        HILOGE("get local networkId failed!");
        return INVALID_PARAMETERS_ERR;
    }
    ErrCode errCode = RemoveSnapshotInfo(networkId, missionId);
    return errCode;
}

int32_t DistributedSchedMissionManager::MissionSnapshotSequence(const Snapshot& snapshot, MessageParcel& data)
{
    bool ret = snapshot.WriteSnapshotInfo(data);
    if (!ret) {
        HILOGE("WriteSnapshotInfo failed!");
        return ERR_FLATTEN_OBJECT;
    }
    ret = snapshot.WritePixelMap(data);
    if (!ret) {
        HILOGE("WritePixelMap failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_OK;
}

void DistributedSchedMissionManager::OnDnetDied()
{
    auto dnetDiedFunc = [this]() {
        HILOGI("OnDnetDied");
        std::lock_guard<std::mutex> autoLock(remoteSyncDeviceLock_);
        if (!isRegMissionChange_) {
            return;
        }
        remoteSyncDeviceSet_.clear();
        DistributedSchedAdapter::GetInstance().UnRegisterMissionListener(missonChangeListener_);
        isRegMissionChange_ = false;
    };
    if (missionHandler_ != nullptr) {
        missionHandler_->PostTask(dnetDiedFunc);
    }
}
} // namespace DistributedSchedule
} // namespace OHOS
