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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_RESULT_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_RESULT_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace DistributedSchedule {
class ContinuationResult : public Parcelable {
public:
    ContinuationResult() = default;
    ~ContinuationResult() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ContinuationResult *Unmarshalling(Parcel &parcel);

    void SetDeviceId(std::string deviceId);
    std::string GetDeviceId() const;

    void SetDeviceType(std::string deviceType);
    std::string GetDeviceType() const;

    void SetDeviceName(std::string deviceName);
    std::string GetDeviceName() const;

    static bool ReadContinuationResultsFromParcel(Parcel& parcel,
        std::vector<ContinuationResult>& continuationResults);
    static bool WriteContinuationResultsToParcel(Parcel& parcel,
        const std::vector<ContinuationResult>& continuationResults);
private:
    std::string deviceId_;
    std::string deviceType_;
    std::string deviceName_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_RESULT_H