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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H

#include "distributed_ability_manager_stub.h"
#include "deviceManager/dms_device_listener.h"
#include "single_instance.h"
#include "system_ability.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedAbilityManagerService : public SystemAbility, public DistributedAbilityManagerStub,
    public DmsDeviceListener {
DECLARE_SYSTEM_ABILITY(DistributedAbilityManagerService);

public:
    DistributedAbilityManagerService(int32_t systemAbilityId, bool runOnCreate);
    ~DistributedAbilityManagerService() = default;
    void OnStart() override;
    void OnStop() override;
    int32_t SendRequestToImpl(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override;
    void DeviceOnlineNotify(const std::string& deviceId) override;
    void DeviceOfflineNotify(const std::string& deviceId) override;

private:
    bool InitDmsImplFunc();
    using LibHandle = void*;
    LibHandle dmsImplHandle_ = nullptr;

    using OnStartFunc = void(*)();
    using OnRemoteRequestFunc = int32_t(*)(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option);
    using DeviceOnlineNotifyFunc = void(*)(const std::string& deviceId);
    using DeviceOfflineNotifyFunc = void(*)(const std::string& deviceId);
    OnRemoteRequestFunc onRemoteRequestFunc_ = nullptr;
    DeviceOnlineNotifyFunc deviceOnlineNotifyFunc_ = nullptr;
    DeviceOfflineNotifyFunc deviceOfflineNotifyFunc_ = nullptr;

    std::atomic_bool isLoaded_ = false;
    std::mutex libLoadLock_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H
