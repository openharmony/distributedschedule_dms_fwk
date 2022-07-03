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

#include "js_device_selection_listener.h"

#include "dtbschedmgr_log.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::AbilityRuntime;
namespace {
const std::string TAG = "JsDeviceSelectionListener";
}

void JsDeviceSelectionListener::OnDeviceConnect(const std::vector<ContinuationResult>& continuationResults)
{
    HILOGD("called.");
    CallJsMethod(EVENT_CONNECT, continuationResults);
}

void JsDeviceSelectionListener::OnDeviceDisconnect(const std::vector<std::string>& deviceIds)
{
    HILOGD("called.");
    CallJsMethod(EVENT_DISCONNECT, deviceIds);
}

void JsDeviceSelectionListener::AddCallback(const std::string& cbType, NativeValue* jsListenerObj)
{
    HILOGD("called.");
    std::unique_ptr<NativeReference> callbackRef;
    if (engine_ == nullptr) {
        HILOGE("engine_ is nullptr");
        return;
    }
    callbackRef.reset(engine_->CreateReference(jsListenerObj, 1));

    std::lock_guard<std::mutex> jsCallBackMapLock(jsCallBackMapMutex_);
    jsCallBackMap_[cbType] = std::move(callbackRef);
    HILOGD("jsCallBackMap_ cbType: %{public}s, size: %{public}u!",
        cbType.c_str(), static_cast<uint32_t>(jsCallBackMap_.size()));
}

void JsDeviceSelectionListener::RemoveCallback(const std::string& cbType)
{
    HILOGD("called.");
    std::lock_guard<std::mutex> jsCallBackMapLock(jsCallBackMapMutex_);
    jsCallBackMap_.erase(cbType);
    HILOGD("jsCallBackMap_ cbType: %{public}s, size: %{public}u!",
        cbType.c_str(), static_cast<uint32_t>(jsCallBackMap_.size()));
}

void JsDeviceSelectionListener::CallJsMethod(const std::string& methodName,
    const std::vector<ContinuationResult>& continuationResults)
{
    HILOGD("methodName = %{public}s", methodName.c_str());
    if (engine_ == nullptr) {
        HILOGE("engine_ is nullptr");
        return;
    }
    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([this, methodName, continuationResults]
            (NativeEngine &engine, AsyncTask &task, int32_t status) {
            CallJsMethodInner(methodName, continuationResults);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsDeviceSelectionListener::OnDeviceConnect",
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsDeviceSelectionListener::CallJsMethodInner(const std::string& methodName,
    const std::vector<ContinuationResult>& continuationResults)
{
    std::lock_guard<std::mutex> jsCallBackMapLock(jsCallBackMapMutex_);
    NativeValue* method = jsCallBackMap_[methodName]->Get();
    if (method == nullptr) {
        HILOGE("Failed to get %{public}s from object", methodName.c_str());
        return;
    }
    NativeValue* argv[] = { WrapContinuationResultArray(*engine_, continuationResults) };
    engine_->CallFunction(engine_->CreateUndefined(), method, argv, ArraySize(argv));
}

void JsDeviceSelectionListener::CallJsMethod(const std::string& methodName, const std::vector<std::string>& deviceIds)
{
    HILOGD("methodName = %{public}s", methodName.c_str());
    if (engine_ == nullptr) {
        HILOGE("engine_ is nullptr");
        return;
    }
    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([this, methodName, deviceIds]
            (NativeEngine &engine, AsyncTask &task, int32_t status) {
            CallJsMethodInner(methodName, deviceIds);
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsDeviceSelectionListener::OnDeviceDisconnect",
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsDeviceSelectionListener::CallJsMethodInner(const std::string& methodName,
    const std::vector<std::string>& deviceIds)
{
    std::lock_guard<std::mutex> jsCallBackMapLock(jsCallBackMapMutex_);
    NativeValue* method = jsCallBackMap_[methodName]->Get();
    if (method == nullptr) {
        HILOGE("Failed to get %{public}s from object", methodName.c_str());
        return;
    }
    NativeValue* argv[] = { WrapDeviceIdArray(*engine_, deviceIds) };
    engine_->CallFunction(engine_->CreateUndefined(), method, argv, ArraySize(argv));
}

NativeValue* JsDeviceSelectionListener::WrapContinuationResult(NativeEngine& engine,
    const ContinuationResult& continuationResult)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("id", CreateJsValue(engine, continuationResult.GetDeviceId()));
    object->SetProperty("type", CreateJsValue(engine, continuationResult.GetDeviceType()));
    object->SetProperty("name", CreateJsValue(engine, continuationResult.GetDeviceName()));
    return objValue;
}

NativeValue* JsDeviceSelectionListener::WrapContinuationResultArray(NativeEngine& engine,
    const std::vector<ContinuationResult>& continuationResults)
{
    NativeValue* arrayValue = engine.CreateArray(continuationResults.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto& continuationResult : continuationResults) {
        array->SetElement(index++, WrapContinuationResult(engine, continuationResult));
    }
    return arrayValue;
}

NativeValue* JsDeviceSelectionListener::WrapDeviceIdArray(NativeEngine& engine,
    const std::vector<std::string>& deviceIds)
{
    NativeValue* arrayValue = engine.CreateArray(deviceIds.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto& deviceId : deviceIds) {
        array->SetElement(index++, CreateJsValue(engine, deviceId));
    }
    return arrayValue;
}
}  // namespace DistributedSchedule
}  // namespace OHOS