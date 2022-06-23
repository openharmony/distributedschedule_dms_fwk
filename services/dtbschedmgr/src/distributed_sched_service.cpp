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
#include "ability_manager_errors.h"
#include "adapter/dnetwork_adapter.h"
#include "bundle/bundle_manager_internal.h"
#include "connect_death_recipient.h"
#include "datetime_ex.h"
#include "distributed_sched_adapter.h"
#include "distributed_sched_dumper.h"
#include "distributed_sched_permission.h"
#include "dms_callback_task.h"
#include "dms_free_install_callback.h"
#include "dms_token_callback.h"
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
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
const int DEFAULT_REQUEST_CODE = -1;
const std::u16string CONNECTION_CALLBACK_INTERFACE_TOKEN = u"ohos.abilityshell.DistributedConnection";
const std::u16string COMPONENT_CHANGE_INTERFACE_TOKEN = u"ohos.rms.DistributedComponent";
const std::u16string ABILITY_MANAGER_SERVICE_TOKEN = u"ohos.aafwk.AbilityManager";
const std::u16string ATOMIC_SERVICE_STATUS_CALLBACK_TOKEN = u"ohos.aafwk.IAtomicServiceStatusCallback";
const std::string BUNDLE_NAME_KEY = "bundleName";
const std::string VERSION_CODE_KEY = "version";
const std::string PID_KEY = "pid";
const std::string UID_KEY = "uid";
const std::string COMPONENT_TYPE_KEY = "componentType";
const std::string DEVICE_TYPE_KEY = "deviceType";
const std::string CHANGE_TYPE_KEY = "changeType";
constexpr int32_t BIND_CONNECT_RETRY_TIMES = 3;
constexpr int32_t BIND_CONNECT_TIMEOUT = 500; // 500ms
constexpr int32_t MAX_DISTRIBUTED_CONNECT_NUM = 600;
constexpr int32_t INVALID_CALLER_UID = -1;
constexpr int32_t IASS_CALLBACK_ON_REMOTE_FREE_INSTALL_DONE = 1;
constexpr int32_t DISTRIBUTED_COMPONENT_ADD = 1;
constexpr int32_t DISTRIBUTED_COMPONENT_REMOVE = 2;
constexpr int32_t REPORT_DISTRIBUTED_COMPONENT_CHANGE_CODE = 1;
}

extern "C" {
void OnStart()
{
    DistributedSchedService::GetInstance().OnStart();
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    return DistributedSchedService::GetInstance().OnRemoteRequest(code, data, reply, option);
}

void DeviceOnlineNotify(const std::string& deviceId)
{
    DistributedSchedAdapter::GetInstance().DeviceOnline(deviceId);
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().DeviceOnlineNotify(deviceId);
#endif
}

void DeviceOfflineNotify(const std::string& deviceId)
{
    DistributedSchedAdapter::GetInstance().DeviceOffline(deviceId);
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().DeviceOfflineNotify(deviceId);
#endif
}
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedService);
static const sptr<DistributedSchedService> INSTANCE = &DistributedSchedService::GetInstance();

DistributedSchedService::DistributedSchedService()
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

    DmsCallbackTaskInitCallbackFunc freeCallback = [this] (int64_t taskId) {
        HILOGW("DmsCallbackTaskInitCallbackFunc timeout, taskId:%{public}" PRId64 ".", taskId);
        NotifyCompleteFreeInstallFromRemote(taskId, AAFwk::FREE_INSTALL_TIMEOUT);
    };

    dschedContinuation_ = std::make_shared<DSchedContinuation>();
    dmsCallbackTask_ = std::make_shared<DmsCallbackTask>();
    dschedContinuation_->Init(continuationCallback);
    dmsCallbackTask_->Init(freeCallback);
    HILOGI("OnStart start service success.");
}

