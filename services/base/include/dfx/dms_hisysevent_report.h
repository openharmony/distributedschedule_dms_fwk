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

#ifndef OHOS_DISTRIBUTED_HISTSEVENT_REPORT_H
#define OHOS_DISTRIBUTED_HISTSEVENT_REPORT_H

#include <string>

namespace OHOS {
namespace DistributedSchedule {
namespace BehaviorEvent {
    const std::string START_REMOTE_ABILITY = "START_REMOTE_ABILITY";
    const std::string CONNECT_REMOTE_ABILITY = "CONNECT_REMOTE_ABILITY";
    const std::string DISCONNECT_REMOTE_ABILITY = "DISCONNECT_REMOTE_ABILITY";
    const std::string START_REMOTE_ABILITY_BYCALL = "START_REMOTE_ABILITY_BYCALL";
    const std::string RELEASE_REMOTE_ABILITY = "RELEASE_REMOTE_ABILITY";
    const std::string START_CONTINUATION = "START_CONTINUATION";
}

namespace FaultEvent {
    const std::string START_REMOTE_ABILITY = "START_ABILITY_FAILED";
    const std::string CONNECT_REMOTE_ABILITY = "CONNECT_ABILITY_FAILED";
    const std::string DISCONNECT_REMOTE_ABILITY = "DISCONNECT_ABILITY_FAILED";
    const std::string START_REMOTE_ABILITY_BYCALL = "START_ABILITY_BYCALL_FAILED";
    const std::string RELEASE_REMOTE_ABILITY = "RELEASE_ABILITY_FAILED";
    const std::string START_CONTINUATION = "START_CONTINUATION_FAILED";
}

namespace EventErrorType {
    const std::string GET_BUNDLE_MGR_FAILED = "GET_BUNDLE_MGR_FAILED";
    const std::string GET_ABILITY_MGR_FAILED = "GET_ABILITY_MGR_FAILED";
}

namespace EventCallingType {
    const std::string LOCAL = "LOCAL";
    const std::string REMOTE = "REMOTE";
}

struct BehaviorEventParam {
    std::string callingType;
    std::string eventName;
    int32_t eventResult;
    std::string bundleName = "";
    std::string abilityName = "";
    int32_t callingAppUid = -1;
};

class DmsHiSysEventReport {
public:
    static int ReportBehaviorEvent(const BehaviorEventParam& param);
    static int ReportFaultEvent(const std::string& eventName, const std::string& errorType);
};
} // namespace DistributedSchedule
} // namespace OHOS

#endif /* OHOS_DISTRIBUTED_HISTSEVENT_REPORT_H */