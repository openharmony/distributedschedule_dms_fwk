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

#include "distributed_sched_adapter.h"

#include "datetime_ex.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
#include "mission/distributed_sched_mission_manager.h"
#include "mission/mission_info_converter.h"
#endif
#include "os_account_manager.h"
#include "parcel_helper.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace std;
using namespace AAFwk;
using namespace AccountSA;
using namespace AppExecFwk;
using DstbMissionChangeListener = DistributedMissionChangeListener;
namespace {
// set a non-zero value on need later
constexpr int64_t DEVICE_OFFLINE_DELAY_TIME = 0;
const std::string TAG = "DistributedSchedAdapter";
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedAdapter);

void DistributedSchedAdapter::Init()
{
    if (dmsAdapterHandler_ == nullptr) {
        shared_ptr<EventRunner> runner = EventRunner::Create("dmsAdapter");
        if (runner == nullptr) {
            HILOGE("create runner failed");
            return;
        }
        dmsAdapterHandler_ = make_shared<EventHandler>(runner);
    }
}

void DistributedSchedAdapter::UnInit()
{
    dmsAdapterHandler_ = nullptr;
}

int32_t DistributedSchedAdapter::ConnectAbility(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const sptr<IRemoteObject>& callerToken)
{
    HILOGD("ConnectAbility");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want,
        iface_cast<AAFwk::IAbilityConnection>(connect), callerToken, ids[0]);
    return ret;
}

int32_t DistributedSchedAdapter::DisconnectAbility(const sptr<IRemoteObject>& connect)
{
    HILOGD("DisconnectAbility");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(
        iface_cast<AAFwk::IAbilityConnection>(connect));
    return ret;
}

void DistributedSchedAdapter::DeviceOnline(const std::string& deviceId)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("DeviceOnline dmsAdapterHandler is null");
        return;
    }

    if (deviceId.empty()) {
        HILOGW("DeviceOnline deviceId is empty");
        return;
    }
    HILOGD("process DeviceOnline deviceId is %s", deviceId.c_str());
    dmsAdapterHandler_->RemoveTask(deviceId);
}

void DistributedSchedAdapter::DeviceOffline(const std::string& deviceId)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("DeviceOffline dmsAdapterHandler is null");
        return;
    }

    if (deviceId.empty()) {
        HILOGW("DeviceOffline deviceId is empty");
        return;
    }
    HILOGD("process DeviceOffline deviceId is %s", deviceId.c_str());
    auto callback = [deviceId, this] () {
        ProcessDeviceOffline(deviceId);
    };
    if (!dmsAdapterHandler_->PostTask(callback, deviceId, DEVICE_OFFLINE_DELAY_TIME)) {
        HILOGW("DeviceOffline PostTask failed");
    }
}

void DistributedSchedAdapter::ProcessDeviceOffline(const std::string& deviceId)
{
    HILOGD("ProcessDeviceOffline");
    DistributedSchedService::GetInstance().ProcessDeviceOffline(deviceId);
}

void DistributedSchedAdapter::ProcessConnectDied(const sptr<IRemoteObject>& connect)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("ProcessConnectDied dmsAdapterHandler is null");
        return;
    }

    if (connect == nullptr) {
        HILOGE("ProcessConnectDied connect is null");
        return;
    }
    HILOGD("process connect died");
    auto callback = [connect] () {
        DistributedSchedService::GetInstance().ProcessConnectDied(connect);
    };
    if (!dmsAdapterHandler_->PostTask(callback)) {
        HILOGW("ProcessConnectDied PostTask failed");
    }
}

void DistributedSchedAdapter::ProcessCalleeDied(const sptr<IRemoteObject>& connect)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("ProcessCalleeDied dmsAdapterHandler is null");
        return;
    }
    if (connect == nullptr) {
        HILOGE("ProcessCalleeDied connect is null");
        return;
    }
    HILOGD("process callee died");
    auto callback = [connect] () {
        DistributedSchedService::GetInstance().ProcessCalleeDied(connect);
    };
    if (!dmsAdapterHandler_->PostTask(callback)) {
        HILOGE("ProcessCalleeDied PostTask failed");
    }
}

void DistributedSchedAdapter::ProcessCallerDied(const sptr<IRemoteObject>& connect, int32_t deviceType)
{
    if (dmsAdapterHandler_ == nullptr) {
        HILOGE("ProcessCallerDied dmsAdapterHandler is null");
        return;
    }
    if (connect == nullptr) {
        HILOGE("ProcessCallerDied connect is null");
        return;
    }
    HILOGD("process caller died");
    auto callback = [connect, deviceType] () {
        DistributedSchedService::GetInstance().ProcessCallerDied(connect, deviceType);
    };
    if (!dmsAdapterHandler_->PostTask(callback)) {
        HILOGE("ProcessCallerDied PostTask failed");
    }
}

