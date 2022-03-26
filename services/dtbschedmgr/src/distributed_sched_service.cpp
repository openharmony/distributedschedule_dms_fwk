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

#include "distributed_sched_service.h"

#include <cinttypes>
#include <unistd.h>

#include "ability_connection_wrapper_stub.h"
#include "ability_manager_client.h"
#include "adapter/dnetwork_adapter.h"
#include "bundle/bundle_manager_internal.h"
#include "connect_death_recipient.h"
#include "datetime_ex.h"
#include "distributed_sched_adapter.h"
#include "distributed_sched_dumper.h"
#include "distributed_sched_permission.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "element_name.h"
#include "file_ex.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
#include "mission/distributed_mission_info.h"
#include "mission/distributed_sched_mission_manager.h"
#endif
#include "os_account_manager.h"
#include "parcel_helper.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AAFwk;
using namespace AccountSA;
using namespace AppExecFwk;

namespace {
const std::string TAG = "DistributedSchedService";
const std::u16string CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.abilityshell.DistributedConnection";
const std::u16string ABILITY_MANAGER_SERVICE_TOKEN = u"ohos.aafwk.AbilityManager";
constexpr int32_t ABILITY_MANAGER_CONTINUE_ABILITY = 1104;
constexpr int32_t ABILITY_MANAGER_NOTIFY_CONTINUATION_RESULT = 1102;
constexpr int32_t ABILITY_MANAGER_CLEAN_MISSION = 45;
constexpr int32_t BIND_CONNECT_RETRY_TIMES = 3;
constexpr int32_t BIND_CONNECT_TIMEOUT = 500; // 500ms
constexpr int32_t MAX_DISTRIBUTED_CONNECT_NUM = 600;
constexpr int32_t SYSTEM_UID = 1000;
constexpr int32_t INVALID_CALLER_UID = -1;
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedService);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&DistributedSchedService::GetInstance());

DistributedSchedService::DistributedSchedService() : SystemAbility(DISTRIBUTED_SCHED_SA_ID, true)
{
}

void DistributedSchedService::OnStart()
{
    if (!Init()) {
        HILOGE("failed to init DistributedSchedService");
        return;
    }
    FuncContinuationCallback continuationCallback = [this] (int32_t missionId) {
        HILOGW("continuationCallback timeout.");
        NotifyContinuationCallbackResult(missionId, CONTINUE_ABILITY_TIMEOUT_ERR);
    };
    dschedContinuation_ = std::make_shared<DSchedContinuation>();
    dschedContinuation_->Init(continuationCallback);
    HILOGI("OnStart start service success.");
}

bool DistributedSchedService::Init()
{
    HILOGD("Init ready to init.");
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().Init();
#endif
    bool ret = Publish(this);
    if (!ret) {
        HILOGE("Init Publish failed!");
        return false;
    }

    if (!DtbschedmgrDeviceInfoStorage::GetInstance().Init()) {
        HILOGE("Init DtbschedmgrDeviceInfoStorage init failed.");
    }
    HILOGD("Init init success.");
    DistributedSchedAdapter::GetInstance().Init();
    DnetworkAdapter::GetInstance()->Init();
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().InitDataStorage();
#endif
    connectDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new ConnectDeathRecipient());
    callerDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new CallerDeathRecipient());
    return true;
}

void DistributedSchedService::OnStop()
{
    HILOGD("OnStop ready to stop service.");
}

int32_t DistributedSchedService::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode, uint32_t accessToken)
{
    std::string localDeviceId;
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (IPCSkeleton::GetCallingUid() != SYSTEM_UID) {
        HILOGE("check uid failed");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("get remoteDms failed");
        return INVALID_PARAMETERS_ERR;
    }
    AppExecFwk::AbilityInfo abilityInfo;
    CallerInfo callerInfo;
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.uid = callerUid;
    callerInfo.accessToken = accessToken;
    AccountInfo accountInfo;
    HILOGI("[PerformanceTest] StartRemoteAbility transact begin");
    int32_t result = remoteDms->StartAbilityFromRemote(want, abilityInfo, requestCode, callerInfo, accountInfo);
    HILOGI("[PerformanceTest] StartRemoteAbility transact end");
    return result;
}

