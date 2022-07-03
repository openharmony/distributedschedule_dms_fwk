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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_DEVICE_CONNECT_STATUS_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_DEVICE_CONNECT_STATUS_H

namespace OHOS {
namespace DistributedSchedule {
/**
 * @enum DeviceConnectStatus
 * DeviceConnectStatus defines the device connection status.
 */
enum class DeviceConnectStatus {
    /**
     * Device initial status.
     */
    IDLE = 0,

    /**
     * Device connecting status.
     */
    CONNECTING = 1,

    /**
     * Device connected status.
     */
    CONNECTED = 2,

    /**
     * Device disconnected status.
     */
    DISCONNECTING = 3,
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_DEVICE_CONNECT_STATUS_H