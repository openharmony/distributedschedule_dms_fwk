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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_EXTRA_PARAMS_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_EXTRA_PARAMS_H

#include <string>
#include <vector>

#include "continuation_mode.h"
#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace DistributedSchedule {
class ContinuationExtraParams : public Parcelable {
public:
    ContinuationExtraParams() = default;
    ~ContinuationExtraParams() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ContinuationExtraParams *Unmarshalling(Parcel &parcel);

    void SetDeviceType(std::vector<std::string> deviceTypeVec);
    std::vector<std::string> GetDeviceType() const;

    void SetTargetBundle(std::string targetBundle);
    std::string GetTargetBundle() const;

    void SetDescription(std::string description);
    std::string GetDescription() const;

    void SetFilter(std::string filter);
    std::string GetFilter() const;

    void SetContinuationMode(ContinuationMode continuationMode);
    ContinuationMode GetContinuationMode() const;

    void SetAuthInfo(std::string authInfo);
    std::string GetAuthInfo() const;

private:
    std::vector<std::string> deviceTypeVec_;
    std::string targetBundle_;
    std::string description_;
    std::string filter_;
    ContinuationMode continuationMode_;
    std::string authInfo_;
};

class ContinationManagerUtils {
public:
    ContinationManagerUtils() = default;
    ~ContinationManagerUtils() = default;

    static std::vector<std::u16string> Str8VecToStr16Vec(const std::vector<std::string>& input);
    static std::vector<std::string> Str16VecToStr8Vec(const std::vector<std::u16string>& input);
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_INTERFACES_INNERKITS_CONTINUATION_EXTRA_PARAMS_H