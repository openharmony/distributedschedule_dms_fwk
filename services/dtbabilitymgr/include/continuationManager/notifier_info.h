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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_NOTIFIER_INFO_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_NOTIFIER_INFO_H

#include <map>

#include "connect_status_info.h"
#include "iremote_object.h"

namespace OHOS {
namespace DistributedSchedule {
class NotifierInfo {
public:
    NotifierInfo() = default;
    ~NotifierInfo() = default;

    using NotifierMap = std::map<std::string, sptr<IRemoteObject>>;
    NotifierMap& GetNotifierMap();
    sptr<IRemoteObject> GetNotifier(const std::string& cbType);
    void SetNotifier(const std::string& cbType, const sptr<IRemoteObject>& notifier);
    std::shared_ptr<ConnectStatusInfo> GetConnectStatusInfo() const;
    void SetConnectStatusInfo(const std::shared_ptr<ConnectStatusInfo>& connectStatusInfo);
private:
    NotifierMap notifierMap_;
    std::shared_ptr<ConnectStatusInfo> connectStatusInfo_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_NOTIFIER_INFO_H