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

#include "bundle/bundle_manager_internal.h"

#include "distributed_sched_adapter.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace AccountSA;
namespace {
const std::string TAG = "BundleManagerInternal";
}
IMPLEMENT_SINGLE_INSTANCE(BundleManagerInternal);
bool BundleManagerInternal::GetCallerAppIdFromBms(int32_t callingUid, std::string& appId)
{
    std::vector<std::string> bundleNameList;
    if (!GetBundleNameListFromBms(callingUid, bundleNameList)) {
        HILOGE("GetBundleNameListFromBms failed");
        return false;
    }
    if (bundleNameList.empty()) {
        HILOGE("bundleNameList empty");
        return false;
    }
    // getting an arbitrary bundlename for they sharing a same appId, here we get the first one
    return GetCallerAppIdFromBms(bundleNameList.front(), appId);
}

bool BundleManagerInternal::GetCallerAppIdFromBms(const std::string& bundleName, std::string& appId)
{
    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("failed to get bms");
        return false;
    }
    std::vector<int> ids;
    ErrCode result = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (result != ERR_OK || ids.empty()) {
        return false;
    }
    appId = bundleMgr->GetAppIdByBundleName(bundleName, ids[0]);
    HILOGD("appId:%s", appId.c_str());
    return true;
}

bool BundleManagerInternal::GetBundleNameListFromBms(int32_t callingUid, std::vector<std::string>& bundleNameList)
{
    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("failed to get bms");
        return false;
    }
    bool result = bundleMgr->GetBundlesForUid(callingUid, bundleNameList);
    if (!result) {
        HILOGE("GetBundlesForUid failed, result: %{public}d", result);
        return false;
    }
    return result;
}

bool BundleManagerInternal::GetBundleNameListFromBms(int32_t callingUid,
    std::vector<std::u16string>& u16BundleNameList)
{
    std::vector<std::string> bundleNameList;
    if (!GetBundleNameListFromBms(callingUid, bundleNameList)) {
        HILOGE("GetBundleNameListFromBms failed");
        return false;
    }
    for (const std::string& bundleName : bundleNameList) {
        u16BundleNameList.emplace_back(Str8ToStr16(bundleName));
    }
    return true;
}

bool BundleManagerInternal::QueryAbilityInfo(const AAFwk::Want& want, AppExecFwk::AbilityInfo& abilityInfo)
{
    std::vector<int> ids;
    int32_t ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        return false;
    }
    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("failed to get bms");
        return false;
    }
    bool result = bundleMgr->QueryAbilityInfo(want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION, ids[0], abilityInfo);
    if (!result) {
        HILOGE("QueryAbilityInfo failed");
        return false;
    }
    return true;
}

bool BundleManagerInternal::QueryExtensionAbilityInfo(const AAFwk::Want& want, AppExecFwk::ExtensionAbilityInfo& extensionInfo)
{
    std::vector<int> ids;
    int32_t ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        return false;
    }
    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOGE("failed to get bms");
        return false;
    }
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bundleMgr->QueryExtensionAbilityInfos(want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT
        | AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION, ids[0], extensionInfos);
    if (extensionInfos.size() <= 0) {
        HILOGE("QueryExtensionAbilityInfo failed.");
        return false;
    }
    extensionInfo = extensionInfos.front();
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        HILOGE("ExtensionAbilityInfo is empty.");
         return false;
    }
    HILOGD("ExtensionAbilityInfo found, name=%{public}s.",
            extensionInfo.name.c_str());
    return true;
}

