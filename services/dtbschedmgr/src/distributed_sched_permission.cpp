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

#include "distributed_sched_permission.h"

#include "accesstoken_kit.h"
#include "bundle/bundle_manager_internal.h"
#include "caller_info.h"
#include "datetime_ex.h"
#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::Security;
namespace {
const std::string TAG = "DistributedSchedPermission";
const std::string FOUNDATION_PROCESS_NAME = "foundation";
}
IMPLEMENT_SINGLE_INSTANCE(DistributedSchedPermission);

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
    HILOGD("target ability info bundleName:%{public}s abilityName:%{public}s uri:%{private}s visible:%{public}d",
        targetAbility.bundleName.c_str(), targetAbility.name.c_str(), targetAbility.uri.c_str(),
        targetAbility.visible);
    HILOGD("callerType:%{public}d accountType:%{public}d callerUid:%{public}d AccessTokenID:%{public}u",
        callerInfo.callerType, accountInfo.accountType, callerInfo.uid, callerInfo.accessToken);
    // 2.check component access permission, when the ability is not visible.
    if (!CheckComponentAccessPermission(targetAbility, callerInfo, accountInfo, want)) {
        HILOGE("CheckComponentAccessPermission denied or failed! the caller component do not have permission");
        return DMS_COMPONENT_ACCESS_PERMISSION_DENIED;
    }
    // 3.check application custom permissions
    if (!CheckCustomPermission(targetAbility, callerInfo)) {
        HILOGE("CheckCustomPermission denied or failed! the caller component do not have permission");
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

int32_t DistributedSchedPermission::CheckGetCallerPermission(const AAFwk::Want& want, const CallerInfo& callerInfo,
    const AccountInfo& accountInfo, const std::string& localDeviceId)
{
    AppExecFwk::AbilityInfo abilityInfo;
    int32_t result = CheckDPermission(want, callerInfo, accountInfo, abilityInfo, localDeviceId);
    if (result != ERR_OK) {
        HILOGE("CheckGetCallerPermission fail, error:%{public}d", result);
        return result;
    }
    std::string appId;
    if (!BundleManagerInternal::GetCallerAppIdFromBms(want.GetElement().GetBundleName(), appId)) {
        HILOGE("CheckGetCallerPermission get appId fail");
        return CALL_PERMISSION_DENIED;
    }
    if (appId != callerInfo.callerAppId) {
        HILOGE("CheckGetCallerPermission appId is different");
        return CALL_PERMISSION_DENIED;
    }
    return ERR_OK;
}

int32_t DistributedSchedPermission::CheckPermission(uint32_t accessToken, const std::string& permissionName) const
{
    HILOGI("called.");
    if (!IsFoundationCall(IPCSkeleton::GetCallingTokenID())) {
        return DMS_PERMISSION_DENIED;
    }
    if (IsNativeCall(accessToken)) {
        return ERR_OK;
    }
    if (VerifyPermission(accessToken, permissionName)) {
        return ERR_OK;
    }
    return DMS_PERMISSION_DENIED;
}

bool DistributedSchedPermission::IsFoundationCall(uint32_t accessToken) const
{
    if (!IsNativeCall(accessToken)) {
        return false;
    }
    AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (result == ERR_OK && nativeTokenInfo.processName == FOUNDATION_PROCESS_NAME) {
        HILOGD("foundation called.");
        return true;
    }
    HILOGE("not foundation called, processName:%{private}s", nativeTokenInfo.processName.c_str());
    return false;
}

bool DistributedSchedPermission::IsNativeCall(uint32_t accessToken) const
{
    auto tokenType = AccessToken::AccessTokenKit::GetTokenTypeFlag(accessToken);
    if (tokenType == AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOGD("caller tokenType is native, verify success.");
        return true;
    }
    HILOGE("not native called.");
    return false;
}

bool DistributedSchedPermission::VerifyPermission(uint32_t accessToken, const std::string& permissionName) const
{
    int32_t result = AccessToken::AccessTokenKit::VerifyAccessToken(accessToken, permissionName);
    if (result == AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOGE("permission denied, permissionName:%{public}s", permissionName.c_str());
        return false;
    }
    HILOGD("permission matched.");
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

bool DistributedSchedPermission::CheckCustomPermission(const AppExecFwk::AbilityInfo& targetAbility,
    const CallerInfo& callerInfo) const
{
    const auto& permissions = targetAbility.permissions;
    if (permissions.empty()) {
        HILOGD("no need any permission, so granted!");
        return true;
    }
    if (callerInfo.accessToken == 0) {
        HILOGW("kernel is not support or field is not parsed, so granted!");
        return true;
    }
    int64_t begin = GetTickCount();
    uint32_t dAccessToken = AccessToken::AccessTokenKit::AllocLocalTokenID(
        callerInfo.sourceDeviceId, callerInfo.accessToken);
    HILOGI("[PerformanceTest] AllocLocalTokenID spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (dAccessToken == 0) {
        HILOGE("dAccessTokenID is invalid!");
        return false;
    }
    for (const auto& permission : permissions) {
        if (permission.empty()) {
            continue;
        }
        int32_t result = AccessToken::AccessTokenKit::VerifyAccessToken(dAccessToken, permission);
        if (result == AccessToken::PermissionState::PERMISSION_DENIED) {
            HILOGD("dAccessTokenID:%{public}u, permission:%{public}s denied!", dAccessToken, permission.c_str());
            return false;
        }
        HILOGD("dAccessTokenID:%{public}u, permission:%{public}s matched!", dAccessToken, permission.c_str());
    }
    return true;
}
}
}