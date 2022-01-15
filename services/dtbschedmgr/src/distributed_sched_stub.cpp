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

#include "distributed_sched_stub.h"

#include "ability_info.h"
#include "adapter/dnetwork_adapter.h"
#include "caller_info.h"
#include "datetime_ex.h"
#include "dtbschedmgr_log.h"
#include "dtbschedmgr_device_info_storage.h"
#include "image_source.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "mission/distributed_sched_mission_manager.h"
#include "mission/snapshot_converter.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;
using namespace AppExecFwk;

namespace {
constexpr int32_t HID_HAP = 10000; /* first hap user */
const std::string TAG = "DistributedSchedStub";
const std::u16string DMS_STUB_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}

DistributedSchedStub::DistributedSchedStub()
{
    localFuncsMap_[START_REMOTE_ABILITY] = &DistributedSchedStub::StartRemoteAbilityInner;
    localFuncsMap_[CONTINUE_MISSION] = &DistributedSchedStub::ContinueMissionInner;
    localFuncsMap_[START_CONTINUATION] = &DistributedSchedStub::StartContinuationInner;
    localFuncsMap_[NOTIFY_COMPLETE_CONTINUATION] = &DistributedSchedStub::NotifyCompleteContinuationInner;
    localFuncsMap_[CONNECT_REMOTE_ABILITY] = &DistributedSchedStub::ConnectRemoteAbilityInner;
    localFuncsMap_[DISCONNECT_REMOTE_ABILITY] = &DistributedSchedStub::DisconnectRemoteAbilityInner;
    // request codes for mission mananger
    localFuncsMap_[CHECK_SUPPORTED_OSD] = &DistributedSchedStub::CheckSupportOsdInner;
    localFuncsMap_[STORE_SNAPSHOT_INFO] = &DistributedSchedStub::StoreSnapshotInfoInner;
    localFuncsMap_[REMOVE_SNAPSHOT_INFO] = &DistributedSchedStub::RemoveSnapshotInfoInner;
    localFuncsMap_[GET_REMOTE_SNAPSHOT_INFO] = &DistributedSchedStub::GetRemoteSnapshotInfoInner;
    localFuncsMap_[GET_REMOTE_MISSION_SNAPSHOT_INFO] = &DistributedSchedStub::GetRemoteMissionSnapshotInfoInner;
    localFuncsMap_[REGISTER_MISSION_LISTENER] = &DistributedSchedStub::RegisterMissionListenerInner;
    localFuncsMap_[UNREGISTER_MISSION_LISTENER] = &DistributedSchedStub::UnRegisterMissionListenerInner;
    localFuncsMap_[GET_MISSION_INFOS] = &DistributedSchedStub::GetMissionInfosInner;
    localFuncsMap_[START_SYNC_MISSIONS] = &DistributedSchedStub::StartSyncRemoteMissionsInner;
    localFuncsMap_[STOP_SYNC_MISSIONS] = &DistributedSchedStub::StopSyncRemoteMissionsInner;
    localFuncsMap_[SWITCH_CHANGED] = &DistributedSchedStub::NotifyOsdSwitchChangedInner;
    localFuncsMap_[GET_CACHED_SUPPORTED_OSD] = &DistributedSchedStub::GetCachedOsdSwitchInner;

    remoteFuncsMap_[START_ABILITY_FROM_REMOTE] = &DistributedSchedStub::StartAbilityFromRemoteInner;
    remoteFuncsMap_[NOTIFY_CONTINUATION_RESULT_FROM_REMOTE] =
        &DistributedSchedStub::NotifyContinuationResultFromRemoteInner;
    remoteFuncsMap_[CONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::ConnectAbilityFromRemoteInner;
    remoteFuncsMap_[DISCONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::DisconnectAbilityFromRemoteInner;
    remoteFuncsMap_[NOTIFY_PROCESS_DIED_FROM_REMOTE] = &DistributedSchedStub::NotifyProcessDiedFromRemoteInner;
    // request codes for mission mananger
    remoteFuncsMap_[CHECK_SUPPORT_OSD_FROM_REMOTE] = &DistributedSchedStub::GetOsdSwitchValueFromRemoteInner;
    remoteFuncsMap_[START_SYNC_MISSIONS_FROM_REMOTE] =
        &DistributedSchedStub::StartSyncMissionsFromRemoteInner;
    remoteFuncsMap_[STOP_SYNC_MISSIONS_FROM_REMOTE] =
        &DistributedSchedStub::StopSyncMissionsFromRemoteInner;
    remoteFuncsMap_[NOTIFY_MISSIONS_CHANGED_FROM_REMOTE] = &DistributedSchedStub::NotifyMissionsChangedFromRemoteInner;
    remoteFuncsMap_[NOTIFY_SWITCH_CHANGED_FROM_REMOTE] = &DistributedSchedStub::UpdateOsdSwitchValueFromRemoteInner;
    remoteFuncsMap_[CONTINUE_MISSION] = &DistributedSchedStub::ContinueMissionInner;
}

DistributedSchedStub::~DistributedSchedStub()
{
    remoteFuncsMap_.clear();
    localFuncsMap_.clear();
}

int32_t DistributedSchedStub::OnRemoteRequest(uint32_t code,
    MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    bool IsLocalCalling = IPCSkeleton::IsLocalCalling();
    HILOGI("OnRemoteRequest, code = %{public}d, flags = %{public}d, IsLocalCalling = %{public}d.",
        code, option.GetFlags(), IsLocalCalling);

    const auto& funcsMap = IsLocalCalling ? localFuncsMap_ : remoteFuncsMap_;
    auto iter = funcsMap.find(code);
    if (iter != funcsMap.end()) {
        auto func = iter->second;
        if (!EnforceInterfaceToken(data)) {
            HILOGW("OnRemoteRequest interface token check failed!");
            return DMS_PERMISSION_DENIED;
        }
        if (func != nullptr) {
            return (this->*func)(data, reply);
        }
    }

    HILOGW("OnRemoteRequest default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DistributedSchedStub::StartRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("START_ABILITY want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    HILOGI("get callerUid = %{public}d", callerUid);
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    int32_t result = StartRemoteAbility(*want, callerUid, requestCode);
    HILOGI("StartRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<CompatibleAbilityInfo> cmpAbilityInfo(data.ReadParcelable<CompatibleAbilityInfo>());
    if (cmpAbilityInfo == nullptr) {
        HILOGW("AbilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    AbilityInfo abilityInfo;
    cmpAbilityInfo->ConvertToAbilityInfo(abilityInfo);
    std::string package = abilityInfo.bundleName;
    std::string deviceId = abilityInfo.deviceId;
    int64_t begin = GetTickCount();
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    AccountInfo accountInfo;
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    int32_t result = StartAbilityFromRemote(*want, abilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    PARCEL_WRITE_HELPER(reply, String, package);
    PARCEL_WRITE_HELPER(reply, String, deviceId);
    return ERR_NONE;
}

int32_t DistributedSchedStub::ContinueMissionInner(MessageParcel& data, MessageParcel& reply)
{
    std::string srcDevId;
    std::string dstDevId;
    PARCEL_READ_HELPER(data, String, srcDevId);
    PARCEL_READ_HELPER(data, String, dstDevId);
    int32_t missionId = 0;
    PARCEL_READ_HELPER(data, Int32, missionId);
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOGW("read callback failed!");
        return ERR_NULL_OBJECT;
    }
    shared_ptr<AAFwk::WantParams> wantParams(data.ReadParcelable<AAFwk::WantParams>());
    if (wantParams == nullptr) {
        HILOGW("wantParams readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = ContinueMission(srcDevId, dstDevId, missionId, callback, *wantParams);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartContinuationInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t missionId = data.ReadInt32();
    int32_t callerUid = data.ReadInt32();
    int32_t status = data.ReadInt32();
    int32_t result = StartContinuation(*want, missionId, callerUid, status);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::NotifyCompleteContinuationInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGE("devId is empty!");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t sessionId = 0;
    PARCEL_READ_HELPER(data, Int32, sessionId);
    bool continuationResult = false;
    PARCEL_READ_HELPER(data, Bool, continuationResult);
    NotifyCompleteContinuation(devId, sessionId, continuationResult);
    return ERR_OK;
}

int32_t DistributedSchedStub::NotifyContinuationResultFromRemoteInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    int32_t sessionId = 0;
    PARCEL_READ_HELPER(data, Int32, sessionId);
    bool continuationResult = false;
    PARCEL_READ_HELPER(data, Bool, continuationResult);
    return NotifyContinuationResultFromRemote(sessionId, continuationResult);
}

int32_t DistributedSchedStub::ConnectRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    HILOGI("get callerUid = %{public}d", callerUid);
    int32_t callerPid = 0;
    PARCEL_READ_HELPER(data, Int32, callerPid);
    HILOGI("get callerPid = %{public}d", callerPid);
    int32_t result = ConnectRemoteAbility(*want, connect, callerUid, callerPid);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::DisconnectRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t result = DisconnectRemoteAbility(connect);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::ConnectAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    unique_ptr<CompatibleAbilityInfo> cmpAbilityInfo(data.ReadParcelable<CompatibleAbilityInfo>());
    if (cmpAbilityInfo == nullptr) {
        HILOGW("abilityInfo readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    AbilityInfo abilityInfo;
    cmpAbilityInfo->ConvertToAbilityInfo(abilityInfo);
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, Int32, callerInfo.pid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    AccountInfo accountInfo;
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    std::string package = abilityInfo.bundleName;
    std::string deviceId = abilityInfo.deviceId;
    int64_t begin = GetTickCount();
    int32_t result = ConnectAbilityFromRemote(*want, abilityInfo, connect, callerInfo, accountInfo);
    HILOGW("result = %{public}d", result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int32, result);
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    PARCEL_WRITE_HELPER(reply, String, package);
    PARCEL_WRITE_HELPER(reply, String, deviceId);
    return ERR_NONE;
}

int32_t DistributedSchedStub::DisconnectAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t uid = 0;
    PARCEL_READ_HELPER(data, Int32, uid);
    string sourceDeviceId;
    PARCEL_READ_HELPER(data, String, sourceDeviceId);
    int32_t result = DisconnectAbilityFromRemote(connect, uid, sourceDeviceId);
    HILOGI("result %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::NotifyProcessDiedFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    int32_t uid = 0;
    PARCEL_READ_HELPER(data, Int32, uid);
    int32_t pid = 0;
    PARCEL_READ_HELPER(data, Int32, pid);
    string sourceDeviceId;
    PARCEL_READ_HELPER(data, String, sourceDeviceId);
    CallerInfo callerInfo;
    callerInfo.uid = uid;
    callerInfo.pid = pid;
    callerInfo.sourceDeviceId = sourceDeviceId;
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    int32_t result = NotifyProcessDiedFromRemote(callerInfo);
    HILOGI("result %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

bool DistributedSchedStub::CheckCallingUid()
{
    // never allow non-system uid for distributed request
    auto callingUid = IPCSkeleton::GetCallingUid();
    return callingUid < HID_HAP;
}

bool DistributedSchedStub::EnforceInterfaceToken(MessageParcel& data)
{
    u16string interfaceToken = data.ReadInterfaceToken();
    return interfaceToken == DMS_STUB_INTERFACE_TOKEN;
}

int32_t DistributedSchedStub::GetMissionInfosInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    std::u16string deviceId = data.ReadString16();
    int32_t numMissions = 0;
    PARCEL_READ_HELPER(data, Int32, numMissions);

    std::vector<DstbMissionInfo> missionInfos;
    int result = GetMissionInfos(Str16ToStr8(deviceId), numMissions, missionInfos);
    HILOGI("result = %{public}d", result);
    if (result == ERR_NONE) {
        return DstbMissionInfo::WriteMissionInfoVectorFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
    }
    return ERR_NULL_OBJECT;
}

int32_t DistributedSchedStub::GetRemoteSnapshotInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    u16string deviceId = data.ReadString16();
    int32_t missionId = 0;
    PARCEL_READ_HELPER(data, Int32, missionId);
    if (deviceId.empty()) {
        HILOGE("deviceId is empty!");
        return ERR_FLATTEN_OBJECT;
    }
    unique_ptr<Snapshot> snapShot = GetRemoteSnapshotInfo(deviceId, missionId);
    if (snapShot == nullptr) {
        HILOGE("snapShot object nullptr!");
        return ERR_NULL_OBJECT;
    }
    bool ret = snapShot->WriteToParcel(reply);
    if (!ret) {
        HILOGE("snapShot WriteToParcel failed!");
        return ERR_NULL_OBJECT;
    }
    std::string uuid = DnetworkAdapter::GetInstance()->GetUuidByNetworkId(Str16ToStr8(deviceId));
    if (uuid.empty()) {
        HILOGE("uuid is empty!");
        return ERR_NULL_OBJECT;
    }
    DistributedSchedMissionManager::GetInstance().EnqueueCachedSnapshotInfo(uuid,
        missionId, std::move(snapShot));
    return ERR_NONE;
}

int32_t DistributedSchedStub::GetRemoteMissionSnapshotInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    string networkId = data.ReadString();
    if (networkId.empty()) {
        HILOGE("networkId is empty!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t missionId = 0;
    PARCEL_READ_HELPER(data, Int32, missionId);
    if (missionId < 0) {
        HILOGE("missionId is invalid");
        return INVALID_PARAMETERS_ERR;
    }
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr = std::make_unique<MissionSnapshot>();
    int32_t errCode = GetRemoteMissionSnapshotInfo(networkId, missionId, missionSnapshotPtr);
    if (errCode != ERR_NONE) {
        HILOGE("get mission snapshot failed!");
        return ERR_NULL_OBJECT;
    }
    PARCEL_WRITE_HELPER(reply, Parcelable, missionSnapshotPtr.get());
    std::string uuid = DnetworkAdapter::GetInstance()->GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        HILOGE("uuid is empty!");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<Snapshot> snapshotPtr = make_unique<Snapshot>();
    SnapshotConverter::ConvertToSnapshot(*missionSnapshotPtr, snapshotPtr);
    DistributedSchedMissionManager::GetInstance().EnqueueCachedSnapshotInfo(uuid,
        missionId, std::move(snapshotPtr));
    return ERR_NONE;
}

int32_t DistributedSchedStub::RegisterMissionListenerInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    if (!EnforceInterfaceToken(data)) {
        HILOGW("RegisterMissionListenerInner interface token check failed!");
        return DMS_PERMISSION_DENIED;
    }
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGW("read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> missionChangedListener = data.ReadRemoteObject();
    if (missionChangedListener == nullptr) {
        HILOGW("read IRemoteObject failed!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = RegisterMissionListener(devId, missionChangedListener);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::UnRegisterMissionListenerInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGW("read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    sptr<IRemoteObject> missionChangedListener = data.ReadRemoteObject();
    if (missionChangedListener == nullptr) {
        HILOGW("read IRemoteObject failed!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = UnRegisterMissionListener(devId, missionChangedListener);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartSyncMissionsFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    CallerInfo callerInfo;
    if (!CallerInfoUnmarshalling(callerInfo, data)) {
        HILOGW("read callerInfo failed!");
        return ERR_FLATTEN_OBJECT;
    }
    std::vector<DstbMissionInfo> missionInfos;
    if (StartSyncMissionsFromRemote(callerInfo, missionInfos) != ERR_NONE) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!reply.WriteInt32(VERSION)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!DstbMissionInfo::WriteMissionInfoVectorFromParcel(reply, missionInfos)) {
        HILOGE("write mission info failed!");
        return ERR_FLATTEN_OBJECT;
    }
    return ERR_NONE;
}

int32_t DistributedSchedStub::StopSyncRemoteMissionsInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGW("read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = StopSyncRemoteMissions(Str16ToStr8(devId));
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StopSyncMissionsFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    CallerInfo callerInfo;
    if (!CallerInfoUnmarshalling(callerInfo, data)) {
        HILOGW("read callerInfo failed!");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t result = StopSyncMissionsFromRemote(callerInfo);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::CheckSupportOsdInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    int64_t begin = GetTickCount();
    std::u16string deviceId = data.ReadString16();
    std::string u8DeviceId = Str16ToStr8(deviceId);
    int32_t result = CheckSupportOsd(u8DeviceId);
    HILOGI("result = %{public}d, deviceId = %{public}s spend %{public}" PRId64 " ms",
        result, DnetworkAdapter::AnonymizeDeviceId(u8DeviceId).c_str(), GetTickCount() - begin);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::GetCachedOsdSwitchInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    std::vector<std::u16string> deviceIds;
    std::vector<int32_t> values;
    int64_t begin = GetTickCount();
    GetCachedOsdSwitch(deviceIds, values);
    HILOGI("spend %{public}" PRId64 " ms", GetTickCount() - begin);
    PARCEL_WRITE_HELPER(reply, String16Vector, deviceIds);
    PARCEL_WRITE_HELPER(reply, Int32Vector, values);
    return ERR_NONE;
}

int32_t DistributedSchedStub::NotifyMissionsChangedFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t version = data.ReadInt32();
    HILOGD("version is %{public}d", version);
    std::vector<DstbMissionInfo> missionInfos;
    if (!DstbMissionInfo::ReadMissionInfoVectorFromParcel(data, missionInfos)) {
        return ERR_FLATTEN_OBJECT;
    }
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = data.ReadString();
    callerInfo.uid = data.ReadInt32();
    callerInfo.pid = data.ReadInt32();
    callerInfo.dmsVersion = data.ReadInt32();
    int32_t result = NotifyMissionsChangedFromRemote(missionInfos, callerInfo);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::GetOsdSwitchValueFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t result = GetOsdSwitchValueFromRemote();
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::StoreSnapshotInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    int32_t missionId = 0;
    if (!data.ReadInt32(missionId)) {
        return ERR_FLATTEN_OBJECT;
    }
    size_t len = data.GetReadableBytes();
    const uint8_t* byteSteam = data.ReadBuffer(len);
    std::string deviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(deviceId);
    if (deviceId.empty()) {
        HILOGE("get deviceId failed!");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t result = StoreSnapshotInfo(deviceId, missionId, byteSteam, len);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::RemoveSnapshotInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    int32_t missionId = 0;
    if (!data.ReadInt32(missionId)) {
        return ERR_FLATTEN_OBJECT;
    }
    std::string deviceId;
    DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(deviceId);
    if (deviceId.empty()) {
        HILOGE("get deviceId failed!");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t result = RemoveSnapshotInfo(deviceId, missionId);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::NotifyOsdSwitchChangedInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t result = DistributedSchedMissionManager::GetInstance().UpdateSwitchValueToRemote();
    HILOGI("result: %{public}d!", result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::UpdateOsdSwitchValueFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t switchVal = data.ReadInt32();
    std::string srcUuid = data.ReadString();
    int32_t result = UpdateOsdSwitchValueFromRemote(switchVal, srcUuid);
    HILOGD("srcUuid: %s, result: %d!",
        srcUuid.c_str(), result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartSyncRemoteMissionsInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest], IPC end = %{public}" PRId64, GetTickCount());
    u16string devId = data.ReadString16();
    if (devId.empty()) {
        HILOGW("read deviceId failed!");
        return ERR_FLATTEN_OBJECT;
    }
    string deviceId = Str16ToStr8(devId);
    bool fixConflict = data.ReadBool();
    long tag = data.ReadInt64();
    int32_t result = StartSyncRemoteMissions(deviceId, fixConflict, tag);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

bool DistributedSchedStub::CallerInfoUnmarshalling(CallerInfo& callerInfo, MessageParcel& data)
{
    int32_t uid = -1;
    PARCEL_READ_HELPER_RET(data, Int32, uid, false);
    int32_t pid = -1;
    PARCEL_READ_HELPER_RET(data, Int32, pid, false);
    int32_t callerType = CALLER_TYPE_NONE;
    PARCEL_READ_HELPER_RET(data, Int32, callerType, false);
    std::string sourceDeviceId;
    PARCEL_READ_HELPER_RET(data, String, sourceDeviceId, false);
    HILOGI("sourceDeviceId = %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(sourceDeviceId).c_str());
    int32_t duid = -1;
    PARCEL_READ_HELPER_RET(data, Int32, duid, false);
    std::string callerAppId;
    PARCEL_READ_HELPER_RET(data, String, callerAppId, false);
    int32_t version = -1;
    PARCEL_READ_HELPER_RET(data, Int32, version, false);
    callerInfo.uid = uid;
    callerInfo.pid = pid;
    callerInfo.callerType = callerType;
    callerInfo.sourceDeviceId = sourceDeviceId;
    callerInfo.duid = duid;
    callerInfo.callerAppId = callerAppId;
    callerInfo.dmsVersion = version;
    return true;
}
} // namespace DistributedSchedule
} // namespace OHOS
