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
#include "dfx/dms_hisysevent_report.h"
#include "dfx/dms_hitrace_chain.h"
#include "dfx/dms_hitrace_constants.h"
#include "distributed_sched_permission.h"
#include "dtbschedmgr_log.h"
#include "dtbschedmgr_device_info_storage.h"
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
#include "image_source.h"
#include "mission/distributed_sched_mission_manager.h"
#include "mission/mission_info_converter.h"
#include "mission/snapshot_converter.h"
#endif
#include "ipc_skeleton.h"
#include "message_parcel.h"

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
const std::string EXTRO_INFO_JSON_KEY_ACCESS_TOKEN = "accessTokenID";
const std::string EXTRO_INFO_JSON_KEY_REQUEST_CODE = "requestCode";
const std::string PERMISSION_DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";
const int DEFAULT_REQUEST_CODE = -1;
}

DistributedSchedStub::DistributedSchedStub()
{
    localFuncsMap_[START_REMOTE_ABILITY] = &DistributedSchedStub::StartRemoteAbilityInner;
    localFuncsMap_[CONTINUE_MISSION] = &DistributedSchedStub::ContinueMissionInner;
    localFuncsMap_[START_CONTINUATION] = &DistributedSchedStub::StartContinuationInner;
    localFuncsMap_[NOTIFY_COMPLETE_CONTINUATION] = &DistributedSchedStub::NotifyCompleteContinuationInner;
    localFuncsMap_[CONNECT_REMOTE_ABILITY] = &DistributedSchedStub::ConnectRemoteAbilityInner;
    localFuncsMap_[DISCONNECT_REMOTE_ABILITY] = &DistributedSchedStub::DisconnectRemoteAbilityInner;
    // request codes for mission manager
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    localFuncsMap_[CHECK_SUPPORTED_OSD] = &DistributedSchedStub::CheckSupportOsdInner;
    localFuncsMap_[GET_REMOTE_MISSION_SNAPSHOT_INFO] = &DistributedSchedStub::GetRemoteMissionSnapshotInfoInner;
    localFuncsMap_[REGISTER_MISSION_LISTENER] = &DistributedSchedStub::RegisterMissionListenerInner;
    localFuncsMap_[UNREGISTER_MISSION_LISTENER] = &DistributedSchedStub::UnRegisterMissionListenerInner;
    localFuncsMap_[GET_MISSION_INFOS] = &DistributedSchedStub::GetMissionInfosInner;
    localFuncsMap_[START_SYNC_MISSIONS] = &DistributedSchedStub::StartSyncRemoteMissionsInner;
    localFuncsMap_[STOP_SYNC_MISSIONS] = &DistributedSchedStub::StopSyncRemoteMissionsInner;
    localFuncsMap_[SWITCH_CHANGED] = &DistributedSchedStub::NotifyOsdSwitchChangedInner;
    localFuncsMap_[GET_CACHED_SUPPORTED_OSD] = &DistributedSchedStub::GetCachedOsdSwitchInner;
#endif
    remoteFuncsMap_[START_ABILITY_FROM_REMOTE] = &DistributedSchedStub::StartAbilityFromRemoteInner;
    remoteFuncsMap_[SEND_RESULT_FROM_REMOTE] = &DistributedSchedStub::SendResultFromRemoteInner;
    remoteFuncsMap_[NOTIFY_CONTINUATION_RESULT_FROM_REMOTE] =
        &DistributedSchedStub::NotifyContinuationResultFromRemoteInner;
    remoteFuncsMap_[CONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::ConnectAbilityFromRemoteInner;
    remoteFuncsMap_[DISCONNECT_ABILITY_FROM_REMOTE] = &DistributedSchedStub::DisconnectAbilityFromRemoteInner;
    remoteFuncsMap_[NOTIFY_PROCESS_DIED_FROM_REMOTE] = &DistributedSchedStub::NotifyProcessDiedFromRemoteInner;
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    // request codes for mission manager
    remoteFuncsMap_[CHECK_SUPPORT_OSD_FROM_REMOTE] = &DistributedSchedStub::GetOsdSwitchValueFromRemoteInner;
    remoteFuncsMap_[START_SYNC_MISSIONS_FROM_REMOTE] =
        &DistributedSchedStub::StartSyncMissionsFromRemoteInner;
    remoteFuncsMap_[STOP_SYNC_MISSIONS_FROM_REMOTE] =
        &DistributedSchedStub::StopSyncMissionsFromRemoteInner;
    remoteFuncsMap_[NOTIFY_MISSIONS_CHANGED_FROM_REMOTE] = &DistributedSchedStub::NotifyMissionsChangedFromRemoteInner;
    remoteFuncsMap_[NOTIFY_SWITCH_CHANGED_FROM_REMOTE] = &DistributedSchedStub::UpdateOsdSwitchValueFromRemoteInner;
#endif
    remoteFuncsMap_[CONTINUE_MISSION] = &DistributedSchedStub::ContinueMissionInner;
    // request codes for call ability
    localFuncsMap_[START_REMOTE_ABILITY_BY_CALL] = &DistributedSchedStub::StartRemoteAbilityByCallInner;
    localFuncsMap_[RELEASE_REMOTE_ABILITY] = &DistributedSchedStub::ReleaseRemoteAbilityInner;
    remoteFuncsMap_[START_ABILITY_BY_CALL_FROM_REMOTE] = &DistributedSchedStub::StartAbilityByCallFromRemoteInner;
    remoteFuncsMap_[RELEASE_ABILITY_FROM_REMOTE] = &DistributedSchedStub::ReleaseAbilityFromRemoteInner;
    localFuncsMap_[REGISTER_DISTRIBUTED_COMPONENT_LISTENER] =
        &DistributedSchedStub::RegisterDistributedComponentListenerInner;
    localFuncsMap_[GET_DISTRIBUTED_COMPONENT_LIST] = &DistributedSchedStub::GetDistributedComponentListInner;
    localFuncsMap_[START_REMOTE_FREE_INSTALL] = &DistributedSchedStub::StartRemoteFreeInstallInner;
    remoteFuncsMap_[START_FREE_INSTALL_FROM_REMOTE] = &DistributedSchedStub::StartFreeInstallFromRemoteInner;
    remoteFuncsMap_[NOTIFY_COMPLETE_FREE_INSTALL_FROM_REMOTE] =
        &DistributedSchedStub::NotifyCompleteFreeInstallFromRemoteInner;
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
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::START_REMOTE_ABILITY);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::START_REMOTE_ABILITY);
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("START_ABILITY want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    int32_t requestCode = 0;
    PARCEL_READ_HELPER(data, Int32, requestCode);
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    HILOGD("get callerUid = %d, AccessTokenID = %u", callerUid, accessToken);
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = StartRemoteAbility(*want, callerUid, requestCode, accessToken);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::START_REMOTE_ABILITY, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerUid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("StartRemoteAbilityInner result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
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
    std::string extraInfo = data.ReadString();
    if (extraInfo.empty()) {
        HILOGD("extra info is empty!");
    }
    nlohmann::json extraInfoJson = nlohmann::json::parse(extraInfo, nullptr, false);
    if (!extraInfoJson.is_discarded()) {
        uint32_t accessToken = extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN];
        callerInfo.accessToken = accessToken;
        HILOGD("parse extra info, accessTokenID = %u", accessToken);
    }
    int32_t result = StartAbilityFromRemote(*want, abilityInfo, requestCode, callerInfo, accountInfo);
    BehaviorEventParam eventParam = { EventCallingType::REMOTE, BehaviorEvent::START_REMOTE_ABILITY, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerInfo.uid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    PARCEL_WRITE_HELPER(reply, String, package);
    PARCEL_WRITE_HELPER(reply, String, deviceId);
    return ERR_NONE;
}

int32_t DistributedSchedStub::SendResultFromRemoteInner(MessageParcel& data, MessageParcel& reply)
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
    int32_t resultCode = 0;
    PARCEL_READ_HELPER(data, Int32, resultCode);
    std::string extraInfo = data.ReadString();
    if (extraInfo.empty()) {
        HILOGD("extra info is empty!");
    }
    callerInfo.extraInfoJson = nlohmann::json::parse(extraInfo, nullptr, false);
    int32_t result = SendResultFromRemote(*want, requestCode, callerInfo, accountInfo, resultCode);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    return ERR_NONE;
}

