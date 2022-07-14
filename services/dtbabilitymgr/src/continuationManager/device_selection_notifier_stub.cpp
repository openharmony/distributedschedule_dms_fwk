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

#include <iosfwd>
#include <string>
#include <vector>

#include "dtbschedmgr_log.h"
#include "errors.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "parcel_helper.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DeviceSelectionNotifierStub";
}

int32_t DeviceSelectionNotifierStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    HILOGI("code = %{public}u", code);
    std::u16string descriptor = IDeviceSelectionNotifier::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("descriptor check failed");
        return ERR_INVALID_VALUE;
    }
    switch (code) {
        case IDeviceSelectionNotifier::EVENT_DEVICE_CONNECT: {
            std::vector<ContinuationResult> continuationResults;
            if (!ContinuationResult::ReadContinuationResultsFromParcel(data, continuationResults)) {
                return ERR_FLATTEN_OBJECT;
            }
            int32_t result = OnDeviceConnectInner(continuationResults);
            return result;
        }
        case IDeviceSelectionNotifier::EVENT_DEVICE_DISCONNECT: {
            std::vector<std::string> deviceIds;
            PARCEL_READ_HELPER(data, StringVector, &deviceIds);
            int32_t result = OnDeviceDisconnectInner(deviceIds);
            return result;
        }
        default: {
            HILOGE("unknown request code, please check");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t DeviceSelectionNotifierStub::OnDeviceConnectInner(const std::vector<ContinuationResult>& continuationResults)
{
    HILOGD("called");
    OnDeviceConnect(continuationResults);
    return ERR_OK;
}

int32_t DeviceSelectionNotifierStub::OnDeviceDisconnectInner(const std::vector<std::string>& deviceIds)
{
    HILOGD("called");
    OnDeviceDisconnect(deviceIds);
    return ERR_OK;
}
} // namespace DistributedSchedule
} // namespace OHOS