int32_t BundleManagerInternal::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionAbilityInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionAbilityInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionAbilityInfo.applicationInfo;
    abilityInfo.bundleName = extensionAbilityInfo.bundleName;
    abilityInfo.package = extensionAbilityInfo.moduleName;
    abilityInfo.moduleName = extensionAbilityInfo.moduleName;
    abilityInfo.name = extensionAbilityInfo.name;
    abilityInfo.srcEntrance = extensionAbilityInfo.srcEntrance;
    abilityInfo.srcPath = extensionAbilityInfo.srcEntrance;
    abilityInfo.iconPath = extensionAbilityInfo.icon;
    abilityInfo.iconId = extensionAbilityInfo.iconId;
    abilityInfo.label = extensionAbilityInfo.label;
    abilityInfo.labelId = extensionAbilityInfo.labelId;
    abilityInfo.description = extensionAbilityInfo.description;
    abilityInfo.descriptionId = extensionAbilityInfo.descriptionId;
    abilityInfo.permissions = extensionAbilityInfo.permissions;
    abilityInfo.readPermission = extensionAbilityInfo.readPermission;
    abilityInfo.writePermission = extensionAbilityInfo.writePermission;
    abilityInfo.extensionAbilityType = extensionAbilityInfo.type;
    abilityInfo.visible = extensionAbilityInfo.visible;
    abilityInfo.resourcePath = extensionAbilityInfo.resourcePath;
    abilityInfo.enabled = extensionAbilityInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionAbilityInfo.process;
    abilityInfo.metadata = extensionAbilityInfo.metadata;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    return 0;
}

bool BundleManagerInternal::IsSameAppId(const std::string& callerAppId, const std::string& targetBundleName)
{
    if (targetBundleName.empty() || callerAppId.empty()) {
        HILOGE("targetBundleName:%{public}s or callerAppId:%s is empty",
            targetBundleName.c_str(), callerAppId.c_str());
        return false;
    }
    HILOGD("callerAppId:%s", callerAppId.c_str());
    std::string calleeAppId;
    if (!GetCallerAppIdFromBms(targetBundleName, calleeAppId)) {
        HILOGE("GetCallerAppIdFromBms failed");
        return false;
    }
    HILOGD("calleeAppId:%s", calleeAppId.c_str());
    return callerAppId == calleeAppId;
}

int32_t BundleManagerInternal::GetLocalBundleInfo(const std::string& bundleName,
    AppExecFwk::BundleInfo &localBundleInfo)
{
    auto bms = GetBundleManager();
    if (bms == nullptr) {
        HILOGE("get bundle manager failed");
        return INVALID_PARAMETERS_ERR;
    }

    std::vector<int> ids;
    ErrCode ret = OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        HILOGE("QueryActiveOsAccountIds failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        localBundleInfo, ids[0])) {
        HILOGE("get local bundle info failed");
        return INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t BundleManagerInternal::CheckRemoteBundleInfoForContinuation(const std::string& dstDeviceId,
    const std::string& bundleName, AppExecFwk::DistributedBundleInfo& remoteBundleInfo)
{
    if (bundleName.empty()) {
        HILOGE("bundle name empty");
        return INVALID_PARAMETERS_ERR;
    }
    HILOGI("bundleName: %{public}s", bundleName.c_str());

    auto bms = GetBundleManager();
    if (bms == nullptr) {
        HILOGE("get bundle manager failed");
        return INVALID_PARAMETERS_ERR;
    }

    bool isInstalled = bms->GetDistributedBundleInfo(dstDeviceId, bundleName, remoteBundleInfo);
    if (isInstalled) {
        return ERR_OK;
    }

    AppExecFwk::BundleInfo localBundleInfo;
    if (GetLocalBundleInfo(bundleName, localBundleInfo) != ERR_OK) {
        HILOGE("get local bundle info failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (localBundleInfo.entryInstallationFree) {
        HILOGE("remote not installed and support free install");
        return CONTINUE_REMOTE_UNINSTALLED_SUPPORT_FREEINSTALL;
    }
    HILOGE("remote not installed and not support free install");
    return CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL;
}

sptr<AppExecFwk::IBundleMgr> BundleManagerInternal::GetBundleManager()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        return nullptr;
    }
    sptr<IRemoteObject> bmsProxy = samgrProxy->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsProxy == nullptr) {
        HILOGE("failed to get bms from samgr");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bmsProxy);
}
} // namespace DistributedSchedule
} // namespace OHOS