int32_t DistributedSchedStub::ContinueMissionInner(MessageParcel& data, MessageParcel& reply)
{
    bool isLocalCalling = IPCSkeleton::IsLocalCalling();
    if ((isLocalCalling && !DistributedSchedPermission::GetInstance().IsFoundationCall()) ||
        (!isLocalCalling && !CheckCallingUid())) {
        HILOGE("check permission failed!");
        return DMS_PERMISSION_DENIED;
    }

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
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::START_CONTINUATION);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::START_CONTINUATION);
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t missionId = data.ReadInt32();
    int32_t callerUid = data.ReadInt32();
    int32_t status = data.ReadInt32();
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    HILOGI("get AccessTokenID = %{public}u", accessToken);
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = StartContinuation(*want, missionId, callerUid, status, accessToken);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::START_CONTINUATION, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerUid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::NotifyCompleteContinuationInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        HILOGE("check permission failed!");
        return DMS_PERMISSION_DENIED;
    }

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
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::CONNECT_REMOTE_ABILITY);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::CONNECT_REMOTE_ABILITY);
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    int32_t callerPid = 0;
    PARCEL_READ_HELPER(data, Int32, callerPid);
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    HILOGD("get callerUid = %d, callerPid = %d, AccessTokenID = %u", callerUid, callerPid,
        accessToken);
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = ConnectRemoteAbility(*want, connect, callerUid, callerPid, accessToken);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::CONNECT_REMOTE_ABILITY, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerUid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::DisconnectRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::DISCONNECT_REMOTE_ABILITY);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::DISCONNECT_REMOTE_ABILITY);
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    HILOGD("get callerUid = %d, AccessTokenID = %u", callerUid, accessToken);
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = DisconnectRemoteAbility(connect, callerUid, accessToken);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::DISCONNECT_REMOTE_ABILITY, result };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
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
    std::string extraInfo = data.ReadString();
    if (extraInfo.empty()) {
        HILOGD("extra info is empty!");
    }
    nlohmann::json extraInfoJson = nlohmann::json::parse(extraInfo, nullptr, false);
    if (!extraInfoJson.is_discarded()) {
        uint32_t accessToken = extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN];
        callerInfo.accessToken = accessToken;
        HILOGD("parse extra info, accessTokenID = %u", accessToken);
    }
    std::string package = abilityInfo.bundleName;
    std::string deviceId = abilityInfo.deviceId;
    int64_t begin = GetTickCount();
    int32_t result = ConnectAbilityFromRemote(*want, abilityInfo, connect, callerInfo, accountInfo);
    BehaviorEventParam eventParam = { EventCallingType::REMOTE, BehaviorEvent::CONNECT_REMOTE_ABILITY, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerInfo.uid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
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
    BehaviorEventParam eventParam = { EventCallingType::REMOTE, BehaviorEvent::DISCONNECT_REMOTE_ABILITY, result };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
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

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
int32_t DistributedSchedStub::GetMissionInfosInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    std::u16string deviceId = data.ReadString16();
    int32_t numMissions = 0;
    PARCEL_READ_HELPER(data, Int32, numMissions);

    std::vector<MissionInfo> missionInfos;
    int32_t result = GetMissionInfos(Str16ToStr8(deviceId), numMissions, missionInfos);
    HILOGI("result = %{public}d", result);
    if (result == ERR_NONE) {
        result = MissionInfoConverter::WriteMissionInfosToParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
    }
    return result;
}

