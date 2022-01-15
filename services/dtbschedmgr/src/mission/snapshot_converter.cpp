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

#include "mission/snapshot_converter.h"

#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "SnapshotConverter";
constexpr int32_t DMS_VERSION = 200;
}

int32_t SnapshotConverter::ConvertToSnapshot(AAFwk::MissionSnapshot& missionSnapshot, Snapshot& snapshot)
{
    snapshot.version_ = DMS_VERSION;
    snapshot.orientation_ = 0;
    std::unique_ptr<Rect> contentInsets = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot.rect_ = std::move(contentInsets);
    snapshot.reducedResolution_ = true;
    snapshot.scale_ = 0.0;
    snapshot.isRealSnapshot_ = true;
    snapshot.windowingMode_ = 0;
    snapshot.systemUiVisibility_ = 0;
    snapshot.isTranslucent_ = true;
    std::unique_ptr<Rect> windowBounds = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot.windowBounds_ = std::move(windowBounds);
    std::u16string appLabel;
    snapshot.appLabel_ = appLabel;
    std::u16string abilityLabel;
    snapshot.abilityLabel_ = abilityLabel;
    std::vector<uint8_t> icon;
    snapshot.icon_ = icon;
    std::u16string secAppLabel;
    snapshot.secAppLabel_ = secAppLabel;
    std::u16string secAbilityLabel;
    snapshot.secAbilityLabel_ = secAbilityLabel;
    std::vector<uint8_t> secIcon;
    snapshot.secIcon_ = secIcon;
    std::u16string sourceDeviceTips;
    snapshot.sourceDeviceTips_ = sourceDeviceTips;
    snapshot.pixelMap_ = missionSnapshot.snapshot;
    return ERR_OK;
}

int32_t SnapshotConverter::ConvertToSnapshot(AAFwk::MissionSnapshot& missionSnapshot,
    std::unique_ptr<Snapshot>& snapshot)
{
    snapshot->version_ = DMS_VERSION;
    snapshot->orientation_ = 0;
    std::unique_ptr<Rect> contentInsets = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot->rect_ = std::move(contentInsets);
    snapshot->reducedResolution_ = true;
    snapshot->scale_ = 0.0;
    snapshot->isRealSnapshot_ = true;
    snapshot->windowingMode_ = 0;
    snapshot->systemUiVisibility_ = 0;
    snapshot->isTranslucent_ = true;
    std::unique_ptr<Rect> windowBounds = std::make_unique<Rect>(0, 0, 0, 0);
    snapshot->windowBounds_ = std::move(windowBounds);
    std::u16string appLabel;
    snapshot->appLabel_ = appLabel;
    std::u16string abilityLabel;
    snapshot->abilityLabel_ = abilityLabel;
    std::vector<uint8_t> icon;
    snapshot->icon_ = icon;
    std::u16string secAppLabel;
    snapshot->secAppLabel_ = secAppLabel;
    std::u16string secAbilityLabel;
    snapshot->secAbilityLabel_ = secAbilityLabel;
    std::vector<uint8_t> secIcon;
    snapshot->secIcon_ = secIcon;
    std::u16string sourceDeviceTips;
    snapshot->sourceDeviceTips_ = sourceDeviceTips;
    snapshot->pixelMap_ = missionSnapshot.snapshot;
    return ERR_OK;
}

int32_t SnapshotConverter::ConvertToMissionSnapshot(Snapshot& snapshot,
    std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot)
{
    missionSnapshot->snapshot = snapshot.pixelMap_;
    return ERR_OK;
}
}
}
