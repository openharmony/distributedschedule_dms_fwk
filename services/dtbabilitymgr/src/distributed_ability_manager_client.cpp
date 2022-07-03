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
#include "distributed_ability_manager_client.h"

#include "distributed_ability_manager_proxy.h"
#include "dtbschedmgr_log.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedAbilityManagerClient";
}

IMPLEMENT_SINGLE_INSTANCE(DistributedAbilityManagerClient);

sptr<IDistributedAbilityManager> DistributedAbilityManagerClient::GetDmsProxy()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("get samgr failed.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObj = samgrProxy->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    if (remoteObj == nullptr) {
        HILOGE("get dms SA failed.");
        return nullptr;
    }
    return iface_cast<IDistributedAbilityManager>(remoteObj);
}

int32_t DistributedAbilityManagerClient::Register(
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->Register(continuationExtraParams, token);
}

int32_t DistributedAbilityManagerClient::Unregister(int32_t token)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->Unregister(token);
}

int32_t DistributedAbilityManagerClient::RegisterDeviceSelectionCallback(int32_t token, const std::string& cbType,
    const sptr<DeviceSelectionNotifierStub>& notifier)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->RegisterDeviceSelectionCallback(token, cbType, notifier);
}

int32_t DistributedAbilityManagerClient::UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->UnregisterDeviceSelectionCallback(token, cbType);
}

int32_t DistributedAbilityManagerClient::UpdateConnectStatus(int32_t token, const std::string& deviceId,
    const DeviceConnectStatus& deviceConnectStatus)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->UpdateConnectStatus(token, deviceId, deviceConnectStatus);
}

int32_t DistributedAbilityManagerClient::StartDeviceManager(
    int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    HILOGD("called.");
    sptr<IDistributedAbilityManager> dmsProxy = GetDmsProxy();
    if (dmsProxy == nullptr) {
        HILOGE("dmsProxy is nullptr");
        return ERR_NULL_OBJECT;
    }
    return dmsProxy->StartDeviceManager(token, continuationExtraParams);
}
}  // namespace DistributedSchedule
}  // namespace OHOS