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

#ifndef OHOS_DISTRIBUTED_HITRACE_CONSTANTS_H
#define OHOS_DISTRIBUTED_HITRACE_CONSTANTS_H

#include "hitrace_meter.h"

namespace OHOS {
namespace DistributedSchedule {
namespace TraceTag {
    constexpr uint64_t DSCHED = HITRACE_TAG_DISTRIBUTED_SCHEDULE;
}

namespace TraceValue {
    constexpr const char* START_REMOTE_ABILITY = "START_REMOTE_ABILITY";
    constexpr const char* CONNECT_REMOTE_ABILITY = "CONNECT_REMOTE_ABILITY";
    constexpr const char* DISCONNECT_REMOTE_ABILITY = "DISCONNECT_REMOTE_ABILITY";
    constexpr const char* START_REMOTE_ABILITY_BYCALL = "START_REMOTE_ABILITY_BYCALL";
    constexpr const char* RELEASE_REMOTE_ABILITY = "RELEASE_REMOTE_ABILITY";
    constexpr const char* START_CONTINUATION = "START_CONTINUATION";
    constexpr const char* REMOTE_PROCEDURE_CALL = "REMOTE_PROCEDURE_CALL";
}
} // namespace DistributedSchedule
} // namespace OHOS

#endif /* OHOS_DISTRIBUTED_HITRACE_CONSTANTS_H */