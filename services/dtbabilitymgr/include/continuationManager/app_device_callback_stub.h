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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_APP_DEVICE_CALLBACK_STUB_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_APP_DEVICE_CALLBACK_STUB_H

#include <map>

#include "dms_notifier.h"
#include "iapp_device_callback_interface.h"
#include "iremote_stub.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedSchedule {
class AppDeviceCallbackStub : public IRemoteStub<IAppDeviceCallback> {
public:
    explicit AppDeviceCallbackStub(const sptr<DmsNotifier>& dmsNotifier);
    virtual ~AppDeviceCallbackStub() = default;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data,
        MessageParcel& reply, MessageOption& option) override;

private:
    DISALLOW_COPY_AND_MOVE(AppDeviceCallbackStub);
    int32_t OnDeviceConnect(int32_t token, const std::vector<ContinuationResult>& continuationResults) override;
    int32_t OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds) override;
    int32_t OnDeviceCancel() override;

    sptr<DmsNotifier> dmsNotifier_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_APP_DEVICE_CALLBACK_STUB_H