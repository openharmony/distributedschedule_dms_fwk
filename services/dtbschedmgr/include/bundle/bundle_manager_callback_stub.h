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

#ifndef OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_STUB_H
#define OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>

#include "bundle_manager_callback_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace DistributedSchedule {
/**
 * @class DmsBundleManagerCallbackStub
 * DmsBundleManagerCallbackStub.
 */
class DmsBundleManagerCallbackStub : public IRemoteStub<IDmsBundleManagerCallback> {
public:
    DmsBundleManagerCallbackStub();
    virtual ~DmsBundleManagerCallbackStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    DISALLOW_COPY_AND_MOVE(DmsBundleManagerCallbackStub);
    int32_t OnQueryInstallationFinishedInner(MessageParcel& data, MessageParcel& reply);
    int32_t OnQueryInstallationFinished(int32_t resultCode, std::string deviceId, int32_t missionId, int versionCode);
    using DmsBundleManagerCallbackFunc = int32_t (DmsBundleManagerCallbackStub::*)(
        MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, DmsBundleManagerCallbackFunc> memberFuncMap_;
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_STUB_H