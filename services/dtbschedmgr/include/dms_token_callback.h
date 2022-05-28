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

#ifndef OHOS_DMS_TOKEN_CALLBACK_H
#define OHOS_DMS_TOKEN_CALLBACK_H

#include "distributed_sched_interface.h"
#include "hilog/log.h"
#include "system_ability_token_callback_stub.h"

namespace OHOS {
namespace DistributedSchedule {
class DmsTokenCallback : public AAFwk::SystemAbilityTokenCallbackStub {
public:
    using AccountInfo = IDistributedSched::AccountInfo;

    DmsTokenCallback() = default;
    ~DmsTokenCallback() = default;
    int32_t SendResult(OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, int32_t resultCode) override;

private:
    bool GetLocalDeviceId(std::string& localDeviceId);
    bool CheckDeviceId(const std::string& localDeviceId, const std::string& remoteDeviceId);
    sptr<IDistributedSched> GetRemoteDms(const std::string& remoteDeviceId);
};
} // namespace Distributedschedule
} // namespace OHOS
#endif // OHOS_DMS_TOKEN_CALLBACK_H