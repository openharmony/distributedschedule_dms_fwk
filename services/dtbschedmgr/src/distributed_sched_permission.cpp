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

#include <cinttypes>

#include "distributed_sched_permission.h"

#include "caller_info.h"
#include "datetime_ex.h"
#include "distributed_permission_kit.h"
#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"

using namespace OHOS::Security;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedSchedPermission";
constexpr int32_t ERROR_DUID = -1;
constexpr int32_t PERMISSION_GRANTED = 0;
}

IMPLEMENT_SINGLE_INSTANCE(DistributedSchedPermission);

bool DistributedSchedPermission::CheckCustomPermission(const AppExecFwk::AbilityInfo& abilityInfo,
    const CallerInfo& callerInfo)
{
    const auto& permissions = abilityInfo.permissions;
    if (permissions.empty()) {
        HILOGI("no need any permission, so granted!");
        return true;
    }
    int32_t duid = callerInfo.duid;
    if (callerInfo.callerType == CALLER_TYPE_HARMONY) {
        duid = AllocateDuid(callerInfo.uid, callerInfo.sourceDeviceId);
        HILOGD("AllocateDuid uid = %{public}d, duid = %{public}d", callerInfo.uid, duid);
    }
    if (duid < 0) {
        HILOGE("CheckCustomPermission duid invalid!");
        return false;
    }
    for (const auto& permission : permissions) {
        if (permission.empty()) {
            continue;
        }
        auto result = Permission::DistributedPermissionKit::CheckDPermission(duid, permission);
        if (result == PERMISSION_GRANTED) {
            HILOGD("CheckCustomPermission duid:%{public}d, permission:%{public}s GRANTED!",
                duid, permission.c_str());
            return true;
        }
        HILOGI("duid:%{public}d, permission:%{public}s check failed!",
            duid, permission.c_str());
    }
    return false;
}

int32_t DistributedSchedPermission::AllocateDuid(int32_t rUid, const std::string& deviceId)
{
    if (rUid < 0 || deviceId.empty()) {
        HILOGE("DistributedSchedPermission::AllocateDuid invalid parameters!");
        return ERROR_DUID;
    }
    int64_t begin = GetTickCount();
    auto duid = Permission::DistributedPermissionKit::AllocateDuid(deviceId, rUid);
    int64_t end = GetTickCount();
    HILOGI("AllocateDuid spend:%{public}" PRId64 " ms", (end - begin));
    return duid;
}

int32_t DistributedSchedPermission::CheckDPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
    const AccountInfo& accountInfo, const AppExecFwk::AbilityInfo& abilityInfo, const std::string& localDeviceId)
{
    if (localDeviceId.empty()) {
        return INVALID_PARAMETERS_ERR;
    }
    AppExecFwk::AbilityInfo targetAbility;
    bool result = getTargetAbility(want, abilityInfo, localDeviceId, targetAbility, callerInfo);
    if (!result) {
        HILOGE("CheckDPermission can not find the target ability");
        return INVALID_PARAMETERS_ERR;
    }
    HILOGD("target ability info bundleName:%s abilityName:%s uri:%s visible:%d", targetAbility.bundleName.c_str(),
        targetAbility.name.c_str(), targetAbility.uri.c_str(), targetAbility.visible);
    HILOGD("callerType:%d accountType:%d callerUid:%d", callerInfo.callerType, accountInfo.accountType, callerInfo.uid);
    // 2.check component access permission, when the ability is not visible.
    if (!CheckComponentAccessPermission(targetAbility, callerInfo, accountInfo, want)) {
        HILOGE("CheckComponentAccessPermission denied or failed! the caller component do not have permission");
        return DMS_COMPONENT_ACCESS_PERMISSION_DENIED;
    }
    // 3.check application custom permissions.
    if (!CheckCustomPermission(targetAbility, callerInfo)) {
        HILOGE("CheckCustomPermission denied or failed! the caller component do not have permission!");
        return DMS_COMPONENT_ACCESS_PERMISSION_DENIED;
    }
    HILOGI("CheckDPermission success!!");
    return ERR_OK;
}

bool DistributedSchedPermission::getTargetAbility(const AAFwk::Want& want,
    const AppExecFwk::AbilityInfo& abilityInfo, const std::string& localDeviceId,
    AppExecFwk::AbilityInfo& targetAbility, const CallerInfo& callerInfo) const
{
    HILOGD("getTargetAbility");
    bool result = DistributedSchedAdapter::GetInstance().QueryAbilityInfo(want, targetAbility);
    if (!result) {
        HILOGE("getTargetAbility QueryAbilityInfo fail");
        return false;
    }
    return true;
}

bool DistributedSchedPermission::CheckComponentAccessPermission(const AppExecFwk::AbilityInfo& targetAbility,
    const CallerInfo& callerInfo, const AccountInfo& accountInfo, const AAFwk::Want& want) const
{
    // reject directly when in no account networking environment and target ability is not visible,
    if (!targetAbility.visible) {
        HILOGE("CheckComponentAccessPermission target ability is not visible, permission denied!");
        return false;
    }
    HILOGD("CheckComponentAccessPermission success");
    return true;
}
}
}