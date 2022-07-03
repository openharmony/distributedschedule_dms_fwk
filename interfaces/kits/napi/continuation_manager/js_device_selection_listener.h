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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_DEVICE_SELECTION_LISTENER_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_DEVICE_SELECTION_LISTENER_H

#include <map>
#include <mutex>

#include "device_selection_notifier_stub.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace DistributedSchedule {
class JsDeviceSelectionListener : public DeviceSelectionNotifierStub {
public:
    explicit JsDeviceSelectionListener(NativeEngine* engine) : engine_(engine) {}
    virtual ~JsDeviceSelectionListener() = default;

    void OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults) override;
    void OnDeviceDisconnect(const std::vector<std::string>& deviceIds) override;

    void AddCallback(const std::string& cbType, NativeValue* jsListenerObj);
    void RemoveCallback(const std::string& cbType);

private:
    void CallJsMethod(const std::string& methodName, const std::vector<ContinuationResult>& continuationResults);
    void CallJsMethod(const std::string& methodName, const std::vector<std::string>& deviceIds);
    void CallJsMethodInner(const std::string& methodName, const std::vector<ContinuationResult>& continuationResults);
    void CallJsMethodInner(const std::string& methodName, const std::vector<std::string>& deviceIds);
    NativeValue* WrapContinuationResult(NativeEngine& engine, const ContinuationResult& continuationResult);
    NativeValue* WrapContinuationResultArray(NativeEngine& engine,
        const std::vector<ContinuationResult>& continuationResults);
    NativeValue* WrapDeviceIdArray(NativeEngine& engine, const std::vector<std::string>& deviceIds);

    NativeEngine* engine_ = nullptr;
    std::mutex jsCallBackMapMutex_;
    std::map<std::string, std::unique_ptr<NativeReference>> jsCallBackMap_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_DEVICE_SELECTION_LISTENER_H