bool DistributedSchedService::Init()
{
    HILOGD("Init ready to init.");
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().Init();
#endif
    HILOGD("Init init success.");
    DistributedSchedAdapter::GetInstance().Init();
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
    DistributedSchedMissionManager::GetInstance().InitDataStorage();
#endif
    connectDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new ConnectDeathRecipient());
    callerDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new CallerDeathRecipient());
    callerDeathRecipientForLocalDevice_ = sptr<IRemoteObject::DeathRecipient>(
        new CallerDeathRecipient(IDistributedSched::CALLER));
    if (componentChangeHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("DmsComponentChange");
        componentChangeHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    return true;
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
    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!BundleManagerInternal::GetBundleNameListFromBms(callerInfo.uid, callerInfo.bundleNames)) {
        HILOGE("GetBundleNameListFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    AccountInfo accountInfo;
    int32_t ret = DistributedSchedPermission::GetInstance().GetAccountInfo(deviceId, callerInfo, accountInfo);
    if (ret != ERR_OK) {
        HILOGE("GetAccountInfo failed");
        return ret;
    }
    AAFwk::Want* newWant = const_cast<Want*>(&want);
    newWant->SetParam(DMS_SRC_NETWORK_ID, localDeviceId);
    HILOGI("[PerformanceTest] StartRemoteAbility transact begin");
    int32_t result = remoteDms->StartAbilityFromRemote(*newWant, abilityInfo, requestCode, callerInfo, accountInfo);
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
    ErrCode err = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, deviceId);
    if (err != ERR_OK) {
        HILOGE("CheckDPermission denied!!");
        return err;
    }
    return StartAbility(want, requestCode);
}

