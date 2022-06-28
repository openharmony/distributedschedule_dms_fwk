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

#ifndef OHOS_DISTRIBUTED_SCHED_PROXY_H
#define OHOS_DISTRIBUTED_SCHED_PROXY_H

#include "distributed_sched_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedSchedProxy : public IRemoteProxy<IDistributedSched> {
public:
    explicit DistributedSchedProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDistributedSched>(impl) {}
    ~DistributedSchedProxy() {}
    int32_t Register(
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token) override;
    int32_t Unregister(int32_t token) override;
    int32_t RegisterDeviceSelectionCallback(
        int32_t token, const std::string& cbType, const sptr<IRemoteObject>& notifier) override;
    int32_t UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType) override;
    int32_t UpdateConnectStatus(int32_t token, const std::string& deviceId,
        const DeviceConnectStatus& deviceConnectStatus) override;
    int32_t StartDeviceManager(
        int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr) override;
    int32_t OnDeviceConnect(int32_t token, const std::vector<ContinuationResult>& continuationResults) override;
    int32_t OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds) override;
    int32_t OnDeviceCancel(int32_t token) override;
private:
    static inline BrokerDelegator<DistributedSchedProxy> delegator_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_PROXY_H
