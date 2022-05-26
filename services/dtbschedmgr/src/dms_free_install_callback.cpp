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

#include "dms_free_install_callback.h"

#include "dtbschedmgr_log.h"
#include "distributed_sched_service.h"
#include "distributed_sched_interface.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DmsFreeInstallCallback";
}
DmsFreeInstallCallback::DmsFreeInstallCallback(int64_t taskId, const IDistributedSched::FreeInstallInfo& info)
    : taskId_(taskId), info_(info)
{
    HILOGI("DmsFreeInstallCallback Constructors.");
}

void DmsFreeInstallCallback::OnInstallFinished(const OHOS::AAFwk::Want& want, int32_t requestCode, int32_t resultCode)
{
    HILOGI("DmsFreeInstallCallback::OnFinished");
    info_.want = want;
    info_.requestCode = requestCode;
    DistributedSchedService::GetInstance().NotifyCompleteFreeInstall(info_, taskId_, resultCode);
}
}  // namespace DistributedSchedule
}  // namespace OHOS
