/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_SCHED_SERVICE_H
#define OHOS_DISTRIBUTED_SCHED_SERVICE_H

#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>

#include "ability_connect_callback_interface.h"
#include "system_ability.h"
#include "distributed_sched_stub.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedSchedule {
enum class ServiceRunningState {
    STATE_NO_START,
    STATE_RUNNING
};
enum class TargetComponent {
    HARMONY_COMPONENT,
    DEF_COMPONENT,
};

class DistributedSchedService : public SystemAbility, public DistributedSchedStub {
DECLARE_SYSTEM_ABILITY(DistributedSchedService);

DECLARE_SINGLE_INSTANCE_BASE(DistributedSchedService);

public:
    ~DistributedSchedService() = default;
    void OnStart() override;
    void OnStop() override;
    int32_t StartRemoteAbility(const OHOS::AAFwk::Want& userWant, const OHOS::AppExecFwk::AbilityInfo& abilityInfo,
        int32_t requestCode) override;
    int32_t StartAbilityFromRemote(const OHOS::AAFwk::Want& userWant,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode, const CallerInfo& callerInfo,
        const AccountInfo& accountInfo) override;
private:
    DistributedSchedService();
    bool Init();
    sptr<IDistributedSched> GetRemoteDms(const std::string& remoteDeviceId);
    bool registerToService_ = false;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SCHED_SERVICE_H
