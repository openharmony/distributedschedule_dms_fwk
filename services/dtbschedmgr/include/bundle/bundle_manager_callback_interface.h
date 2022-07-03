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

#ifndef OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_INTERFACE_H
#define OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_INTERFACE_H

#include <iremote_broker.h>

namespace OHOS {
namespace DistributedSchedule {
/**
 * @class IDmsBundleManagerCallback
 * IDmsBundleManagerCallback is used to notify caller ability that free install is complete.
 */
class IDmsBundleManagerCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedSchedule.IDmsBundleManagerCallback");

protected:
    enum IDmsBundleManagerCallbackCmd {
        ON_QUERY_INSTALLATION_DONE = 1,
    };
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DISTRIBUTED_BUNDLE_MANANGER_CALLBACK_INTERFACE_H