int32_t DistributedSchedService::StartAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    std::string localDeviceId;
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, deviceId, callerInfo.sourceDeviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    ErrCode err = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, abilityInfo, deviceId);
    if (err != ERR_OK) {
        HILOGE("CheckDPermission denied!!");
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("connect ability server failed %{public}d", err);
        return err;
    }
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode, ids[0]);
    if (err != ERR_OK) {
        HILOGE("StartAbility failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::ContinueToAbilityManager(const std::string& deviceId, int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MANAGER_SERVICE_TOKEN)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(deviceId)) {
        HILOGE("deviceId write failed.");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOGE("missionId write failed.");
        return INVALID_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> abilityManager = GetAbilityManagerProxy();
    if (abilityManager == nullptr) {
        HILOGE("get ability manager failed");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = abilityManager->SendRequest(ABILITY_MANAGER_CONTINUE_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t DistributedSchedService::ContinueLocalMission(const std::string& dstDeviceId, int32_t missionId,
    const sptr<IRemoteObject>& callback)
{
    if (dschedContinuation_ == nullptr) {
        HILOGE("continuation object null!");
        return INVALID_PARAMETERS_ERR;
    }
    if (dschedContinuation_->IsInContinuationProgress(missionId)) {
        HILOGE("ContinueLocalMission already in progress!");
        return INVALID_PARAMETERS_ERR;
    }
    dschedContinuation_->PushCallback(missionId, callback);
    int32_t result = ContinueToAbilityManager(dstDeviceId, missionId);
    HILOGI("ContinueLocalMission result: %{public}d!", result);
    return result;
}

int32_t DistributedSchedService::ContinueRemoteMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    sptr<IDistributedSched> remoteDms = GetRemoteDms(srcDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("get remote dms null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    int32_t result = remoteDms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    HILOGI("ContinueRemoteMission result: %{public}d!", result);
    return result;
}

int32_t DistributedSchedService::ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        HILOGE("srcDeviceId or dstDeviceId or callback is null!");
        return INVALID_PARAMETERS_ERR;
    }
    std::string localDevId;
    if (!GetLocalDeviceId(localDevId)) {
        HILOGE("get local deviceId failed!");
        return INVALID_PARAMETERS_ERR;
    }

    if (srcDeviceId == localDevId) {
        return ContinueLocalMission(dstDeviceId, missionId, callback);
    } else if (dstDeviceId == localDevId) {
        return ContinueRemoteMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    } else {
        HILOGE("source or target device must be local!");
        return INVALID_PARAMETERS_ERR;
    }
}

int32_t DistributedSchedService::StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId,
    int32_t callerUid, int32_t status, uint32_t accessToken)
{
    HILOGD("[PerformanceTest] StartContinuation begin");
    if (status != ERR_OK) {
        HILOGE("continuation has been rejected, status: %{public}d", status);
        NotifyContinuationCallbackResult(missionId, status);
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    auto flags = want.GetFlags();
    if ((flags & AAFwk::Want::FLAG_ABILITY_CONTINUATION) == 0) {
        HILOGE("StartContinuation want continuation flags invalid!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    HILOGD("StartContinuation: devId = %{private}s, bundleName = %{private}s, abilityName = %{private}s",
        want.GetElement().GetDeviceID().c_str(),
        want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());

    int32_t uid = IPCSkeleton::GetCallingUid();
    if (uid != SYSTEM_UID) {
        HILOGE("StartContinuation not allowed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    std::string devId;
    if (!GetLocalDeviceId(devId)) {
        HILOGE("StartContinuation get local deviceId failed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    if (dschedContinuation_ == nullptr) {
        HILOGE("StartContinuation continuation object null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    if (!dschedContinuation_->IsInContinuationProgress(missionId)) {
        dschedContinuation_->SetTimeOut(missionId);
    }

    int32_t sessionId = missionId;
    AAFwk::Want newWant = want;
    newWant.SetParam("sessionId", sessionId);
    newWant.SetParam("deviceId", devId);
    int32_t result = ERR_OK;
    result = StartRemoteAbility(newWant, callerUid, 0, accessToken);
    if (result != ERR_OK) {
        HILOGE("continue ability failed, errorCode = %{public}d", result);
        return result;
    }

    HILOGD("[PerformanceTest] StartContinuation end");
    return result;
}

void DistributedSchedService::NotifyCompleteContinuation(const std::u16string& devId,
    int32_t sessionId, bool isSuccess)
{
    if (!isSuccess) {
        HILOGE("NotifyCompleteContinuation failed!");
    }
    if (sessionId <= 0) {
        HILOGE("NotifyCompleteContinuation sessionId invalid!");
        return;
    }
    std::string deviceId = Str16ToStr8(devId);
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("NotifyCompleteContinuation get remote dms null!");
        return;
    }
    remoteDms->NotifyContinuationResultFromRemote(sessionId, isSuccess);
}

int32_t DistributedSchedService::NotifyContinuationResultFromRemote(int32_t sessionId, bool isSuccess)
{
    if (sessionId <= 0) {
        HILOGE("NotifyContinuationResultFromRemote sessionId:%{public}d invalid!", sessionId);
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    int32_t missionId = sessionId;
    NotifyContinuationCallbackResult(missionId, isSuccess ? 0 : NOTIFYCOMPLETECONTINUATION_FAILED);
    return ERR_OK;
}

sptr<IRemoteObject> DistributedSchedService::GetAbilityManagerProxy()
{
    HILOGI("%{public}s begin.", __func__);
    auto sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        HILOGE("GetAbilityManagerProxy sm is nullptr");
        return nullptr;
    }

    abilityManagerProxy_ = sm->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (abilityManagerProxy_ == nullptr) {
        HILOGE("failed to get ability manager service");
        return nullptr;
    }
    return abilityManagerProxy_;
}

int32_t DistributedSchedService::NotifyResultToAbilityManager(int32_t missionId, int32_t isSuccess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ABILITY_MANAGER_SERVICE_TOKEN)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOGE("NotifyResultToAbilityManager missionId write failed.");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(isSuccess)) {
        HILOGE("NotifyResultToAbilityManager result write failed.");
        return INVALID_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> abilityManager = GetAbilityManagerProxy();
    if (abilityManager == nullptr) {
        HILOGE("get ability manager failed");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = abilityManager->SendRequest(ABILITY_MANAGER_NOTIFY_CONTINUATION_RESULT,
        data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("NotifyResultToAbilityManager Send request error: %{public}d", error);
        return INVALID_PARAMETERS_ERR;
    }
    return reply.ReadInt32();
}

int32_t DistributedSchedService::CleanMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ABILITY_MANAGER_SERVICE_TOKEN)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        HILOGE("CleanMission WriteInt32 fail.");
        return INVALID_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> abilityManager = GetAbilityManagerProxy();
    if (abilityManager == nullptr) {
        HILOGE("get ability manager failed");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = abilityManager->SendRequest(ABILITY_MANAGER_CLEAN_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("CleanMission error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void DistributedSchedService::NotifyContinuationCallbackResult(int32_t missionId, int32_t isSuccess)
{
    HILOGD("Continuation result is: %{public}d", isSuccess);

    if (dschedContinuation_ == nullptr) {
        HILOGE("continuation object null!");
        return;
    }

    int32_t result = 0;
    if (dschedContinuation_->IsInContinuationProgress(missionId)) {
        if (!isSuccess) {
            result = CleanMission(missionId);
            HILOGD("clean mission result:%{public}d", result);
        }
        result = dschedContinuation_->NotifyMissionCenterResult(missionId, isSuccess);
    } else {
        result = NotifyResultToAbilityManager(missionId, isSuccess);
        dschedContinuation_->RemoveTimeOut(missionId);
    }
    HILOGD("NotifyContinuationCallbackResult result:%{public}d", result);
}

void DistributedSchedService::RemoteConnectAbilityMappingLocked(const sptr<IRemoteObject>& connect,
    const std::string& localDeviceId, const std::string& remoteDeviceId, const AppExecFwk::ElementName& element,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
{
    if (connect == nullptr) {
        return;
    }
    auto itConnect = distributedConnectAbilityMap_.find(connect);
    if (itConnect == distributedConnectAbilityMap_.end()) {
        // add uid's connect number
        uint32_t number = ++trackingUidMap_[callerInfo.uid];
        HILOGD("uid %d has %u connection(s), targetComponent:%d", callerInfo.uid, number, targetComponent);
        // new connect, add death recipient
        connect->AddDeathRecipient(connectDeathRecipient_);
    }
    auto& sessionsList = distributedConnectAbilityMap_[connect];
    for (auto& session : sessionsList) {
        if (remoteDeviceId == session.GetDestinationDeviceId()) {
            session.AddElement(element);
            // already added session for remote device
            return;
        }
    }
    // connect to another remote device, add a new session to list
    auto& session = sessionsList.emplace_back(localDeviceId, remoteDeviceId, callerInfo, targetComponent);
    session.AddElement(element);
    HILOGD("add connection success");
}

int32_t DistributedSchedService::CheckDistributedConnectLocked(const CallerInfo& callerInfo) const
{
    if (callerInfo.uid < 0) {
        HILOGE("uid %d is invalid", callerInfo.uid);
        return BIND_ABILITY_UID_INVALID_ERR;
    }

    if (callerInfo.uid == SYSTEM_UID) {
        return ERR_OK;
    }
    auto it = trackingUidMap_.find(callerInfo.uid);
    if (it != trackingUidMap_.end() && it->second >= MAX_DISTRIBUTED_CONNECT_NUM) {
        HILOGE("uid %{public}d connected too much abilities, it maybe leak", callerInfo.uid);
        return BIND_ABILITY_LEAK_ERR;
    }
    return ERR_OK;
}

void DistributedSchedService::DecreaseConnectLocked(int32_t uid)
{
    if (uid < 0) {
        HILOGE("DecreaseConnectLocked invalid uid %{public}d", uid);
        return;
    }
    auto it = trackingUidMap_.find(uid);
    if (it != trackingUidMap_.end()) {
        auto& conns = it->second;
        if (conns > 0) {
            conns--;
        }
        if (conns == 0) {
            HILOGD("DecreaseConnectLocked uid %{public}d connection(s) is 0", uid);
            trackingUidMap_.erase(it);
        }
    }
}

int32_t DistributedSchedService::GetUidLocked(const std::list<ConnectAbilitySession>& sessionsList)
{
    if (!sessionsList.empty()) {
        return sessionsList.front().GetCallerInfo().uid;
    }
    return INVALID_CALLER_UID;
}

int32_t DistributedSchedService::ConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    std::string localDeviceId;
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, remoteDeviceId)) {
        HILOGE("ConnectRemoteAbility check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (IPCSkeleton::GetCallingUid() != SYSTEM_UID) {
        HILOGE("ConnectRemoteAbility check uid failed");
        return INVALID_PARAMETERS_ERR;
    }
    CallerInfo callerInfo = { callerUid, callerPid, CALLER_TYPE_HARMONY, localDeviceId };
    callerInfo.accessToken = accessToken;
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        int32_t checkResult = CheckDistributedConnectLocked(callerInfo);
        if (checkResult != ERR_OK) {
            return checkResult;
        }
    }

    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("ConnectRemoteAbility GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = DistributedSchedAdapter::GetInstance().GetBundleNameListFromBms(
        callerInfo.uid, callerInfo.bundleNames);
    if (ret != ERR_OK) {
        HILOGE("ConnectRemoteAbility GetBundleNameListFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }

    HILOGD("[PerformanceTest] ConnectRemoteAbility begin");
    int32_t result = TryConnectRemoteAbility(want, connect, callerInfo);
    if (result != ERR_OK) {
        HILOGE("ConnectRemoteAbility result is %{public}d", result);
    }
    HILOGD("[PerformanceTest] ConnectRemoteAbility end");
    return result;
}

int32_t DistributedSchedService::TryConnectRemoteAbility(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo)
{
    AppExecFwk::AbilityInfo abilityInfo;
    AccountInfo accountInfo;
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr || connect == nullptr) {
        HILOGE("TryConnectRemoteAbility invalid parameters");
        return INVALID_PARAMETERS_ERR;
    }

    int32_t retryTimes = BIND_CONNECT_RETRY_TIMES;
    int32_t result = REMOTE_DEVICE_BIND_ABILITY_ERR;
    while (retryTimes--) {
        int64_t start = GetTickCount();
        HILOGD("[PerformanceTest] ConnectRemoteAbility begin");
        result = remoteDms->ConnectAbilityFromRemote(want, abilityInfo, connect, callerInfo, accountInfo);
        HILOGD("[PerformanceTest] ConnectRemoteAbility end");
        if (result == ERR_OK) {
            std::lock_guard<std::mutex> autoLock(distributedLock_);
            RemoteConnectAbilityMappingLocked(connect, callerInfo.sourceDeviceId, remoteDeviceId,
                want.GetElement(), callerInfo, TargetComponent::HARMONY_COMPONENT);
            break;
        }
        if (result == INVALID_REMOTE_PARAMETERS_ERR || result == REMOTE_DEVICE_BIND_ABILITY_ERR) {
            break;
        }
        int64_t elapsedTime = GetTickCount() - start;
        if (elapsedTime > BIND_CONNECT_TIMEOUT) {
            HILOGW("ConnectRemoteAbility timeout, elapsedTime is %{public}" PRId64 " ms", elapsedTime);
            break;
        }
    }
    return result;
}

void DistributedSchedService::ProcessCallerDied(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("ProcessCallerDied connect is null");
        return;
    }
    sptr<IRemoteObject> callbackWrapper = connect;
    AppExecFwk::ElementName element;
    {
        std::lock_guard<std::mutex> autoLock(calleeLock_);
        auto itConnect = calleeMap_.find(connect);
        if (itConnect != calleeMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
            element = itConnect->second.element;
            calleeMap_.erase(itConnect);
        } else {
            HILOGW("ProcessCallerDied connect not found");
        }
    }
    int32_t result = DistributedSchedAdapter::GetInstance().ReleaseAbility(callbackWrapper, element);
    if (result != ERR_OK) {
        HILOGW("ProcessCallerDied failed, error: %{public}d", result);
    }
}

void DistributedSchedService::ProcessCalleeDied(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("ProcessCalleeDied connect is null");
        return;
    }
    std::lock_guard<std::mutex> autoLock(calleeLock_);
    auto itConnect = calleeMap_.find(connect);
    if (itConnect != calleeMap_.end()) {
        calleeMap_.erase(itConnect);
    } else {
        HILOGW("ProcessCalleeDied connect not found");
    }
}

int32_t DistributedSchedService::TryStartRemoteAbilityByCall(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo)
{
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    HILOGD("[PerformanceTest] TryStartRemoteAbilityByCall get remote DMS");
    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("TryStartRemoteAbilityByCall get remote DMS failed, remoteDeviceId : %{public}s",
            DnetworkAdapter::AnonymizeDeviceId(remoteDeviceId).c_str());
        return INVALID_PARAMETERS_ERR;
    }
    HILOGD("[PerformanceTest] TryStartRemoteAbilityByCall RPC begin");
    AccountInfo accountInfo;
    int result = remoteDms->StartAbilityByCallFromRemote(want, connect, callerInfo, accountInfo);
    HILOGD("[PerformanceTest] TryStartRemoteAbilityByCall RPC end");
    if (result != ERR_OK) {
        HILOGE("TryStartRemoteAbilityByCall failed, result : %{public}d", result);
    }
    return result;
}

int32_t DistributedSchedService::StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, int32_t callerUid, int32_t callerPid, uint32_t accessToken)
{
    if (connect == nullptr) {
        HILOGE("StartRemoteAbilityByCall connect is null");
        return INVALID_PARAMETERS_ERR;
    }
    std::string localDeviceId;
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, remoteDeviceId)) {
        HILOGE("StartRemoteAbilityByCall check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (IPCSkeleton::GetCallingUid() != SYSTEM_UID) {
        HILOGE("StartRemoteAbilityByCall check system uid failed");
        return INVALID_PARAMETERS_ERR;
    }
    CallerInfo callerInfo;
    callerInfo = { callerUid, callerPid };
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.accessToken = accessToken;
    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("StartRemoteAbilityByCall GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t ret = TryStartRemoteAbilityByCall(want, connect, callerInfo);
    if (ret != ERR_OK) {
        HILOGE("StartRemoteAbilityByCall result is %{public}d", ret);
    }
    return ret;
}

int32_t DistributedSchedService::ReleaseRemoteAbility(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element)
{
    if (connect == nullptr) {
        HILOGE("ReleaseRemoteAbility connect is null");
        return INVALID_PARAMETERS_ERR;
    }
    if (element.GetDeviceID().empty()) {
        HILOGE("ReleaseRemoteAbility remote deviceId empty");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(element.GetDeviceID());
    if (remoteDms == nullptr) {
        HILOGE("ReleaseRemoteAbility get remote dms failed, devId : %{public}s",
            DnetworkAdapter::AnonymizeDeviceId(element.GetDeviceID()).c_str());
        return INVALID_PARAMETERS_ERR;
    }
    CallerInfo callerInfo;
    if (!GetLocalDeviceId(callerInfo.sourceDeviceId)) {
        HILOGE("ReleaseRemoteAbility get local deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t result = remoteDms->ReleaseAbilityFromRemote(connect, element, callerInfo);
    if (result != ERR_OK) {
        HILOGE("ReleaseRemoteAbility result is %{public}d", result);
    }
    return result;
}

int32_t DistributedSchedService::StartAbilityByCallFromRemote(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    HILOGD("[PerformanceTest] DistributedSchedService StartAbilityByCallFromRemote begin");
    if (connect == nullptr) {
        HILOGE("StartAbilityByCallFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    std::string localDeviceId;
    std::string destinationDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, destinationDeviceId, callerInfo.sourceDeviceId)) {
        HILOGE("StartAbilityByCallFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    int32_t result = permissionInstance.CheckGetCallerPermission(want, callerInfo, accountInfo, localDeviceId);
    if (result != ERR_OK) {
        HILOGE("StartAbilityByCallFromRemote CheckDPermission denied!!");
        return result;
    }
    sptr<IRemoteObject> callbackWrapper;
    {
        std::lock_guard<std::mutex> autoLock(calleeLock_);
        auto itConnect = calleeMap_.find(connect);
        if (itConnect != calleeMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
        } else {
            callbackWrapper = new AbilityConnectionWrapperStub(connect, localDeviceId);
        }
    }
    int32_t errCode = DistributedSchedAdapter::GetInstance().StartAbilityByCall(want, callbackWrapper, this);
    HILOGD("[PerformanceTest] StartAbilityByCallFromRemote end");
    if (errCode == ERR_OK) {
        {
            std::lock_guard<std::mutex> autoLock(calleeLock_);
            ConnectInfo connectInfo {callerInfo, callbackWrapper, want.GetElement()};
            calleeMap_.emplace(connect, connectInfo);
        }
        connect->AddDeathRecipient(callerDeathRecipient_);
    }
    return errCode;
}

int32_t DistributedSchedService::ReleaseAbilityFromRemote(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element, const CallerInfo& callerInfo)
{
    if (connect == nullptr) {
        HILOGE("ReleaseAbilityFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    HILOGD("[PerformanceTest] ReleaseAbilityFromRemote begin");
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId.empty() ||
        callerInfo.sourceDeviceId.empty() || localDeviceId == callerInfo.sourceDeviceId) {
        HILOGE("ReleaseAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> callbackWrapper;
    {
        std::lock_guard<std::mutex> autoLock(calleeLock_);
        auto itConnect = calleeMap_.find(connect);
        if (itConnect == calleeMap_.end()) {
            HILOGE("ReleaseAbilityFromRemote callee not found");
            return INVALID_REMOTE_PARAMETERS_ERR;
        }
        callbackWrapper = itConnect->second.callbackWrapper;
        calleeMap_.erase(itConnect);
        connect->RemoveDeathRecipient(callerDeathRecipient_);
    }
    int32_t result = DistributedSchedAdapter::GetInstance().ReleaseAbility(callbackWrapper, element);
    HILOGD("[PerformanceTest] ReleaseAbilityFromRemote end");
    if (result != ERR_OK) {
        HILOGE("ReleaseAbilityFromRemote failed, error: %{public}d", result);
    }
    return result;
}

sptr<IDistributedSched> DistributedSchedService::GetRemoteDms(const std::string& remoteDeviceId)
{
    if (remoteDeviceId.empty()) {
        HILOGE("GetRemoteDms remoteDeviceId is empty");
        return nullptr;
    }
    HILOGD("GetRemoteDms connect deviceid is %s", remoteDeviceId.c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("GetRemoteDms failed to connect to systemAbilityMgr!");
        return nullptr;
    }
    HILOGD("[PerformanceTest] GetRemoteDms begin");
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID, remoteDeviceId);
    HILOGD("[PerformanceTest] GetRemoteDms end");
    if (object == nullptr) {
        HILOGE("GetRemoteDms failed to get remote DistributedSched %{private}s", remoteDeviceId.c_str());
        return nullptr;
    }
    return iface_cast<IDistributedSched>(object);
}

bool DistributedSchedService::GetLocalDeviceId(std::string& localDeviceId)
{
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        HILOGE("GetLocalDeviceId failed");
        return false;
    }
    return true;
}

bool DistributedSchedService::CheckDeviceId(const std::string& localDeviceId, const std::string& remoteDeviceId)
{
    // remoteDeviceId must not same with localDeviceId
    if (localDeviceId.empty() || remoteDeviceId.empty() || localDeviceId == remoteDeviceId) {
        HILOGE("check deviceId failed");
        return false;
    }
    return true;
}

bool DistributedSchedService::CheckDeviceIdFromRemote(const std::string& localDeviceId,
    const std::string& destinationDeviceId, const std::string& sourceDeviceId)
{
    if (localDeviceId.empty() || destinationDeviceId.empty() || sourceDeviceId.empty()) {
        HILOGE("CheckDeviceIdFromRemote failed");
        return false;
    }
    // destinationDeviceId set by remote must be same with localDeviceId
    if (localDeviceId != destinationDeviceId) {
        HILOGE("destinationDeviceId is not same with localDeviceId");
        return false;
    }
    HILOGD("CheckDeviceIdFromRemote sourceDeviceId %s", sourceDeviceId.c_str());
    HILOGD("CheckDeviceIdFromRemote localDeviceId %s", localDeviceId.c_str());
    HILOGD("CheckDeviceIdFromRemote destinationDeviceId %s", destinationDeviceId.c_str());

    if (sourceDeviceId == destinationDeviceId || sourceDeviceId == localDeviceId) {
        HILOGE("destinationDeviceId is different with localDeviceId and destinationDeviceId");
        return false;
    }
    return true;
}

int32_t DistributedSchedService::ConnectAbilityFromRemote(const OHOS::AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo)
{
    HILOGD("[PerformanceTest] DistributedSchedService ConnectAbilityFromRemote begin");
    if (connect == nullptr) {
        HILOGE("ConnectAbilityFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    HILOGD("ConnectAbilityFromRemote uid is %{public}d, pid is %{public}d, AccessTokenID is %{public}u",
        callerInfo.uid, callerInfo.pid, callerInfo.accessToken);
    std::string localDeviceId;
    std::string destinationDeviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, destinationDeviceId, callerInfo.sourceDeviceId)) {
        HILOGE("ConnectAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    int32_t result = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, abilityInfo, localDeviceId);
    if (result != ERR_OK) {
        HILOGE("ConnectAbilityFromRemote CheckDPermission denied!!");
        return result;
    }

    HILOGD("ConnectAbilityFromRemote callerType is %{public}d", callerInfo.callerType);
    sptr<IRemoteObject> callbackWrapper = connect;
    std::map<sptr<IRemoteObject>, ConnectInfo>::iterator itConnect;
    if (callerInfo.callerType == CALLER_TYPE_HARMONY) {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        itConnect = connectAbilityMap_.find(connect);
        if (itConnect != connectAbilityMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
        } else {
            callbackWrapper = new AbilityConnectionWrapperStub(connect);
        }
    }
    int32_t errCode = DistributedSchedAdapter::GetInstance().ConnectAbility(want, callbackWrapper, this);
    HILOGD("[PerformanceTest] ConnectAbilityFromRemote end");
    if (errCode == ERR_OK) {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        if (itConnect == connectAbilityMap_.end()) {
            ConnectInfo connectInfo {callerInfo, callbackWrapper};
            connectAbilityMap_.emplace(connect, connectInfo);
        }
    }
    return errCode;
}

int32_t DistributedSchedService::DisconnectEachRemoteAbilityLocked(const std::string& localDeviceId,
    const std::string& remoteDeviceId, const sptr<IRemoteObject>& connect)
{
    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("DisconnectRemoteAbility get remote dms failed");
        return INVALID_PARAMETERS_ERR;
    }
    int32_t result = remoteDms->DisconnectAbilityFromRemote(connect, IPCSkeleton::GetCallingUid(), localDeviceId);
    if (result != ERR_OK) {
        HILOGE("DisconnectEachRemoteAbilityLocked result is %{public}d", result);
    }
    return result;
}

int32_t DistributedSchedService::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid,
    uint32_t accessToken)
{
    if (connect == nullptr) {
        HILOGE("DisconnectRemoteAbility connect is null");
        return INVALID_PARAMETERS_ERR;
    }

    if (IPCSkeleton::GetCallingUid() != SYSTEM_UID) {
        HILOGE("DisconnectRemoteAbility check uid failed");
        return INVALID_PARAMETERS_ERR;
    }
    std::list<ConnectAbilitySession> sessionsList;
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        auto it = distributedConnectAbilityMap_.find(connect);
        if (it != distributedConnectAbilityMap_.end()) {
            sessionsList = it->second;
            int32_t uid = GetUidLocked(sessionsList);
            // also decrease number when erase connect
            DecreaseConnectLocked(uid);
            connect->RemoveDeathRecipient(connectDeathRecipient_);
            distributedConnectAbilityMap_.erase(it);
            HILOGI("remove connection success");
        }
    }
    if (!sessionsList.empty()) {
        for (const auto& session : sessionsList) {
            if (session.GetTargetComponent() == TargetComponent::HARMONY_COMPONENT) {
                DisconnectEachRemoteAbilityLocked(session.GetSourceDeviceId(),
                    session.GetDestinationDeviceId(), connect);
            } else {
                HILOGW("DisconnectRemoteAbility non-harmony component");
            }
        }
        return ERR_OK;
    }
    return NO_CONNECT_CALLBACK_ERR;
}

int32_t DistributedSchedService::DisconnectAbilityFromRemote(const sptr<IRemoteObject>& connect,
    int32_t uid, const std::string& sourceDeviceId)
{
    if (connect == nullptr) {
        HILOGE("DisconnectAbilityFromRemote connect is null");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    HILOGD("[PerformanceTest] DisconnectAbilityFromRemote begin");
    std::string localDeviceId;
    AppExecFwk::AbilityInfo abilityInfo;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId.empty() ||
        sourceDeviceId.empty() || localDeviceId == sourceDeviceId) {
        HILOGE("DisconnectAbilityFromRemote check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    sptr<IRemoteObject> callbackWrapper = connect;
    {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        auto itConnect = connectAbilityMap_.find(connect);
        if (itConnect != connectAbilityMap_.end()) {
            callbackWrapper = itConnect->second.callbackWrapper;
            connectAbilityMap_.erase(itConnect);
        } else {
            if (!IPCSkeleton::IsLocalCalling()) {
                HILOGE("DisconnectAbilityFromRemote connect not found");
                return INVALID_REMOTE_PARAMETERS_ERR;
            }
        }
    }
    int32_t result = DistributedSchedAdapter::GetInstance().DisconnectAbility(callbackWrapper);
    HILOGD("[PerformanceTest] DisconnectAbilityFromRemote end");
    return result;
}

int32_t DistributedSchedService::NotifyProcessDiedFromRemote(const CallerInfo& callerInfo)
{
    HILOGI("NotifyProcessDiedFromRemote called");
    int32_t errCode = ERR_OK;
    {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        for (auto iter = connectAbilityMap_.begin(); iter != connectAbilityMap_.end();) {
            ConnectInfo& connectInfo = iter->second;
            if (callerInfo.sourceDeviceId == connectInfo.callerInfo.sourceDeviceId
                && callerInfo.uid == connectInfo.callerInfo.uid
                && callerInfo.pid == connectInfo.callerInfo.pid
                && callerInfo.callerType == connectInfo.callerInfo.callerType) {
                HILOGI("NotifyProcessDiedFromRemote erase connection success");
                int32_t ret = DistributedSchedAdapter::GetInstance().DisconnectAbility(connectInfo.callbackWrapper);
                if (ret != ERR_OK) {
                    errCode = ret;
                }
                connectAbilityMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }
    return errCode;
}

void DistributedSchedService::ProcessDeviceOffline(const std::string& deviceId)
{
    HILOGI("ProcessDeviceOffline called");
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("ProcessDeviceOffline check deviceId failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        for (auto iter = distributedConnectAbilityMap_.begin(); iter != distributedConnectAbilityMap_.end();) {
            std::list<ConnectAbilitySession>& sessionsList = iter->second;
            int32_t uid = GetUidLocked(sessionsList);
            auto itSession = std::find_if(sessionsList.begin(), sessionsList.end(), [&deviceId](const auto& session) {
                return session.GetDestinationDeviceId() == deviceId;
            });
            if (itSession != sessionsList.end()) {
                NotifyDeviceOfflineToAppLocked(iter->first, *itSession);
                CallerInfo callerInfo = itSession->GetCallerInfo();
                sessionsList.erase(itSession);
            }

            if (sessionsList.empty()) {
                if (iter->first != nullptr) {
                    iter->first->RemoveDeathRecipient(connectDeathRecipient_);
                }
                DecreaseConnectLocked(uid);
                distributedConnectAbilityMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }

    {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        for (auto iter = connectAbilityMap_.begin(); iter != connectAbilityMap_.end();) {
            ConnectInfo& connectInfo = iter->second;
            if (deviceId == connectInfo.callerInfo.sourceDeviceId) {
                DistributedSchedAdapter::GetInstance().DisconnectAbility(connectInfo.callbackWrapper);
                connectAbilityMap_.erase(iter++);
                HILOGI("ProcessDeviceOffline erase connection success");
            } else {
                iter++;
            }
        }
    }
}

void DistributedSchedService::NotifyDeviceOfflineToAppLocked(const sptr<IRemoteObject>& connect,
    const ConnectAbilitySession& session)
{
    std::list<AppExecFwk::ElementName> elementsList = session.GetElementsList();
    for (const auto& element : elementsList) {
        int32_t errCode = NotifyApp(connect, element, DEVICE_OFFLINE_ERR);
        if (errCode != ERR_NONE) {
            HILOGW("ProcessDeviceOffline notify failed, errCode = %{public}d", errCode);
        }
    }
}

int32_t DistributedSchedService::NotifyApp(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName& element, int32_t errCode)
{
    if (connect == nullptr) {
        return OBJECT_NULL;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(CONNECTION_CALLBACK_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &element);
    PARCEL_WRITE_HELPER(data, Int32, errCode);
    MessageParcel reply;
    MessageOption option;
    return connect->SendRequest(IAbilityConnection::ON_ABILITY_DISCONNECT_DONE, data, reply, option);
}

void DistributedSchedService::ProcessConnectDied(const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        HILOGE("ProcessConnectDied connect is null");
        return;
    }

    std::list<ProcessDiedNotifyInfo> notifyList;
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        auto it = distributedConnectAbilityMap_.find(connect);
        if (it == distributedConnectAbilityMap_.end()) {
            return;
        }
        std::list<ConnectAbilitySession>& connectSessionsList = it->second;
        if (connectSessionsList.empty()) {
            return;
        }
        CallerInfo callerInfo = connectSessionsList.front().GetCallerInfo();
        std::set<std::string> processedDeviceSet;
        // to reduce the number of communications between devices, clean all the died process's connections
        for (auto iter = distributedConnectAbilityMap_.begin(); iter != distributedConnectAbilityMap_.end();) {
            std::list<ConnectAbilitySession>& sessionsList = iter->second;
            if (!sessionsList.empty() && sessionsList.front().IsSameCaller(callerInfo)) {
                for (const auto& session : sessionsList) {
                    std::string remoteDeviceId = session.GetDestinationDeviceId();
                    TargetComponent targetComponent = session.GetTargetComponent();
                    // the same session can connect different types component on the same device
                    std::string key = remoteDeviceId + std::to_string(static_cast<int32_t>(targetComponent));
                    // just notify one time for same remote device
                    auto [_, isSuccess] = processedDeviceSet.emplace(key);
                    if (isSuccess) {
                        ProcessDiedNotifyInfo notifyInfo = { remoteDeviceId, callerInfo, targetComponent };
                        notifyList.push_back(notifyInfo);
                    }
                }
                DecreaseConnectLocked(callerInfo.uid);
                if (iter->first != nullptr) {
                    iter->first->RemoveDeathRecipient(connectDeathRecipient_);
                }
                distributedConnectAbilityMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }
    NotifyProcessDiedAll(notifyList);
}

void DistributedSchedService::NotifyProcessDiedAll(const std::list<ProcessDiedNotifyInfo>& notifyList)
{
    for (auto it = notifyList.begin(); it != notifyList.end(); ++it) {
        NotifyProcessDied(it->remoteDeviceId, it->callerInfo, it->targetComponent);
    }
}

void DistributedSchedService::NotifyProcessDied(const std::string& remoteDeviceId,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
{
    if (targetComponent != TargetComponent::HARMONY_COMPONENT) {
        HILOGD("NotifyProcessDied not harmony component, no need to notify");
        return;
    }

    sptr<IDistributedSched> remoteDms = GetRemoteDms(remoteDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("NotifyProcessDied get remote dms failed");
        return;
    }
    int32_t result = remoteDms->NotifyProcessDiedFromRemote(callerInfo);
    HILOGI("NotifyProcessDied result is %{public}d", result);
}

ConnectAbilitySession::ConnectAbilitySession(const std::string& sourceDeviceId, const std::string& destinationDeviceId,
    const CallerInfo& callerInfo, TargetComponent targetComponent)
    : sourceDeviceId_(sourceDeviceId),
      destinationDeviceId_(destinationDeviceId),
      callerInfo_(callerInfo),
      targetComponent_(targetComponent)
{
}

void ConnectAbilitySession::AddElement(const AppExecFwk::ElementName& element)
{
    for (const auto& elementName : elementsList_) {
        if (elementName == element) {
            return;
        }
    }
    elementsList_.emplace_back(element);
}

bool ConnectAbilitySession::IsSameCaller(const CallerInfo& callerInfo)
{
    return (callerInfo.uid == callerInfo_.uid &&
            callerInfo.pid == callerInfo_.pid &&
            callerInfo.sourceDeviceId == callerInfo_.sourceDeviceId &&
            callerInfo.callerType == callerInfo_.callerType);
}

int32_t DistributedSchedService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::vector<std::string> argsInStr8;
    for (const auto& arg : args) {
        argsInStr8.emplace_back(Str16ToStr8(arg));
    }

    std::string result;
    DistributedSchedDumper::Dump(argsInStr8, result);

    if (!SaveStringToFd(fd, result)) {
        HILOGE("save to fd failed");
        return DMS_WRITE_FILE_FAILED_ERR;
    }
    return ERR_OK;
}

void DistributedSchedService::DumpConnectInfo(std::string& info)
{
    std::lock_guard<std::mutex> autoLock(distributedLock_);
    info += "connected remote abilities:\n";
    if (!distributedConnectAbilityMap_.empty()) {
        for (const auto& distributedConnect : distributedConnectAbilityMap_) {
            const std::list<ConnectAbilitySession> sessionsList = distributedConnect.second;
            DumpSessionsLocked(sessionsList, info);
        }
    } else {
        info += "  <none info>\n";
    }
}

void DistributedSchedService::DumpSessionsLocked(const std::list<ConnectAbilitySession>& sessionsList,
    std::string& info)
{
    for (const auto& session : sessionsList) {
        info += "  ";
        info += "SourceDeviceId: ";
        info += session.GetSourceDeviceId();
        info += ", ";
        info += "DestinationDeviceId: ";
        info += session.GetDestinationDeviceId();
        info += ", ";
        info += "CallerUid: ";
        info += std::to_string(session.GetCallerInfo().uid);
        info += ", ";
        info += "CallerPid: ";
        info += std::to_string(session.GetCallerInfo().pid);
        info += ", ";
        info += "CallerType: ";
        info += std::to_string(session.GetCallerInfo().callerType);
        DumpElementLocked(session.GetElementsList(), info);
        info += "\n";
    }
}

void DistributedSchedService::DumpElementLocked(const std::list<AppExecFwk::ElementName>& elementsList,
    std::string& info)
{
    for (const auto& element : elementsList) {
        info += ", ";
        info += "BundleName: ";
        info += element.GetBundleName();
        info += ", ";
        info += "AbilityName: ";
        info += element.GetAbilityName();
    }
}

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
int32_t DistributedSchedService::GetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<MissionInfo>& missionInfos)
{
    return DistributedSchedMissionManager::GetInstance().GetMissionInfos(deviceId, numMissions, missionInfos);
}

int32_t DistributedSchedService::NotifyMissionsChangedFromRemote(const std::vector<DstbMissionInfo>& missionInfos,
    const CallerInfo& callerInfo)
{
    return DistributedSchedMissionManager::GetInstance()
        .NotifyMissionsChangedFromRemote(callerInfo, missionInfos);
}

std::unique_ptr<Snapshot> DistributedSchedService::GetRemoteSnapshotInfo(const std::u16string& deviceId,
    int32_t missionId)
{
    std::string strDeviceId = Str16ToStr8(deviceId);
    HILOGI("GetRemoteSnapShot missionId:%{public}d", missionId);
    return DistributedSchedMissionManager::GetInstance().GetRemoteSnapshotInfo(strDeviceId, missionId);
}

int32_t DistributedSchedService::GetRemoteMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    std::unique_ptr<MissionSnapshot>& missionSnapshot)
{
    return DistributedSchedMissionManager::GetInstance()
        .GetRemoteMissionSnapshotInfo(networkId, missionId, missionSnapshot);
}

int32_t DistributedSchedService::CheckSupportOsd(const std::string& deviceId)
{
    return DistributedSchedMissionManager::GetInstance().CheckSupportOsd(deviceId);
}

void DistributedSchedService::GetCachedOsdSwitch(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values)
{
    return DistributedSchedMissionManager::GetInstance().GetCachedOsdSwitch(deviceIds, values);
}

int32_t DistributedSchedService::GetOsdSwitchValueFromRemote()
{
    return DistributedSchedMissionManager::GetInstance().GetOsdSwitchValueFromRemote();
}

int32_t DistributedSchedService::StoreSnapshotInfo(const std::string& deviceId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    return DistributedSchedMissionManager::GetInstance().StoreSnapshotInfo(deviceId, missionId, byteStream, len);
}

int32_t DistributedSchedService::RemoveSnapshotInfo(const std::string& deviceId, int32_t missionId)
{
    return DistributedSchedMissionManager::GetInstance().RemoveSnapshotInfo(deviceId, missionId);
}

int32_t DistributedSchedService::RegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    return DistributedSchedMissionManager::GetInstance().RegisterMissionListener(devId, obj);
}

int32_t DistributedSchedService::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    return DistributedSchedMissionManager::GetInstance().UnRegisterMissionListener(devId, obj);
}

int32_t DistributedSchedService::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    return DistributedSchedMissionManager::GetInstance().StartSyncRemoteMissions(devId, fixConflict, tag);
}

int32_t DistributedSchedService::StopSyncRemoteMissions(const std::string& devId)
{
    return DistributedSchedMissionManager::GetInstance().StopSyncRemoteMissions(devId, false, true);
}

int32_t DistributedSchedService::StartSyncMissionsFromRemote(const CallerInfo& callerInfo,
    std::vector<DstbMissionInfo>& missionInfos)
{
    return DistributedSchedMissionManager::GetInstance().StartSyncMissionsFromRemote(callerInfo, missionInfos);
}

int32_t DistributedSchedService::StopSyncMissionsFromRemote(const CallerInfo& callerInfo)
{
    DistributedSchedMissionManager::GetInstance().StopSyncMissionsFromRemote(callerInfo.sourceDeviceId);
    return ERR_NONE;
}

int32_t DistributedSchedService::UpdateOsdSwitchValueFromRemote(int32_t switchVal,
    const std::string& sourceDeviceId)
{
    return DistributedSchedMissionManager::GetInstance()
        .UpdateOsdSwitchValueFromRemote(switchVal, sourceDeviceId);
}
#endif

void CallerDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("CallerDeathRecipient OnRemoteDied called");
    DistributedSchedAdapter::GetInstance().ProcessCallerDied(remote.promote());
}
} // namespace DistributedSchedule
} // namespace OHOS