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

#ifndef OHOS_DISTRIBUTED_SCHED_APP_CONNECTION_STUB_H
#define OHOS_DISTRIBUTED_SCHED_APP_CONNECTION_STUB_H

#include "ability_connect_callback_interface.h"

#include "continuation_extra_params.h"
#include "dms_notifier.h"
#include "message_parcel.h"
#include "nocopyable.h"
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class AppConnectionStub : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    AppConnectionStub(const sptr<DmsNotifier>& dmsNotifier, int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr);
    virtual ~AppConnectionStub() = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject,
        int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int32_t resultCode) override;

    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override;

private:
    DISALLOW_COPY_AND_MOVE(AppConnectionStub);
    sptr<DmsNotifier> dmsNotifier_;
    int32_t token_;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_APP_CONNECTION_STUB_H