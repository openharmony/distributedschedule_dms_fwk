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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_I_DEVICE_SELECTION_NOTIFIER_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_I_DEVICE_SELECTION_NOTIFIER_H

#include <vector>

#include "continuation_result.h"
#include "iremote_broker.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string EVENT_CONNECT = "deviceConnect";
const std::string EVENT_DISCONNECT = "deviceDisconnect";
}

class IDeviceSelectionNotifier : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedSchedule.IDeviceSelectionNotifier");

    IDeviceSelectionNotifier() = default;
    virtual ~IDeviceSelectionNotifier() = default;

    virtual void OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults) = 0;
    virtual void OnDeviceDisconnect(const std::vector<std::string>& deviceIds) = 0;

    enum RequestCode {
        EVENT_DEVICE_CONNECT = 1,
        EVENT_DEVICE_DISCONNECT = 2,
    };
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_I_DEVICE_SELECTION_NOTIFIER_H