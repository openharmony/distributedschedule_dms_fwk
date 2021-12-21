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

#ifndef DISTRIBUTED_DATA_CHANGE_LISTENER_H
#define DISTRIBUTED_DATA_CHANGE_LISTENER_H

#include "kvstore_observer.h"
#include "change_notification.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedDataChangeListener : public DistributedKv::KvStoreObserver {
public:
    DistributedDataChangeListener() = default;
    ~DistributedDataChangeListener() override = default;

    void OnChange(const DistributedKv::ChangeNotification &appChangeNotification) override;
};
} // DistributedSchedule
} // OHOS

#endif // DISTRIBUTED_DATA_CHANGE_LISTENER_H
