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

#include "continuationManager/notifier_info.h"

namespace OHOS {
namespace DistributedSchedule {
sptr<IRemoteObject> NotifierInfo::GetNotifier(const std::string& cbType) const
{
    auto iter = notifierMap_.find(cbType);
    if (iter == notifierMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

void NotifierInfo::SetNotifier(const std::string& cbType, const sptr<IRemoteObject>& notifier)
{
    notifierMap_[cbType] = notifier;
}

void NotifierInfo::DeleteNotifier(const std::string& cbType)
{
    auto iter = notifierMap_.find(cbType);
    if (iter != notifierMap_.end()) {
        notifierMap_.erase(iter);
    }
}

bool NotifierInfo::QueryNotifier(const sptr<IRemoteObject>& notifier) const
{
    for (auto iter = notifierMap_.begin(); iter != notifierMap_.end(); iter++) {
        if (iter->second == notifier) {
            return true;
        }
    }
    return false;
}

bool NotifierInfo::IsNotifierMapEmpty()
{
    if (notifierMap_.empty()) {
        return true;
    }
    return false;
}

void NotifierInfo::RemoveDeathRecipient(const sptr<IRemoteObject::DeathRecipient>& notifierDeathRecipient,
    const std::string& cbType)
{
    if (cbType.empty()) {
        for (auto iter = notifierMap_.begin(); iter != notifierMap_.end(); iter++) {
            iter->second->RemoveDeathRecipient(notifierDeathRecipient);
        }
        return;
    }
    auto it = notifierMap_.find(cbType);
    if (it != notifierMap_.end()) {
        it->second->RemoveDeathRecipient(notifierDeathRecipient);
    }
}

std::shared_ptr<ConnectStatusInfo> NotifierInfo::GetConnectStatusInfo() const
{
    return connectStatusInfo_;
}

void NotifierInfo::SetConnectStatusInfo(const std::shared_ptr<ConnectStatusInfo>& connectStatusInfo)
{
    connectStatusInfo_ = connectStatusInfo;
}
} // namespace DistributedSchedule
} // namespace OHOS