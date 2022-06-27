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

#include "dfx/dms_hisysevent_report.h"

#include "dtbschedmgr_log.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::HiviewDFX;
namespace {
    const std::string TAG = "DmsHiSysEventReport";
    const std::string DOMAIN_NAME = std::string(HiSysEvent::Domain::DISTRIBUTED_SCHEDULE);

    const std::string KEY_CALLING_APP_UID = "CALLING_APP_UID";
    const std::string KEY_CALLING_PID = "CALLING_PID";
    const std::string KEY_CALLING_TYPE = "CALLING_TYPE";
    const std::string KEY_CALLING_UID = "CALLING_UID";
    const std::string KEY_ERROR_TYPE = "ERROR_TYPE";

    const std::string KEY_RESULT = "RESULT";
    const std::string KEY_TARGET_ABILITY = "TARGET_ABILITY";
    const std::string KEY_TARGET_BUNDLE = "TARGET_BUNDLE";
}

int DmsHiSysEventReport::ReportBehaviorEvent(const BehaviorEventParam& param)
{
    int result = HiSysEvent::Write(DOMAIN_NAME, param.eventName, HiSysEvent::EventType::BEHAVIOR,
        KEY_CALLING_TYPE, param.callingType,
        KEY_CALLING_UID, IPCSkeleton::GetCallingUid(),
        KEY_CALLING_PID, IPCSkeleton::GetCallingPid(),
        KEY_CALLING_APP_UID, param.callingAppUid,
        KEY_TARGET_BUNDLE, param.bundleName,
        KEY_TARGET_ABILITY, param.abilityName,
        KEY_RESULT, param.eventResult);
    if (result != 0) {
        HILOGE("hisysevent report failed! ret %{public}d.", result);
    }
    return result;
}

int DmsHiSysEventReport::ReportFaultEvent(const std::string& eventName, const std::string& errorType)
{
    int result = HiSysEvent::Write(DOMAIN_NAME, eventName, HiSysEvent::EventType::FAULT,
        KEY_ERROR_TYPE, errorType);
    if (result != 0) {
        HILOGE("hisysevent report failed! ret %{public}d.", result);
    }
    return result;
}
}
}