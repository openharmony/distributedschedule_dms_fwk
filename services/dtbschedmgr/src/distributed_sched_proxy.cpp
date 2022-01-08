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

#include "distributed_sched_proxy.h"

#include "dtbschedmgr_log.h"

#include "ipc_types.h"
#include "mission/mission_info_converter.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;

namespace {
const std::string TAG = "DistributedSchedProxy";
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
constexpr int32_t WAIT_TIME = 15;
}

int32_t DistributedSchedProxy::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode)
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
    int32_t status)
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

int32_t DistributedSchedProxy::ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid)
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
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedSchedProxy::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect)
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
    return DstbMissionInfo::ReadMissionInfoVectorFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
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
    return ERR_NONE;
}

int32_t DistributedSchedProxy::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    return ERR_NONE;
}

int32_t DistributedSchedProxy::GetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<DstbMissionInfo>& missionInfos)
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
    return DstbMissionInfo::ReadMissionInfoVectorFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
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
    if (!DstbMissionInfo::WriteMissionInfoVectorFromParcel(data, missionInfos)) {
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

int32_t DistributedSchedProxy::StoreSnapshotInfo(const std::string& deviceId,
                                                 int32_t missionId,
                                                 const uint8_t* byteStream,
                                                 size_t len)
{
    return ERR_NONE;
}

int32_t DistributedSchedProxy::RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId)
{
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
} // namespace DistributedSchedule
} // namespace OHOS

