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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_CONTINUATION_MANAGER_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_CONTINUATION_MANAGER_H

#include <utility>

#include "continuation_extra_params.h"
#include "js_device_selection_listener.h"
#include "native_engine/native_engine.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedSchedule {
using JsDeviceSelectionListenerPtr = std::shared_ptr<JsDeviceSelectionListener>;

class JsContinuationManager final {
public:
    JsContinuationManager() = default;
    ~JsContinuationManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* Register(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* Unregister(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* RegisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* UnregisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* UpdateConnectStatus(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartDeviceManager(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* InitDeviceConnectStateObject(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* InitContinuationModeObject(NativeEngine* engine, NativeCallbackInfo* info);

private:
    using CallbackPair = std::pair<std::unique_ptr<NativeReference>, sptr<JsDeviceSelectionListener>>;
    NativeValue* OnRegister(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregister(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnRegisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUpdateConnectStatus(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnStartDeviceManager(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnInitDeviceConnectStateObject(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnInitContinuationModeObject(NativeEngine &engine, NativeCallbackInfo &info);
    napi_status SetEnumItem(const napi_env& env, napi_value object, const char* name, int32_t value);

    bool IsCallbackValid(NativeValue* listenerObj);
    bool IsCallbackRegistered(int32_t token, const std::string& cbType);
    bool UnWrapContinuationExtraParams(const napi_env& env, const napi_value& options,
        std::shared_ptr<ContinuationExtraParams>& continuationExtraParams);
    bool UnwrapJsonByPropertyName(const napi_env& env, const napi_value& param,
        const std::string& fieldStr, nlohmann::json& jsonObj);
    bool PraseJson(const napi_env& env, const napi_value& jsonField, const napi_value& jsProNameList,
        uint32_t jsProCount, nlohmann::json& jsonObj);

    std::mutex jsCbMapMutex_;
    std::map<int32_t, std::map<std::string, CallbackPair>> jsCbMap_;
};

NativeValue* JsContinuationManagerInit(NativeEngine* engine, NativeValue* exportObj);
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_JS_CONTINUATION_MANAGER_H