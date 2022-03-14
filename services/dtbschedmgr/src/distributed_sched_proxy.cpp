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

#include "distributed_sched_proxy.h"

#include "dtbschedmgr_log.h"

#include "ipc_types.h"
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
#include "mission/mission_info_converter.h"
#include "pixel_map.h"
#endif
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;

namespace {
const std::string TAG = "DistributedSchedProxy";
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
const std::string EXTRO_INFO_JSON_KEY_ACCESS_TOKEN = "accessTokenID";
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
constexpr int32_t WAIT_TIME = 15;
#endif
}

int32_t DistributedSchedProxy::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode, uint32_t accessToken)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    AppExecFwk::CompatibleAbilityInfo compatibleAbilityInfo;
    abilityInfo.ConvertToCompatiableAbilityInfo(compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Parcelable, &compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, accountInfo.accountType);
    PARCEL_WRITE_HELPER(data, StringVector, accountInfo.groupIdList);
    PARCEL_WRITE_HELPER(data, String, callerInfo.callerAppId);
    nlohmann::json extraInfoJson;
    extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN] = callerInfo.accessToken;
    std::string extraInfo = extraInfoJson.dump();
    PARCEL_WRITE_HELPER(data, String, extraInfo);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    if (callback == nullptr) {
        HILOGE("ContinueMission callback null");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ContinueMission remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, srcDeviceId);
    PARCEL_WRITE_HELPER(data, String, dstDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, RemoteObject, callback);
    PARCEL_WRITE_HELPER(data, Parcelable, &wantParams);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONTINUE_MISSION, data, reply);
}

int32_t DistributedSchedProxy::StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
    int32_t status, uint32_t accessToken)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("StartContinuation remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, status);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_CONTINUATION, data, reply);
}

void DistributedSchedProxy::NotifyCompleteContinuation(const std::u16string& devId, int32_t sessionId, bool isSuccess)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyCompleteContinuation remote service null");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return;
    }
    PARCEL_WRITE_HELPER_NORET(data, String16, devId);
    PARCEL_WRITE_HELPER_NORET(data, Int32, sessionId);
    PARCEL_WRITE_HELPER_NORET(data, Bool, isSuccess);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_NORET(remote, NOTIFY_COMPLETE_CONTINUATION, data, reply);
}

int32_t DistributedSchedProxy::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyContinuationResultFromRemote remote service null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, sessionId);
    PARCEL_WRITE_HELPER(data, Bool, isSuccess);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, NOTIFY_CONTINUATION_RESULT_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
    int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    if (connect == nullptr) {
        HILOGE("ConnectRemoteAbility connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ConnectRemoteAbility remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, callerPid);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid,
    uint32_t accessToken)
{
    if (connect == nullptr) {
        HILOGE("DisconnectRemoteAbility connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("DisconnectRemoteAbility remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DISCONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    if (connect == nullptr) {
        HILOGE("ConnectAbilityFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ConnectAbilityFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    AppExecFwk::CompatibleAbilityInfo compatibleAbilityInfo;
    abilityInfo.ConvertToCompatiableAbilityInfo(compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, Parcelable, &compatibleAbilityInfo);
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, accountInfo.accountType);
    PARCEL_WRITE_HELPER(data, StringVector, accountInfo.groupIdList);
    PARCEL_WRITE_HELPER(data, String, callerInfo.callerAppId);
    nlohmann::json extraInfoJson;
    extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN] = callerInfo.accessToken;
    std::string extraInfo = extraInfoJson.dump();
    PARCEL_WRITE_HELPER(data, String, extraInfo);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    if (connect == nullptr) {
        HILOGE("DisconnectAbilityFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("DisconnectAbilityFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, uid);
    PARCEL_WRITE_HELPER(data, String, sourceDeviceId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DISCONNECT_ABILITY_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    HILOGD("DistributedSchedProxy::NotifyProcessDiedFromRemote called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyProcessDiedFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, NOTIFY_PROCESS_DIED_FROM_REMOTE, data, reply);
}

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
int32_t DistributedSchedProxy::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote system abiity is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(devId));
    PARCEL_WRITE_HELPER(data, Bool, fixConflict);
    PARCEL_WRITE_HELPER(data, Int64, tag);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_SYNC_MISSIONS, data, reply);
}

int32_t DistributedSchedProxy::StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
    std::vector<DstbMissionInfo>& missionInfos)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote service is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC, WAIT_TIME };
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!CallerInfoMarshalling(callerInfo, data)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t error = remote->SendRequest(START_SYNC_MISSIONS_FROM_REMOTE, data, reply, option);
    if (error != ERR_NONE) {
        HILOGW("fail, error: %{public}d", error);
        return error;
    }
    int32_t version = reply.ReadInt32();
    HILOGD("version : %{public}d", version);
    return DstbMissionInfo::ReadDstbMissionInfosFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
}

