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

#include "device_selection_notifier_stub.h"

#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DeviceSelectionNotifierStub";
}

const DeviceSelectionNotifierStub::HandlersMap DeviceSelectionNotifierStub::handlersMap_ =
    DeviceSelectionNotifierStub::InitHandlersMap();

DeviceSelectionNotifierStub::HandlersMap DeviceSelectionNotifierStub::InitHandlersMap()
{
    DeviceSelectionNotifierStub::HandlersMap m;
    m[EVENT_DEVICE_CONNECT] = &DeviceSelectionNotifierStub::OnDeviceConnectInner;
    m[EVENT_DEVICE_DISCONNECT] = &DeviceSelectionNotifierStub::OnDeviceDisconnectInner;
    return m;
}

int32_t DeviceSelectionNotifierStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGI("code = %{public}u", code);
    std::u16string descriptor = DeviceSelectionNotifierStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("descriptor check failed");
        return ERR_INVALID_VALUE;
    }

    auto iter = handlersMap_.find(code);
    if (iter != handlersMap_.end()) {
        auto handler = iter->second;
        if (handler != nullptr) {
            return (this->*handler)(data, reply);
        }
    }
    HILOGW("unknown request code, please check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DeviceSelectionNotifierStub::OnDeviceConnectInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    std::vector<ContinuationResult> continuationResults;
    if (!ContinuationResult::ReadContinuationResultsFromParcel(data, continuationResults)) {
        return ERR_FLATTEN_OBJECT;
    }
    OnDeviceConnect(continuationResults);
    return ERR_OK;
}

int32_t DeviceSelectionNotifierStub::OnDeviceDisconnectInner(MessageParcel& data,
    [[maybe_unused]] MessageParcel& reply)
{
    std::vector<std::string> deviceIds;
    if (!data.ReadStringVector(&deviceIds)) {
        HILOGE("read deviceIds failed.");
        return ERR_FLATTEN_OBJECT;
    }
    OnDeviceDisconnect(deviceIds);
    return ERR_OK;
}
} // namespace DistributedSchedule
} // namespace OHOS