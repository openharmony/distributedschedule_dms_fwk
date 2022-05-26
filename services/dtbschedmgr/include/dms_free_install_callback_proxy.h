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

#ifndef OHOS_DMS_FREE_INSTALL_CALLBACK_PROXY_H
#define OHOS_DMS_FREE_INSTALL_CALLBACK_PROXY_H

#include <string>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "dms_free_install_callback_interface.h"

namespace OHOS {
namespace DistributedSchedule {
/**
 * interface for DmsFreeInstallCallbackProxy.
 */
class DmsFreeInstallCallbackProxy : public IRemoteProxy<IDmsFreeInstallCallback> {
public:
    explicit DmsFreeInstallCallbackProxy(const sptr<IRemoteObject>& impl);

    /**
     * OnInstallFinished, FreeInstall is complete.
     *
     * @param want, the want of the ability to start.
     * @param requestCode the resultCode of the ability to start.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnInstallFinished(const OHOS::AAFwk::Want& want, int32_t requestCode, int32_t resultCode) override;

private:
    void SendRequestCommon(int32_t errcode, IDmsFreeInstallCallback::IDmsFreeInstallCallbackCmd cmd);
    static inline BrokerDelegator<DmsFreeInstallCallbackProxy> delegator_;
};
}  // namespace DistributedSchedule
}  // namespace OHOS
#endif  // OHOS_DMS_FREE_INSTALL_CALLBACK_PROXY_H
