/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dms_token_callback.h"

#include "accesstoken_kit.h"
#include "bundle/bundle_manager_internal.h"
#include "distributed_sched_permission.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parcel_helper.h"
#include "system_ability.h"
#include "system_ability_definition.h"

using namespace OHOS::Security;

namespace OHOS {
namespace DistributedSchedule {
const std::string TAG = "DmsTokenCallback";
const std::string PERMISSION_DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";
const std::string FOUNDATION_PROCESS_NAME = "foundation";
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";

int32_t DmsTokenCallback::SendResult(OHOS::AAFwk::Want& want, int32_t callerUid,
    int32_t requestCode, uint32_t accessToken, int32_t resultCode)
{
    AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t ret = AccessToken::AccessTokenKit::GetNativeTokenInfo(IPCSkeleton::GetCallingTokenID(),
        nativeTokenInfo);
    if (ret != ERR_OK || nativeTokenInfo.processName != FOUNDATION_PROCESS_NAME) {
        HILOGE("check foundation call failed");
        return INVALID_PARAMETERS_ERR;
    }
    std::string localDeviceId;
    std::string deviceId = want.GetStringParam(DMS_SRC_NETWORK_ID);
    if (!GetLocalDeviceId(localDeviceId) || !CheckDeviceId(localDeviceId, deviceId)) {
        HILOGE("check deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IDistributedSched> remoteDms = GetRemoteDms(deviceId);
    if (remoteDms == nullptr) {
        HILOGE("get remoteDms failed");
        return INVALID_PARAMETERS_ERR;
    }
    nlohmann::json extraInfoJson;
    CallerInfo callerInfo = {.uid = callerUid, .sourceDeviceId = localDeviceId, .accessToken = accessToken,
        .extraInfoJson = extraInfoJson};
    if (!BundleManagerInternal::GetCallerAppIdFromBms(callerInfo.uid, callerInfo.callerAppId)) {
        HILOGE("GetCallerAppIdFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!BundleManagerInternal::GetBundleNameListFromBms(callerInfo.uid, callerInfo.bundleNames)) {
        HILOGE("GetBundleNameListFromBms failed");
        return INVALID_PARAMETERS_ERR;
    }
    AccountInfo accountInfo;
    ret = DistributedSchedPermission::GetInstance().GetAccountInfo(deviceId, callerInfo, accountInfo);
    if (ret != ERR_OK) {
        HILOGE("GetAccountInfo failed");
        return ret;
    }
    HILOGI("[PerformanceTest] SendResult transact begin");
    int32_t result = remoteDms->SendResultFromRemote(want, requestCode, callerInfo, accountInfo, resultCode);
    HILOGI("[PerformanceTest] SendResult transact end");
    return result;
}

bool DmsTokenCallback::GetLocalDeviceId(std::string& localDeviceId)
{
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().GetLocalDeviceId(localDeviceId)) {
        HILOGE("GetLocalDeviceId failed");
        return false;
    }
    return true;
}

bool DmsTokenCallback::CheckDeviceId(const std::string& localDeviceId, const std::string& remoteDeviceId)
{
    // remoteDeviceId must not same with localDeviceId
    if (localDeviceId.empty() || remoteDeviceId.empty() || localDeviceId == remoteDeviceId) {
        HILOGE("check deviceId failed");
        return false;
    }
    return true;
}

sptr<IDistributedSched> DmsTokenCallback::GetRemoteDms(const std::string& remoteDeviceId)
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
} // namespace Distributedschedule
} // namespace OHOS