int32_t DistributedSchedProxy::StopSyncRemoteMissions(const std::string& devId)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote system abiity is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(devId));
    PARCEL_TRANSACT_SYNC_RET_INT(remote, STOP_SYNC_MISSIONS, data, reply);
}

int32_t DistributedSchedProxy::StopSyncMissionsFromRemote(const CallerInfo& callerInfo)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote service is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC, WAIT_TIME };
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!CallerInfoMarshalling(callerInfo, data)) {
        return ERR_FLATTEN_OBJECT;
    }
    int32_t error = remote->SendRequest(STOP_SYNC_MISSIONS_FROM_REMOTE, data, reply, option);
    if (error != ERR_NONE) {
        HILOGW("sendRequest fail, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t DistributedSchedProxy::RegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    HILOGI("RegisterMissionListener called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote system abiity is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, devId);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_MISSION_LISTENER, data, reply);
}

int32_t DistributedSchedProxy::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    HILOGI("UnRegisterMissionListener called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote system abiity is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, devId);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UNREGISTER_MISSION_LISTENER, data, reply);
}

int32_t DistributedSchedProxy::GetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote system abiity is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(deviceId));
    PARCEL_WRITE_HELPER(data, Int32, numMissions);
    int32_t ret = remote->SendRequest(GET_MISSION_INFOS, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGW("sendRequest fail, error: %{public}d", ret);
        return ret;
    }
    return MissionInfoConverter::ReadMissionInfosFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
}

