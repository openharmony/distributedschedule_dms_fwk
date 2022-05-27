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

#ifndef OHOS_DMS_FREE_INSTALL_CALLBACK_INTERFACE_H
#define OHOS_DMS_FREE_INSTALL_CALLBACK_INTERFACE_H

#include <iremote_broker.h>
#include "want.h"

namespace OHOS {
namespace DistributedSchedule {
/**
 * @class IDmsFreeInstallCallback
 * IDmsFreeInstallCallback is used to notify caller ability that free install is complete.
 */
class IDmsFreeInstallCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedSchedule.IDmsFreeInstallCallback");

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param want, the want of the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnInstallFinished(const AAFwk::Want& want, int32_t requestCode, int32_t resultCode) = 0;

protected:
    enum IDmsFreeInstallCallbackCmd {
        ON_FREE_INSTALL_DONE = 0,
        CMD_MAX,
    };
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DMS_FREE_INSTALL_CALLBACK_INTERFACE_H