int32_t DistributedSchedAdapter::ReleaseAbility(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element)
{
    HILOGD("ReleaseAbility called");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("ReleaseAbility:connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    AppExecFwk::ElementName elementWithoutDeviceId("", element.GetBundleName(), element.GetAbilityName());
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ReleaseAbility(
        iface_cast<AAFwk::IAbilityConnection>(connect), elementWithoutDeviceId);
    return ret;
}

int32_t DistributedSchedAdapter::StartAbilityByCall(const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect, const sptr<IRemoteObject>& callerToken)
{
    HILOGD("ResolveAbility called");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("ResolveAbility:connect ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(want,
        iface_cast<AAFwk::IAbilityConnection>(connect), callerToken);
    return ret;
}

bool DistributedSchedAdapter::InitHichainService()
{
    if (hichainGmInstance_ != nullptr) {
        HILOGI("hichain GmInstance is already exist");
        return true;
    }
    if (InitDeviceAuthService() != ERR_OK) {
        HILOGE("hichain init DeviceAuthService failed");
        return false;
    }
    hichainGmInstance_ = GetGmInstance();
    if (hichainGmInstance_ == nullptr) {
        HILOGE("hichain get GmInstance failed");
        return false;
    }
    return true;
}

bool DistributedSchedAdapter::CheckAccessToGroup(const std::string& groupId, const std::string& targetBundleName)
{
    std::lock_guard<std::mutex> autoLock(hichainLock_);
    int64_t begin = GetTickCount();
    if (!InitHichainService()) {
        return false;
    }
    int32_t ret = hichainGmInstance_->checkAccessToGroup(ANY_OS_ACCOUNT, targetBundleName.c_str(),
        groupId.c_str());
    HILOGI("[PerformanceTest] checkAccessToGroup spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (ret != ERR_OK) {
        HILOGE("hichain checkAccessToGroup failed, ret:%{public}d", ret);
        return false;
    }
    HILOGD("hichain checkAccessToGroup success");
    return true;
}

bool DistributedSchedAdapter::GetRelatedGroups(const std::string& udid, const std::string& bundleName,
    std::string& returnGroups)
{
    std::lock_guard<std::mutex> autoLock(hichainLock_);
    int64_t begin = GetTickCount();
    if (!InitHichainService()) {
        return false;
    }
    uint32_t groupNum = 0;
    char* groupsJsonStr = nullptr;
    int32_t ret = hichainGmInstance_->getRelatedGroups(ANY_OS_ACCOUNT, bundleName.c_str(), udid.c_str(),
        &groupsJsonStr, &groupNum);
    HILOGI("[PerformanceTest] getRelatedGroups spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (ret != ERR_OK) {
        HILOGE("hichain getRelatedGroups failed, ret:%{public}d", ret);
        return false;
    }
    if (groupsJsonStr == nullptr || groupNum == 0) {
        HILOGE("groupsJsonStr is nullptr");
        return false;
    }
    returnGroups = groupsJsonStr;
    return true;
}

#ifdef SUPPORT_DISTRIBUTED_MISSION_MANAGER
int32_t DistributedSchedAdapter::GetLocalMissionInfos(int32_t numMissions,
    std::vector<DstbMissionInfo>& missionInfos)

{
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("get ability server failed, errCode = %{public}d", errCode);
        return errCode;
    }
    std::vector<MissionInfo> amsMissions;
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->GetMissionInfos("", numMissions, amsMissions);
    if (ret != ERR_OK) {
        HILOGE("GetMissionInfos failed, ret = %{public}d", ret);
        return ret;
    }
    if (amsMissions.empty()) {
        HILOGI("empty missions");
        return ERR_OK;
    }
    HILOGI("GetMissionInfos size:%{public}zu", amsMissions.size());
    return MissionInfoConverter::ConvertToDstbMissionInfos(amsMissions, missionInfos);
}

bool DistributedSchedAdapter::AllowMissionUid(int32_t uid)
{
    if (uid < 0) {
        HILOGE("AllowMissionUid invalid params!");
        return false;
    }
    return true;
}

int32_t DistributedSchedAdapter::RegisterMissionListener(const sptr<DstbMissionChangeListener>& listener)
{
    HILOGD("called.");
    if (listener == nullptr) {
        HILOGE("listener is null");
        return INVALID_PARAMETERS_ERR;
    }
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("get ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterMissionListener(listener);
    if (ret != ERR_OK) {
        HILOGE("RegisterMissionListener failed, ret=%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t DistributedSchedAdapter::UnRegisterMissionListener(const sptr<DstbMissionChangeListener>& listener)
{
    if (listener == nullptr) {
        HILOGE("listener is null");
        return INVALID_PARAMETERS_ERR;
    }
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("get ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }

    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->UnRegisterMissionListener(listener);
    if (ret != ERR_OK) {
        HILOGE("UnRegisterMissionListener failed, ret=%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t DistributedSchedAdapter::GetLocalMissionSnapshotInfo(const std::string& networkId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    int64_t begin = GetTickCount();
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOGE("get ability server failed, errCode=%{public}d", errCode);
        return errCode;
    }
    errCode = AAFwk::AbilityManagerClient::GetInstance()->GetMissionSnapshot(networkId,
        missionId, missionSnapshot);
    HILOGI("[PerformanceTest] GetMissionSnapshot spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (errCode != ERR_OK) {
        HILOGE("get mission snapshot failed, missionId=%{public}d, errCode=%{public}d", missionId, errCode);
        return errCode;
    }
    if (missionSnapshot.snapshot == nullptr) {
        HILOGE("pixel map is nullptr!");
        return ERR_NULL_OBJECT;
    }
    HILOGD("pixelMap size:%{public}d", missionSnapshot.snapshot->GetCapacity());
    return ERR_OK;
}

int32_t DistributedSchedAdapter::GetOsdSwitch()
{
    return MISSION_OSD_ENABLED;
}
#endif

void DistributedSchedAdapter::OnOsdEventOccur(int32_t flag)
{
}
} // namespace DistributedSchedule
} // namespace OHOS