int32_t DistributedSchedProxy::NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
    const CallerInfo& callerInfo)
{
    HILOGI("NotifyMissionsChangedFromRemote is called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("NotifyMissionsChangedFromRemote remote service is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.dmsVersion);
    if (!DstbMissionInfo::WriteDstbMissionInfosToParcel(data, missionInfos)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.dmsVersion);
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC, WAIT_TIME };
    int32_t error = remote->SendRequest(NOTIFY_MISSIONS_CHANGED_FROM_REMOTE, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("%{public}s transact failed, error: %{public}d", __func__, error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    HILOGD("%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int32_t DistributedSchedProxy::CheckSupportOsd(const std::string& deviceId)
{
    HILOGI("CheckSupportOsd is called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("CheckSupportOsd remote service is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, deviceId);
    return remote->SendRequest(CHECK_SUPPORTED_OSD, data, reply, option);
}

void DistributedSchedProxy::GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values)
{
    HILOGI("GetCachedOsdSwitch is called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("GetCachedOsdSwitch remote service is null");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return;
    }
    int32_t error = remote->SendRequest(GET_CACHED_SUPPORTED_OSD, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("%{public}s transact failed, error: %{public}d", __func__, error);
        return;
    }
    PARCEL_READ_HELPER_NORET(reply, String16Vector, &deviceIds);
    PARCEL_READ_HELPER_NORET(reply, Int32Vector, &values);
    return;
}

int32_t DistributedSchedProxy::GetOsdSwitchValueFromRemote()
{
    HILOGI("GetOsdSwitchValueFromRemote is called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("GetOsdSwitchValueFromRemote remote service is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC, WAIT_TIME };
    int32_t error = remote->SendRequest(CHECK_SUPPORT_OSD_FROM_REMOTE, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("%{public}s transact failed, error: %{public}d", __func__, error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    HILOGD("%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}

int32_t DistributedSchedProxy::StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    return ERR_NONE;
}

int32_t DistributedSchedProxy::RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId)
{
    return ERR_NONE;
}

int32_t DistributedSchedProxy::GetRemoteMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    std::unique_ptr<MissionSnapshot>& missionSnapshot)
{
    if (networkId.empty()) {
        HILOGE("networkId is null");
        return ERR_NULL_OBJECT;
    }
    if (missionId < 0) {
        HILOGE("missionId is invalid");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, networkId);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(GET_REMOTE_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr(reply.ReadParcelable<MissionSnapshot>());
    missionSnapshot = std::move(missionSnapshotPtr);
    return ERR_NONE;
}

int32_t DistributedSchedProxy::UpdateOsdSwitchValueFromRemote(int32_t switchVal,
    const std::string& sourceDeviceId)
{
    HILOGD("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote service is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, switchVal);
    PARCEL_WRITE_HELPER(data, String, sourceDeviceId);
    MessageParcel reply;
    MessageOption option { MessageOption::TF_SYNC, WAIT_TIME };
    int32_t error = remote->SendRequest(NOTIFY_SWITCH_CHANGED_FROM_REMOTE, data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("%{public}s transact failed, error: %{public}d", __func__, error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    HILOGD("%{public}s get result from server data = %{public}d", __func__, result);
    return result;
}
#endif

bool DistributedSchedProxy::CallerInfoMarshalling(const CallerInfo& callerInfo, MessageParcel& data)
{
    PARCEL_WRITE_HELPER_RET(data, Int32, callerInfo.uid, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerInfo.pid, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerInfo.callerType, false);
    PARCEL_WRITE_HELPER_RET(data, String, callerInfo.sourceDeviceId, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerInfo.duid, false);
    PARCEL_WRITE_HELPER_RET(data, String, callerInfo.callerAppId, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerInfo.dmsVersion, false);
    return true;
}

int32_t DistributedSchedProxy::StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    if (connect == nullptr) {
        HILOGE("StartRemoteAbilityByCall connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("StartRemoteAbilityByCall remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, callerPid);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_ABILITY_BY_CALL, data, reply);
}

int32_t DistributedSchedProxy::ReleaseRemoteAbility(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element)
{
    if (connect == nullptr) {
        HILOGE("ReleaseRemoteAbility connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ReleaseRemoteAbility remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    if (!data.WriteParcelable(&element)) {
        HILOGE("ReleaseRemoteAbility write element error.");
        return ERR_INVALID_VALUE;
    }
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, RELEASE_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::StartAbilityByCallFromRemote(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    if (connect == nullptr) {
        HILOGE("StartAbilityByCallFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("StartAbilityByCallFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.uid);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.pid);
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, accountInfo.accountType);
    PARCEL_WRITE_HELPER(data, StringVector, accountInfo.groupIdList);
    PARCEL_WRITE_HELPER(data, String, callerInfo.callerAppId);
    nlohmann::json extraInfoJson;
    extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN] = callerInfo.accessToken;
    std::string extraInfo = extraInfoJson.dump();
    PARCEL_WRITE_HELPER(data, String, extraInfo);
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_ABILITY_BY_CALL_FROM_REMOTE, data, reply);
}

int32_t DistributedSchedProxy::ReleaseAbilityFromRemote(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element, const CallerInfo& callerInfo)
{
    if (connect == nullptr) {
        HILOGE("ReleaseAbilityFromRemote connect is null");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("ReleaseAbilityFromRemote remote is null");
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    if (!data.WriteParcelable(&element)) {
        HILOGE("ReleaseAbilityFromRemote write element error.");
        return ERR_INVALID_VALUE;
    }
    PARCEL_WRITE_HELPER(data, String, callerInfo.sourceDeviceId);
    std::string extraInfo = "";
    PARCEL_WRITE_HELPER(data, String, extraInfo);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, RELEASE_ABILITY_FROM_REMOTE, data, reply);
}
} // namespace DistributedSchedule
} // namespace OHOS

