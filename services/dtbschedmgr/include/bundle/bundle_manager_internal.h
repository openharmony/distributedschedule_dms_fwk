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

#ifndef OHOS_DISTRIBUTED_BUNDLE_MANANGER_INTERNAL_H
#define OHOS_DISTRIBUTED_BUNDLE_MANANGER_INTERNAL_H

#include <string>

#include "bundlemgr/bundle_mgr_interface.h"
#include "bundlemgr/bundle_mgr_proxy.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedSchedule {
class BundleManagerInternal {
    DECLARE_SINGLE_INSTANCE(BundleManagerInternal);

public:
    static bool GetCallerAppIdFromBms(int32_t callingUid, std::string& appId);
    static bool GetCallerAppIdFromBms(const std::string& bundleName, std::string& appId);
    static bool GetBundleNameListFromBms(int32_t callingUid, std::vector<std::string>& bundleNameList);
    static bool GetBundleNameListFromBms(int32_t callingUid, std::vector<std::u16string>& u16BundleNameList);
    static bool QueryAbilityInfo(const AAFwk::Want& want, AppExecFwk::AbilityInfo& abilityInfo);
    static bool IsSameAppId(const std::string& callerAppId, const std::string& targetBundleName);
    static int32_t GetLocalBundleInfo(const std::string& bundleName, AppExecFwk::BundleInfo &localBundleInfo);
    static int32_t CheckRemoteBundleInfoForContinuation(const std::string& dstDeviceId,
        const std::string& bundleName, AppExecFwk::DistributedBundleInfo& remoteBundleInfo);
    static sptr<AppExecFwk::IBundleMgr> GetBundleManager();
    static int32_t GetUidFromBms(const std::string& bundleName);
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif /* OHOS_DISTRIBUTED_BUNDLE_MANANGER_INTERNAL_H */
