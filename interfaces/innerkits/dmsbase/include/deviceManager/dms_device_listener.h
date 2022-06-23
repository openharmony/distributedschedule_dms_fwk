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
#ifndef OHOS_DISTRIBUTED_DMS_DEVICE_LISTENER_H
#define OHOS_DISTRIBUTED_DMS_DEVICE_LISTENER_H
#include <string>

namespace OHOS {
namespace DistributedSchedule {

class DmsDeviceListener : public virtual RefBase {
public:
    DmsDeviceListener() = default;
    virtual ~DmsDeviceListener() = default;

    virtual void DeviceOnlineNotify(const std::string& deviceId) = 0;
    virtual void DeviceOfflineNotify(const std::string& deviceId) = 0;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif /* OHOS_DISTRIBUTED_DMS_DEVICE_LISTENER_H */
