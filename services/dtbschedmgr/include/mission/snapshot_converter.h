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

#ifndef DISTRIBUTEDSCHED_SNAPSHOT_CONVERTER_H
#define DISTRIBUTEDSCHED_SNAPSHOT_CONVERTER_H

#include <string>

#include "mission_snapshot.h"
#include "snapshot.h"

namespace OHOS {
namespace DistributedSchedule {
class SnapshotConverter {
public:
    static int32_t ConvertToSnapshot(AAFwk::MissionSnapshot& missionSnapshot, Snapshot& snapshot);
    static int32_t ConvertToSnapshot(AAFwk::MissionSnapshot& missionSnapshot, std::unique_ptr<Snapshot>& snapshot);
    static int32_t ConvertToMissionSnapshot(Snapshot& snapshot,
        std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot);
};
}
}
#endif // DISTRIBUTEDSCHED_SNAPSHOT_CONVERTER_H