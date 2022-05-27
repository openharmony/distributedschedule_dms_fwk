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

#ifndef OHOS_DMS_FREE_INSTALL_CALLBACK_STUB_H
#define OHOS_DMS_FREE_INSTALL_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>

#include "dms_free_install_callback_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace DistributedSchedule {
/**
 * @class DmsFreeInstallCallbackStub
 * DmsFreeInstallCallbackStub.
 */
class DmsFreeInstallCallbackStub : public IRemoteStub<IDmsFreeInstallCallback> {
public:
    DmsFreeInstallCallbackStub();
    virtual ~DmsFreeInstallCallbackStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    DISALLOW_COPY_AND_MOVE(DmsFreeInstallCallbackStub);
    int32_t OnInstallFinishedInner(MessageParcel& data, MessageParcel& reply);
    using DmsFreeInstallCallbackFunc = int32_t (DmsFreeInstallCallbackStub::*)(
        MessageParcel& data, MessageParcel& reply);
    std::map<uint32_t, DmsFreeInstallCallbackFunc> memberFuncMap_;
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DMS_FREE_INSTALL_CALLBACK_STUB_H
