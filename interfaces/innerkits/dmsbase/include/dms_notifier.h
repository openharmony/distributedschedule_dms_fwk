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
#ifndef OHOS_DISTRIBUTED_SCHED_INTERFACES_INNERKITS_DMS_NOTIFIER_H
#define OHOS_DISTRIBUTED_SCHED_INTERFACES_INNERKITS_DMS_NOTIFIER_H

#include <string>

#include "continuation_extra_params.h"
#include "continuation_result.h"
#include "iremote_object.h"

namespace OHOS {
namespace DistributedSchedule {
class DmsNotifier : public virtual RefBase {
public:
    DmsNotifier() = default;
    virtual ~DmsNotifier() = default;

    virtual void DeviceOnlineNotify(const std::string& deviceId) = 0;
    virtual void DeviceOfflineNotify(const std::string& deviceId) = 0;
    virtual void ProcessNotifierDied(const sptr<IRemoteObject>& notifier) = 0;
    virtual void ScheduleStartDeviceManager(const sptr<IRemoteObject>& appProxy, int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr) = 0;
    virtual int32_t OnDeviceConnect(int32_t token, const std::vector<ContinuationResult>& continuationResults) = 0;
    virtual int32_t OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds) = 0;
    virtual int32_t OnDeviceCancel() = 0;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_INTERFACES_INNERKITS_DMS_NOTIFIER_H