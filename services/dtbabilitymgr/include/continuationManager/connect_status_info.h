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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_CONNECT_STATUS_INFO_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_CONNECT_STATUS_INFO_H

#include <iosfwd>
#include <string>

#include "device_connect_status.h"

namespace OHOS {
namespace DistributedSchedule {
class ConnectStatusInfo {
public:
    ConnectStatusInfo(const std::string& deviceId, const DeviceConnectStatus& deviceConnectStatus)
        : deviceId_(deviceId), deviceConnectStatus_(deviceConnectStatus) {}
    ~ConnectStatusInfo() = default;

    std::string GetDeviceId() const;
    void SetDeviceId(const std::string& deviceId);
    DeviceConnectStatus GetDeviceConnectStatus() const;
    void SetDeviceConnectStatus(const DeviceConnectStatus& deviceConnectStatus);
private:
    std::string deviceId_;
    DeviceConnectStatus deviceConnectStatus_ = DeviceConnectStatus::IDLE;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_CONNECT_STATUS_INFO_H