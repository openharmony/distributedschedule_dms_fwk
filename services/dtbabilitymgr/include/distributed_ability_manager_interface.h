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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACE_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACE_H

#include "continuation_extra_params.h"
#include "continuation_result.h"
#include "device_connect_status.h"
#include "iremote_broker.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
constexpr int32_t VALUE_NULL = -1; // no object in parcel
constexpr int32_t VALUE_OBJECT = 1; // object exist in parcel
}
class IDistributedAbilityManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedSchedule.IDistributedAbilityManager");

    virtual int32_t Register(
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token) = 0;
    virtual int32_t Unregister(int32_t token) = 0;
    virtual int32_t RegisterDeviceSelectionCallback(
        int32_t token, const std::string& cbType, const sptr<IRemoteObject>& notifier) = 0;
    virtual int32_t UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType) = 0;
    virtual int32_t UpdateConnectStatus(int32_t token, const std::string& deviceId,
        const DeviceConnectStatus& deviceConnectStatus) = 0;
    virtual int32_t StartDeviceManager(
        int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr) = 0;

    enum {
        // request code for continuation manager
        REGISTER = 500,
        UNREGISTER = 501,
        REGISTER_DEVICE_SELECTION_CALLBACK = 502,
        UNREGISTER_DEVICE_SELECTION_CALLBACK = 503,
        UPDATE_CONNECT_STATUS = 504,
        START_DEVICE_MANAGER = 505,
    };
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACE_H