int32_t DistributedSchedService::SendResultFromRemote(OHOS::AAFwk::Want& want, int32_t requestCode,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo, int32_t resultCode)
{
    std::string localDeviceId;
    std::string deviceId = want.GetStringParam(DMS_SRC_NETWORK_ID);
    want.RemoveParam(DMS_SRC_NETWORK_ID);
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, deviceId, callerInfo.sourceDeviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    DistributedSchedPermission& permissionInstance = DistributedSchedPermission::GetInstance();
    ErrCode err = permissionInstance.CheckDPermission(want, callerInfo, accountInfo, deviceId);
    if (err != ERR_OK) {
        HILOGE("CheckDPermission denied!!");
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("connect ability server failed %{public}d", err);
        return err;
    }
    err = AAFwk::AbilityManagerClient::GetInstance()->SendResultToAbility(requestCode, resultCode, want);
    if (err != ERR_OK) {
        HILOGE("SendResult failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::ContinueLocalMission(const std::string& dstDeviceId, int32_t missionId,
    const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    if (dschedContinuation_ == nullptr) {
        HILOGE("continuation object null!");
        return INVALID_PARAMETERS_ERR;
    }
    if (dschedContinuation_->IsInContinuationProgress(missionId)) {
        HILOGE("ContinueLocalMission already in progress!");
        return INVALID_PARAMETERS_ERR;
    }

    MissionInfo missionInfo;
    int32_t result = AbilityManagerClient::GetInstance()->GetMissionInfo("", missionId, missionInfo);
    if (result != ERR_OK) {
        HILOGE("get missionInfo failed");
        return INVALID_PARAMETERS_ERR;
    }
    std::string bundleName = missionInfo.want.GetBundle();
    missionInfo.want.SetParams(wantParams);
    bool isFreeInstall = false;
    DistributedBundleInfo remoteBundleInfo;
    result = BundleManagerInternal::CheckRemoteBundleInfoForContinuation(dstDeviceId,
        bundleName, remoteBundleInfo);
    if (result != ERR_OK) {
        isFreeInstall = missionInfo.want.GetBoolParam("isFreeInstall", false);
        if (!isFreeInstall) {
            HILOGE("ContinueLocalMission result: %{public}d!", result);
            return result;
        }
        dschedContinuation_->PopCallback(missionId);
    }
    dschedContinuation_->PushCallback(missionId, callback, isFreeInstall);
    uint32_t remoteBundleVersion = remoteBundleInfo.versionCode;

    result = AbilityManagerClient::GetInstance()->ContinueAbility(dstDeviceId, missionId, remoteBundleVersion);
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
        return ContinueLocalMission(dstDeviceId, missionId, callback, wantParams);
    } else if (dstDeviceId == localDevId) {
        return ContinueRemoteMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    } else {
        HILOGE("source or target device must be local!");
        return INVALID_PARAMETERS_ERR;
    }
}

int32_t DistributedSchedService::SetWantForContinuation(AAFwk::Want& newWant, int32_t missionId)
{
    std::string devId;
    if (!GetLocalDeviceId(devId)) {
        HILOGE("StartContinuation get local deviceId failed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    newWant.SetParam("sessionId", missionId);
    newWant.SetParam("deviceId", devId);
    BundleInfo localBundleInfo;
    if (BundleManagerInternal::GetLocalBundleInfo(newWant.GetBundle(), localBundleInfo) != ERR_OK) {
        HILOGE("get local bundle info failed");
        return INVALID_PARAMETERS_ERR;
    }
    newWant.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(localBundleInfo.versionCode));
    HILOGD("local version = %{public}u!", localBundleInfo.versionCode);
    return ERR_OK;
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
        want.GetElement().GetDeviceID().c_str(), want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());

    if (dschedContinuation_ == nullptr) {
        HILOGE("StartContinuation continuation object null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    if (!dschedContinuation_->IsInContinuationProgress(missionId)) {
        dschedContinuation_->SetTimeOut(missionId);
    }

    AAFwk::Want newWant = want;
    int result = SetWantForContinuation(newWant, missionId);
    if (result != ERR_OK) {
        HILOGE("set new want failed");
        return result;
    }
    bool flag = dschedContinuation_->IsFreeInstall(missionId);
    if (flag) {
        result = StartRemoteFreeInstall(newWant, callerUid, 0, accessToken, nullptr);
        if (result != ERR_OK) {
            HILOGE("continue free install failed, result = %{public}d", result);
            return result;
        }
    } else {
        result = StartRemoteAbility(newWant, callerUid, 0, accessToken);
        if (result != ERR_OK) {
            HILOGE("continue ability failed, errorCode = %{public}d", result);
            return result;
        }
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
            result = AbilityManagerClient::GetInstance()->CleanMission(missionId);
            HILOGD("clean mission result:%{public}d", result);
        }
        result = dschedContinuation_->NotifyMissionCenterResult(missionId, isSuccess);
    } else {
        result = AbilityManagerClient::GetInstance()->NotifyContinuationResult(missionId, isSuccess);
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
        ReportDistributedComponentChange(callerInfo, DISTRIBUTED_COMPONENT_ADD, IDistributedSched::CONNECT,
            IDistributedSched::CALLER);
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
        HILOGE("GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!BundleManagerInternal::GetBundleNameListFromBms(callerInfo.uid, callerInfo.bundleNames)) {
        HILOGE("GetBundleNameListFromBms failed");
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
    int32_t ret = DistributedSchedPermission::GetInstance().GetAccountInfo(remoteDeviceId, callerInfo, accountInfo);
    if (ret != ERR_OK) {
        HILOGE("GetAccountInfo failed");
        return ret;
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

void DistributedSchedService::ProcessCallerDied(const sptr<IRemoteObject>& connect, int32_t deviceType)
{
    if (connect == nullptr) {
        HILOGE("ProcessCallerDied connect is null");
        return;
    }
    HILOGI("Caller Died DeviceType : %{public}d", deviceType);
    if (deviceType == IDistributedSched::CALLER) {
        HandleLocalCallerDied(connect);
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
            ReportDistributedComponentChange(itConnect->second, DISTRIBUTED_COMPONENT_REMOVE,
                IDistributedSched::CALL, IDistributedSched::CALLEE);
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

void DistributedSchedService::HandleLocalCallerDied(const sptr<IRemoteObject>& connect)
{
    std::lock_guard<std::mutex> autoLock(callerLock_);
    auto it = callerMap_.find(connect);
    if (it != callerMap_.end()) {
        std::list<ConnectAbilitySession> sessionsList = it->second;
        if (!sessionsList.empty()) {
            ReportDistributedComponentChange(sessionsList.front().GetCallerInfo(), DISTRIBUTED_COMPONENT_REMOVE,
                IDistributedSched::CALL, IDistributedSched::CALLER);
        }
        callerMap_.erase(it);
        HILOGI("remove connection success");
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
        ReportDistributedComponentChange(itConnect->second, DISTRIBUTED_COMPONENT_REMOVE,
            IDistributedSched::CALL, IDistributedSched::CALLEE);
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
    int32_t ret = DistributedSchedPermission::GetInstance().GetAccountInfo(remoteDeviceId, callerInfo, accountInfo);
    if (ret != ERR_OK) {
        HILOGE("GetAccountInfo failed");
        return ret;
    }
    int32_t result = remoteDms->StartAbilityByCallFromRemote(want, connect, callerInfo, accountInfo);
    HILOGD("[PerformanceTest] TryStartRemoteAbilityByCall RPC end");
    if (result == ERR_OK) {
        SaveCallerComponent(want, connect, callerInfo);
    } else {
        HILOGE("TryStartRemoteAbilityByCall failed, result : %{public}d", result);
    }
    return result;
}

void DistributedSchedService::SaveCallerComponent(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const CallerInfo& callerInfo)
{
    std::lock_guard<std::mutex> autoLock(callerLock_);
    auto itConnect = callerMap_.find(connect);
    if (itConnect == callerMap_.end()) {
        connect->AddDeathRecipient(callerDeathRecipientForLocalDevice_);
        ReportDistributedComponentChange(callerInfo, DISTRIBUTED_COMPONENT_ADD, IDistributedSched::CALL,
            IDistributedSched::CALLER);
    }
    auto& sessionsList = callerMap_[connect];
    std::string remoteDeviceId = want.GetElement().GetDeviceID();
    for (auto& session : sessionsList) {
        if (remoteDeviceId == session.GetDestinationDeviceId()) {
            session.AddElement(want.GetElement());
            // already added session for remote device
            return;
        }
    }
    // connect to another remote device, add a new session to list
    auto& session = sessionsList.emplace_back(callerInfo.sourceDeviceId, remoteDeviceId, callerInfo);
    session.AddElement(want.GetElement());
    HILOGD("add connection success");
}

void DistributedSchedService::RemoveCallerComponent(const sptr<IRemoteObject>& connect)
{
    std::lock_guard<std::mutex> autoLock(callerLock_);
    auto it = callerMap_.find(connect);
    if (it != callerMap_.end()) {
        std::list<ConnectAbilitySession> sessionsList = it->second;
        connect->RemoveDeathRecipient(callerDeathRecipientForLocalDevice_);
        if (!sessionsList.empty()) {
            ReportDistributedComponentChange(sessionsList.front().GetCallerInfo(), DISTRIBUTED_COMPONENT_REMOVE,
                IDistributedSched::CALL, IDistributedSched::CALLER);
        }
        callerMap_.erase(it);
        HILOGI("remove connection success");
    }
}

void DistributedSchedService::ProcessCalleeOffline(const std::string& deviceId)
{
    std::lock_guard<std::mutex> autoLock(callerLock_);
    for (auto iter = callerMap_.begin(); iter != callerMap_.end();) {
        std::list<ConnectAbilitySession>& sessionsList = iter->second;
        auto itSession = std::find_if(sessionsList.begin(), sessionsList.end(), [&deviceId](const auto& session) {
            return session.GetDestinationDeviceId() == deviceId;
        });
        CallerInfo callerInfo;
        if (itSession != sessionsList.end()) {
            callerInfo = itSession->GetCallerInfo();
            sessionsList.erase(itSession);
        }

        if (sessionsList.empty()) {
            if (iter->first != nullptr) {
                iter->first->RemoveDeathRecipient(callerDeathRecipientForLocalDevice_);
            }
            ReportDistributedComponentChange(callerInfo, DISTRIBUTED_COMPONENT_REMOVE,
                IDistributedSched::CALL, IDistributedSched::CALLER);
            callerMap_.erase(iter++);
        } else {
            iter++;
        }
    }
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
    CallerInfo callerInfo = { callerUid, callerPid };
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.accessToken = accessToken;
    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!BundleManagerInternal::GetBundleNameListFromBms(callerInfo.uid, callerInfo.bundleNames)) {
        HILOGE("GetBundleNameListFromBms failed");
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
    if (result == ERR_OK) {
        RemoveCallerComponent(connect);
    } else {
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
            ReportDistributedComponentChange(connectInfo, DISTRIBUTED_COMPONENT_ADD,
                IDistributedSched::CALL, IDistributedSched::CALLEE);
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
        ReportDistributedComponentChange(itConnect->second, DISTRIBUTED_COMPONENT_REMOVE,
            IDistributedSched::CALL, IDistributedSched::CALLEE);
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

int32_t DistributedSchedService::RegisterDistributedComponentListener(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        HILOGE("RegisterDistributedComponentListener callback is null");
        return INVALID_PARAMETERS_ERR;
    }
    distributedComponentListener_ = callback;
    return ERR_OK;
}

int32_t DistributedSchedService::GetDistributedComponentList(std::vector<std::string>& distributedComponents)
{
    GetConnectComponentList(distributedComponents);
    GetCallComponentList(distributedComponents);
    return ERR_OK;
}

void DistributedSchedService::GetConnectComponentList(std::vector<std::string>& distributedComponents)
{
    {
        std::lock_guard<std::mutex> autoLock(distributedLock_);
        for (const auto& iter : distributedConnectAbilityMap_) {
            if (iter.second.empty()) {
                continue;
            }
            CallerInfo callerInfo = iter.second.front().GetCallerInfo();
            nlohmann::json componentInfoJson;
            componentInfoJson[PID_KEY] = callerInfo.pid;
            componentInfoJson[UID_KEY] = callerInfo.uid;
            componentInfoJson[BUNDLE_NAME_KEY] =
                callerInfo.bundleNames.empty() ? std::string() : callerInfo.bundleNames.front();
            componentInfoJson[COMPONENT_TYPE_KEY] = IDistributedSched::CONNECT;
            componentInfoJson[DEVICE_TYPE_KEY] = IDistributedSched::CALLER;
            std::string componentInfo = componentInfoJson.dump();
            distributedComponents.emplace_back(componentInfo);
        }
    }
    {
        std::lock_guard<std::mutex> autoLock(connectLock_);
        for (const auto& iter : connectAbilityMap_) {
            ConnectInfo connectInfo = iter.second;
            nlohmann::json componentInfoJson;
            componentInfoJson[UID_KEY] = BundleManagerInternal::GetUidFromBms(connectInfo.element.GetBundleName());
            componentInfoJson[BUNDLE_NAME_KEY] = connectInfo.element.GetBundleName();
            componentInfoJson[COMPONENT_TYPE_KEY] = IDistributedSched::CONNECT;
            componentInfoJson[DEVICE_TYPE_KEY] = IDistributedSched::CALLEE;
            std::string componentInfo = componentInfoJson.dump();
            distributedComponents.emplace_back(componentInfo);
        }
    }
}

void DistributedSchedService::GetCallComponentList(std::vector<std::string>& distributedComponents)
{
    {
        std::lock_guard<std::mutex> autoLock(callerLock_);
        for (const auto& iter : callerMap_) {
            if (iter.second.empty()) {
                continue;
            }
            CallerInfo callerInfo = iter.second.front().GetCallerInfo();
            nlohmann::json componentInfoJson;
            componentInfoJson[PID_KEY] = callerInfo.pid;
            componentInfoJson[UID_KEY] = callerInfo.uid;
            componentInfoJson[BUNDLE_NAME_KEY] =
                callerInfo.bundleNames.empty() ? std::string() : callerInfo.bundleNames.front();
            componentInfoJson[COMPONENT_TYPE_KEY] = IDistributedSched::CALL;
            componentInfoJson[DEVICE_TYPE_KEY] = IDistributedSched::CALLER;
            std::string componentInfo = componentInfoJson.dump();
            distributedComponents.emplace_back(componentInfo);
        }
    }
    {
        std::lock_guard<std::mutex> autoLock(calleeLock_);
        for (const auto& iter : calleeMap_) {
            ConnectInfo connectInfo = iter.second;
            nlohmann::json componentInfoJson;
            componentInfoJson[UID_KEY] = BundleManagerInternal::GetUidFromBms(connectInfo.element.GetBundleName());
            componentInfoJson[BUNDLE_NAME_KEY] = connectInfo.element.GetBundleName();
            componentInfoJson[COMPONENT_TYPE_KEY] = IDistributedSched::CALL;
            componentInfoJson[DEVICE_TYPE_KEY] = IDistributedSched::CALLEE;
            std::string componentInfo = componentInfoJson.dump();
            distributedComponents.emplace_back(componentInfo);
        }
    }
}

bool DistributedSchedService::HandleDistributedComponentChange(const std::string& componentInfo)
{
    if (componentChangeHandler_ == nullptr) {
        HILOGE("HandleDistributedComponentChange componentChangeHandler_ is null");
        return false;
    }
    auto func = [this, componentInfo]() {
        HILOGI("HandleDistributedComponentChange call callback");
        if (distributedComponentListener_ == nullptr) {
            HILOGW("HandleDistributedComponentChange distributedComponentListener_ is null");
            return;
        }
        MessageParcel data;
        if (!data.WriteInterfaceToken(COMPONENT_CHANGE_INTERFACE_TOKEN)) {
            HILOGE("HandleDistributedComponentChange WriteInterfaceToken error");
            return;
        }
        PARCEL_WRITE_HELPER_NORET(data, String, componentInfo);
        MessageParcel reply;
        MessageOption option;
        int32_t result = distributedComponentListener_->SendRequest(REPORT_DISTRIBUTED_COMPONENT_CHANGE_CODE,
            data, reply, option);
        HILOGD("HandleDistributedComponentChange result is %{public}d", result);
    };
    if (!componentChangeHandler_->PostTask(func)) {
        HILOGE("HandleDistributedComponentChange handler postTask failed");
        return false;
    }
    return true;
}

void DistributedSchedService::ReportDistributedComponentChange(const CallerInfo& callerInfo, int32_t changeType,
    int32_t componentType, int32_t deviceType)
{
    if (distributedComponentListener_ == nullptr) {
        HILOGI("distributedComponentListener_ nullptr");
        return;
    }
    HILOGI("caller report");
    nlohmann::json componentInfoJson;
    componentInfoJson[PID_KEY] = callerInfo.pid;
    componentInfoJson[UID_KEY] = callerInfo.uid;
    componentInfoJson[BUNDLE_NAME_KEY] =
        callerInfo.bundleNames.empty() ? std::string() : callerInfo.bundleNames.front();
    componentInfoJson[COMPONENT_TYPE_KEY] = componentType;
    componentInfoJson[DEVICE_TYPE_KEY] = deviceType;
    componentInfoJson[CHANGE_TYPE_KEY] = changeType;
    std::string componentInfo = componentInfoJson.dump();
    if (!HandleDistributedComponentChange(componentInfo)) {
        HILOGW("ReportDistributedComponentChange notify rms failed");
    }
}

void DistributedSchedService::ReportDistributedComponentChange(const ConnectInfo& connectInfo, int32_t changeType,
    int32_t componentType, int32_t deviceType)
{
    if (distributedComponentListener_ == nullptr) {
        HILOGI("distributedComponentListener_ nullptr");
        return;
    }
    HILOGI("callee report");
    nlohmann::json componentInfoJson;
    componentInfoJson[UID_KEY] = BundleManagerInternal::GetUidFromBms(connectInfo.element.GetBundleName());
    componentInfoJson[BUNDLE_NAME_KEY] = connectInfo.element.GetBundleName();
    componentInfoJson[COMPONENT_TYPE_KEY] = componentType;
    componentInfoJson[DEVICE_TYPE_KEY] = deviceType;
    componentInfoJson[CHANGE_TYPE_KEY] = changeType;
    std::string componentInfo = componentInfoJson.dump();
    if (!HandleDistributedComponentChange(componentInfo)) {
        HILOGW("ReportDistributedComponentChange notify rms failed");
    }
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
    int32_t result = permissionInstance.CheckDPermission(want, callerInfo, accountInfo,
        localDeviceId, true);
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
            ConnectInfo connectInfo {callerInfo, callbackWrapper, want.GetElement()};
            ReportDistributedComponentChange(connectInfo, DISTRIBUTED_COMPONENT_ADD,
                IDistributedSched::CONNECT, IDistributedSched::CALLEE);
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
            if (!sessionsList.empty()) {
                ReportDistributedComponentChange(sessionsList.front().GetCallerInfo(), DISTRIBUTED_COMPONENT_REMOVE,
                    IDistributedSched::CONNECT, IDistributedSched::CALLER);
            }
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
            ReportDistributedComponentChange(itConnect->second, DISTRIBUTED_COMPONENT_REMOVE,
                IDistributedSched::CONNECT, IDistributedSched::CALLEE);
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
                ReportDistributedComponentChange(connectInfo, DISTRIBUTED_COMPONENT_REMOVE,
                    IDistributedSched::CONNECT, IDistributedSched::CALLEE);
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
            CallerInfo callerInfo;
            if (itSession != sessionsList.end()) {
                NotifyDeviceOfflineToAppLocked(iter->first, *itSession);
                callerInfo = itSession->GetCallerInfo();
                sessionsList.erase(itSession);
            }

            if (sessionsList.empty()) {
                if (iter->first != nullptr) {
                    iter->first->RemoveDeathRecipient(connectDeathRecipient_);
                }
                DecreaseConnectLocked(uid);
                ReportDistributedComponentChange(callerInfo, DISTRIBUTED_COMPONENT_REMOVE,
                    IDistributedSched::CONNECT, IDistributedSched::CALLER);
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
                ReportDistributedComponentChange(connectInfo, DISTRIBUTED_COMPONENT_REMOVE,
                    IDistributedSched::CONNECT, IDistributedSched::CALLEE);
                connectAbilityMap_.erase(iter++);
                HILOGI("ProcessDeviceOffline erase connection success");
            } else {
                iter++;
            }
        }
    }
    ProcessCalleeOffline(deviceId);
    ProcessFreeInstallOffline(deviceId);
}

void DistributedSchedService::ProcessFreeInstallOffline(const std::string& deviceId)
{
    if (dmsCallbackTask_ == nullptr) {
        HILOGE("callbackTask object null!");
        return;
    }
    dmsCallbackTask_->NotifyDeviceOffline(deviceId);
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
                ReportDistributedComponentChange(callerInfo, DISTRIBUTED_COMPONENT_REMOVE,
                    IDistributedSched::CONNECT, IDistributedSched::CALLER);
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
    DistributedSchedAdapter::GetInstance().ProcessCallerDied(remote.promote(), deviceType_);
}

int32_t DistributedSchedService::SetCallerInfo(
    int32_t callerUid, std::string localDeviceId, uint32_t accessToken, CallerInfo& callerInfo)
{
    callerInfo.uid = callerUid;
    callerInfo.callerType = CALLER_TYPE_HARMONY;
    callerInfo.sourceDeviceId = localDeviceId;
    callerInfo.accessToken = accessToken;
    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!BundleManagerInternal::GetBundleNameListFromBms(callerInfo.uid, callerInfo.bundleNames)) {
        HILOGE("GetBundleNameListFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t DistributedSchedService::StartRemoteFreeInstall(const OHOS::AAFwk::Want& want, int32_t callerUid,
    int32_t requestCode, uint32_t accessToken, const sptr<IRemoteObject>& callback)
{
    HILOGI("called");
    std::string localDeviceId;
    std::string deviceId = want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }

    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("get remoteDms failed");
        return INVALID_PARAMETERS_ERR;
    }

    if (dmsCallbackTask_ == nullptr) {
        HILOGE("callbackTask object null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    int64_t taskId = dmsCallbackTask_->GenerateTaskId();
    LaunchType launchType = LaunchType::FREEINSTALL_START;
    if (((want.GetFlags() & AAFwk::Want::FLAG_ABILITY_CONTINUATION) != 0)) {
        launchType = LaunchType::FREEINSTALL_CONTINUE;
    }
    if (dmsCallbackTask_->PushCallback(taskId, callback, deviceId, launchType, want) != ERR_OK) {
        HILOGE("Push callback failed!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    if (launchType == LaunchType::FREEINSTALL_CONTINUE) {
        dmsCallbackTask_->SetContinuationMissionMap(taskId, want.GetIntParam("sessionId", -1));
    }

    CallerInfo callerInfo;
    if (SetCallerInfo(callerUid, localDeviceId, accessToken, callerInfo) != ERR_OK) {
        HILOGE("SetCallerInfo failed");
        return INVALID_PARAMETERS_ERR;
    }
    AccountInfo accountInfo = {};
    if ((DistributedSchedPermission::GetInstance().GetAccountInfo(deviceId, callerInfo, accountInfo)) != ERR_OK) {
        HILOGE("GetAccountInfo failed");
        return INVALID_PARAMETERS_ERR;
    }
    AAFwk::Want* newWant = const_cast<Want*>(&want);
    newWant->SetParam(DMS_SRC_NETWORK_ID, localDeviceId);
    FreeInstallInfo info = {.want = *newWant, .requestCode = requestCode, .callerInfo = callerInfo,
        .accountInfo = accountInfo};
    int32_t result = remoteDms->StartFreeInstallFromRemote(info, taskId);
    if (result != ERR_OK) {
        HILOGE("result = %{public}d", result);
        CallbackTaskItem item = dmsCallbackTask_->PopCallback(taskId);
        NotifyFreeInstallResult(item, result);
    }
    return result;
}

int32_t DistributedSchedService::StartFreeInstallFromRemote(const FreeInstallInfo& info, int64_t taskId)
{
    HILOGI("begin taskId : %{public} " PRId64 ". ", taskId);
    std::string localDeviceId;
    std::string deviceId = info.want.GetElement().GetDeviceID();
    if (!GetLocalDeviceId(localDeviceId) ||
        !CheckDeviceIdFromRemote(localDeviceId, deviceId, info.callerInfo.sourceDeviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("connect ability server failed %{public}d", err);
        return err;
    }
    std::vector<int32_t> ids;
    err = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (err != ERR_OK || ids.empty()) {
        HILOGE("QueryActiveOsAccountIds passing param invalid or return error!, err : %{public}d", err);
        return INVALID_PARAMETERS_ERR;
    }

    sptr<DmsFreeInstallCallback> callback = new DmsFreeInstallCallback(taskId, info);
    err = AAFwk::AbilityManagerClient::GetInstance()->FreeInstallAbilityFromRemote(
        info.want, callback, ids[0], info.requestCode);
    if (err != ERR_OK) {
        HILOGE("FreeInstallAbilityFromRemote failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::NotifyCompleteFreeInstall(
    const FreeInstallInfo& info, int64_t taskId, int32_t resultCode)
{
    HILOGI("taskId = %{public}" PRId64 ".", taskId);
    if (taskId <= 0) {
        HILOGE("taskId invalid!");
        return INVALID_PARAMETERS_ERR;
    }
    if (resultCode != ERR_OK) {
        HILOGE("free install failed, resultCode : %{public}d", resultCode);
        return HandleRemoteNotify(info, taskId, resultCode);
    }
    int32_t result = StartLocalAbility(info, taskId, resultCode);
    return HandleRemoteNotify(info, taskId, result);
}

int32_t DistributedSchedService::StartLocalAbility(const FreeInstallInfo& info, int64_t taskId, int32_t resultCode)
{
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId)) {
        HILOGE("get local deviceId failed");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    DistributedSchedPermission &permissionInstance = DistributedSchedPermission::GetInstance();
    ErrCode err = permissionInstance.CheckDPermission(info.want, info.callerInfo, info.accountInfo, localDeviceId);
    if (err != ERR_OK) {
        HILOGE("CheckDPermission denied!!");
        return err;
    }
    AAFwk::Want* want = const_cast<Want*>(&info.want);
    want->RemoveFlags(OHOS::AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
    return StartAbility(*want, info.requestCode);
}

int32_t DistributedSchedService::StartAbility(const OHOS::AAFwk::Want& want, int32_t requestCode)
{
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (err != ERR_OK) {
        HILOGE("connect ability server failed %{public}d", err);
        return err;
    }
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    if (requestCode != DEFAULT_REQUEST_CODE) {
        HILOGI("StartAbilityForResult start");
        sptr<IRemoteObject> dmsTokenCallback = new DmsTokenCallback();
        err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, dmsTokenCallback, requestCode, ids[0]);
    } else {
        HILOGI("StartAbility start");
        err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, requestCode, ids[0]);
    }
    if (err != ERR_OK) {
        HILOGE("StartAbility failed %{public}d", err);
    }
    return err;
}

int32_t DistributedSchedService::HandleRemoteNotify(const FreeInstallInfo& info, int64_t taskId, int32_t resultCode)
{
    HILOGI("begin taskId = %{public}" PRId64 ", resultCode = %{public}d", taskId, resultCode);
    sptr<IDistributedSched> remoteDms = GetRemoteDms(info.callerInfo.sourceDeviceId);
    if (remoteDms == nullptr) {
        HILOGE("get remote dms null!");
        return INVALID_PARAMETERS_ERR;
    }
    if (taskId <= 0) {
        HILOGE("taskId invalid!");
        return INVALID_PARAMETERS_ERR;
    }
    return remoteDms->NotifyCompleteFreeInstallFromRemote(taskId, resultCode);
}

int32_t DistributedSchedService::NotifyCompleteFreeInstallFromRemote(int64_t taskId, int32_t resultCode)
{
    HILOGI("begin taskId = %{public}" PRId64 ", resultCode = %{public}d", taskId, resultCode);
    if (dmsCallbackTask_ == nullptr || dschedContinuation_ == nullptr) {
        HILOGE("callbackTask object null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    LaunchType launchType = dmsCallbackTask_->GetLaunchType(taskId);
    CallbackTaskItem item = dmsCallbackTask_->PopCallback(taskId);
    if (launchType == LaunchType::FREEINSTALL_START) {
        return NotifyFreeInstallResult(item, resultCode);
    }

    if (resultCode == ERR_OK) {
        HILOGD("continue free install success, waiting for continue result callback.");
        dmsCallbackTask_->PopContinuationMissionMap(taskId);
        return ERR_OK;
    }

    int32_t missionId = dmsCallbackTask_->GetContinuaionMissionId(taskId);
    NotifyContinuationCallbackResult(missionId, CONTINUE_FREE_INSTALL_FAILED);
    dmsCallbackTask_->PopContinuationMissionMap(taskId);
    return ERR_OK;
}

int32_t DistributedSchedService::NotifyFreeInstallResult(const CallbackTaskItem item, int32_t resultCode)
{
    HILOGI("taskId : %{public} " PRId64 ". ", item.taskId);
    if (item.callback == nullptr) {
        HILOGE("item callback null!");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(ATOMIC_SERVICE_STATUS_CALLBACK_TOKEN)) {
        HILOGE("Write interface token failed.");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOGE("Write resultCode error.");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    if (!data.WriteParcelable(&item.want)) {
        HILOGE("Write want error.");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    int32_t userId = 0;
    if (!data.WriteInt32(userId)) {
        HILOGE("Write userId error.");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    return item.callback->SendRequest(IASS_CALLBACK_ON_REMOTE_FREE_INSTALL_DONE, data, reply, option);
}
} // namespace DistributedSchedule
} // namespace OHOS