int32_t DistributedSchedStub::GetRemoteMissionSnapshotInfoInner(MessageParcel& data, MessageParcel& reply)
{
    HILOGI("[PerformanceTest] called, IPC end = %{public}" PRId64, GetTickCount());
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
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
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
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
    int32_t result = RegisterMissionListener(devId, missionChangedListener);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::UnRegisterMissionListenerInner(MessageParcel& data, MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
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
    if (!DstbMissionInfo::WriteDstbMissionInfosToParcel(reply, missionInfos)) {
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
    if (!DstbMissionInfo::ReadDstbMissionInfosFromParcel(data, missionInfos)) {
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
    int64_t tag = data.ReadInt64();
    int32_t result = StartSyncRemoteMissions(deviceId, fixConflict, tag);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}
#endif

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

int32_t DistributedSchedStub::StartRemoteAbilityByCallInner(MessageParcel& data, MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::START_REMOTE_ABILITY_BYCALL);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::START_REMOTE_ABILITY_BYCALL);
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    int32_t callerUid = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    int32_t callerPid = 0;
    PARCEL_READ_HELPER(data, Int32, callerPid);
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = StartRemoteAbilityByCall(*want, connect, callerUid, callerPid, accessToken);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::START_REMOTE_ABILITY_BYCALL, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerUid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::ReleaseRemoteAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    DmsHiTraceChain hiTraceChain(TraceValue::RELEASE_REMOTE_ABILITY);
    HITRACE_METER_NAME(TraceTag::DSCHED, TraceValue::RELEASE_REMOTE_ABILITY);
    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    shared_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOGE("ReleaseRemoteAbilityInner receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ReleaseRemoteAbility(connect, *element);
    BehaviorEventParam eventParam = { EventCallingType::LOCAL, BehaviorEvent::RELEASE_REMOTE_ABILITY, result,
        element->GetBundleName(), element->GetAbilityName() };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::StartAbilityByCallFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, Int32, callerInfo.pid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    AccountInfo accountInfo;
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    std::string extraInfo = data.ReadString();
    if (extraInfo.empty()) {
        HILOGW("read extraInfo failed!");
        return ERR_NULL_OBJECT;
    }
    nlohmann::json extraInfoJson = nlohmann::json::parse(extraInfo, nullptr, false);
    if (!extraInfoJson.is_discarded()) {
        uint32_t accessToken = extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN];
        callerInfo.accessToken = accessToken;
        HILOGD("parse extra info, accessToken = %u", accessToken);
    }
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGW("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = StartAbilityByCallFromRemote(*want, connect, callerInfo, accountInfo);
    BehaviorEventParam eventParam = { EventCallingType::REMOTE, BehaviorEvent::START_REMOTE_ABILITY_BYCALL, result,
        want->GetElement().GetBundleName(), want->GetElement().GetAbilityName(), callerInfo.uid };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::ReleaseAbilityFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }

    sptr<IRemoteObject> connect = data.ReadRemoteObject();
    shared_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOGE("ReleaseAbilityFromRemoteInner receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    CallerInfo callerInfo;
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    std::string extraInfo;
    PARCEL_READ_HELPER(data, String, extraInfo);
    int32_t result = ReleaseAbilityFromRemote(connect, *element, callerInfo);
    BehaviorEventParam eventParam = { EventCallingType::REMOTE, BehaviorEvent::RELEASE_REMOTE_ABILITY, result,
        element->GetBundleName(), element->GetAbilityName() };
    DmsHiSysEventReport::ReportBehaviorEvent(eventParam);
    HILOGI("result %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::RegisterDistributedComponentListenerInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    int32_t result = RegisterDistributedComponentListener(callback);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}

int32_t DistributedSchedStub::GetDistributedComponentListInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }
    std::vector<std::string> distributedComponents;
    int32_t result = GetDistributedComponentList(distributedComponents);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    PARCEL_WRITE_HELPER(reply, StringVector, distributedComponents);
    return ERR_NONE;
}

int32_t DistributedSchedStub::StartRemoteFreeInstallInner(MessageParcel& data, MessageParcel& reply)
{
    if (!DistributedSchedPermission::GetInstance().IsFoundationCall()) {
        return DMS_PERMISSION_DENIED;
    }
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGE("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t callerUid = 0;
    int32_t requestCode = 0;
    uint32_t accessToken = 0;
    PARCEL_READ_HELPER(data, Int32, callerUid);
    PARCEL_READ_HELPER(data, Int32, requestCode);
    PARCEL_READ_HELPER(data, Uint32, accessToken);
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOGE("read callback failed!");
        return ERR_NULL_OBJECT;
    }
    if (DistributedSchedPermission::GetInstance().CheckPermission(accessToken,
        PERMISSION_DISTRIBUTED_DATASYNC) != ERR_OK) {
        HILOGE("check data_sync permission failed!");
        return DMS_PERMISSION_DENIED;
    }
    int32_t result = StartRemoteFreeInstall(*want, callerUid, requestCode, accessToken, callback);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
    return ERR_NONE;
}

int32_t DistributedSchedStub::StartFreeInstallFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }
    shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOGE("want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    int64_t begin = GetTickCount();
    CallerInfo callerInfo = {.accessToken = 0};
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    AccountInfo accountInfo = {};
    int64_t taskId = 0;

    PARCEL_READ_HELPER(data, Int32, callerInfo.uid);
    PARCEL_READ_HELPER(data, String, callerInfo.sourceDeviceId);
    accountInfo.accountType = data.ReadInt32();
    PARCEL_READ_HELPER(data, StringVector, &accountInfo.groupIdList);
    callerInfo.callerAppId = data.ReadString();
    PARCEL_READ_HELPER(data, Int64, taskId);
    shared_ptr<AAFwk::Want> cmpWant(data.ReadParcelable<AAFwk::Want>());
    std::string extraInfo = data.ReadString();
    if (extraInfo.empty()) {
        HILOGD("extra info is empty!");
    }
    nlohmann::json extraInfoJson = nlohmann::json::parse(extraInfo, nullptr, false);
    int32_t requestCode = DEFAULT_REQUEST_CODE;
    if (!extraInfoJson.is_discarded()) {
        uint32_t accessToken = extraInfoJson[EXTRO_INFO_JSON_KEY_ACCESS_TOKEN];
        callerInfo.accessToken = accessToken;
        HILOGD("parse extra info, accessTokenID = %u", accessToken);
        if (extraInfoJson.contains(EXTRO_INFO_JSON_KEY_REQUEST_CODE)) {
            requestCode = extraInfoJson[EXTRO_INFO_JSON_KEY_REQUEST_CODE];
            HILOGD("parse extra info, requestCode = %d", requestCode);
        }
    }

    FreeInstallInfo info = {
        .want = *want,
        .callerInfo = callerInfo,
        .accountInfo = accountInfo,
        .requestCode = requestCode
    };
    int32_t result = StartFreeInstallFromRemote(info, taskId);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_HELPER(reply, Int32, result);
    int64_t end = GetTickCount();
    PARCEL_WRITE_HELPER(reply, Int64, end - begin);
    return ERR_NONE;
}

int32_t DistributedSchedStub::NotifyCompleteFreeInstallFromRemoteInner(MessageParcel& data, MessageParcel& reply)
{
    if (!CheckCallingUid()) {
        HILOGW("request DENIED!");
        return DMS_PERMISSION_DENIED;
    }
    int64_t taskId = 0;
    int32_t resultCode = 0;
    PARCEL_READ_HELPER(data, Int64, taskId);
    PARCEL_READ_HELPER(data, Int32, resultCode);
    int32_t result = NotifyCompleteFreeInstallFromRemote(taskId, resultCode);
    HILOGI("result = %{public}d", result);
    PARCEL_WRITE_REPLY_NOERROR(reply, Int32, result);
}
} // namespace DistributedSchedule
} // namespace OHOS
