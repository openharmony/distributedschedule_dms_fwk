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

#include "js_continuation_manager.h"

#include <memory>

#include "device_connect_status.h"
#include "distributed_ability_manager_client.h"
#include "dtbschedmgr_log.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace DistributedSchedule {
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
namespace {
const std::string TAG = "JsContinuationManager";
constexpr int32_t ERR_NOT_OK = -1;
constexpr int32_t ARG_COUNT_ONE = 1;
constexpr int32_t ARG_COUNT_TWO = 2;
constexpr int32_t ARG_COUNT_THREE = 3;
}

void JsContinuationManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOGI("JsContinuationManager::Finalizer is called");
    JsContinuationManager* jsContinuationManager = static_cast<JsContinuationManager*>(data);
    if (jsContinuationManager != nullptr) {
        delete jsContinuationManager;
        jsContinuationManager = nullptr;
    }
}

NativeValue* JsContinuationManager::Register(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnRegister(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::Unregister(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUnregister(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::RegisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnRegisterDeviceSelectionCallback(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::UnregisterDeviceSelectionCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsContinuationManager* me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUnregisterDeviceSelectionCallback(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::UpdateConnectStatus(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnUpdateConnectStatus(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::StartDeviceManager(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnStartDeviceManager(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::InitDeviceConnectStateObject(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnInitDeviceConnectStateObject(*engine, *info) : nullptr;
}

NativeValue *JsContinuationManager::InitContinuationModeObject(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsContinuationManager *me = CheckParamsAndGetThis<JsContinuationManager>(engine, info);
    return (me != nullptr) ? me->OnInitContinuationModeObject(*engine, *info) : nullptr;
}

NativeValue* JsContinuationManager::OnRegister(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    int32_t errCode = 0;
    decltype(info.argc) unwrapArgc = 0;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams = std::make_shared<ContinuationExtraParams>();
    if (info.argc > 0 && info.argv[0]->TypeOf() == NATIVE_OBJECT) {
        HILOGI("register options is used.");
        if (!UnWrapContinuationExtraParams(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), continuationExtraParams)) {
            HILOGE("Parse continuationExtraParams failed");
            errCode = ERR_NOT_OK;
        }
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [continuationExtraParams, unwrapArgc, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        int32_t token = -1;
        int32_t ret = (unwrapArgc == 0) ? DistributedAbilityManagerClient::GetInstance().Register(nullptr, token) :
            DistributedAbilityManagerClient::GetInstance().Register(continuationExtraParams, token);
        if (ret == ERR_OK) {
            task.Resolve(engine, engine.CreateNumber(token));
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "Register failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsContinuationManager::OnRegister",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsContinuationManager::OnUnregister(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    int32_t errCode = 0;
    if (info.argc == 0) {
        HILOGE("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOGE("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    AsyncTask::CompleteCallback complete =
        [token, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        int32_t ret = DistributedAbilityManagerClient::GetInstance().Unregister(token);
        if (ret == ERR_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "Unregister failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= ARG_COUNT_ONE) ? nullptr : info.argv[ARG_COUNT_ONE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsContinuationManager::OnUnregister",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsContinuationManager::OnRegisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    if (info.argc != ARG_COUNT_THREE) {
        HILOGE("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOGE("Parse cbType failed");
        return engine.CreateUndefined();
    }
    int32_t token = -1;
    if (!ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], token)) {
        HILOGE("Parse token failed");
        return engine.CreateUndefined();
    }
    NativeValue* jsListenerObj = info.argv[ARG_COUNT_TWO];
    if (!IsCallbackValid(jsListenerObj)) {
        return engine.CreateUndefined();
    }
    {
        std::lock_guard<std::mutex> jsCbMapLock(jsCbMapMutex_);
        if (IsCallbackRegistered(token, cbType)) {
            return engine.CreateUndefined();
        }
        std::unique_ptr<NativeReference> callbackRef;
        callbackRef.reset(engine.CreateReference(jsListenerObj, 1));
        sptr<JsDeviceSelectionListener> deviceSelectionListener = new JsDeviceSelectionListener(&engine);
        if (deviceSelectionListener == nullptr) {
            HILOGE("deviceSelectionListener is nullptr");
            return engine.CreateUndefined();
        }
        int32_t ret = DistributedAbilityManagerClient::GetInstance().RegisterDeviceSelectionCallback(
            token, cbType, deviceSelectionListener);
        if (ret == ERR_OK) {
            deviceSelectionListener->AddCallback(cbType, jsListenerObj);
            CallbackPair callbackPair = std::make_pair(std::move(callbackRef), deviceSelectionListener);
            jsCbMap_[token][cbType] = std::move(callbackPair); // move assignment
            HILOGI("RegisterDeviceSelectionListener success");
        } else {
            deviceSelectionListener = nullptr;
            HILOGE("RegisterDeviceSelectionListener failed");
        }
    }
    return engine.CreateUndefined();
}

NativeValue* JsContinuationManager::OnUnregisterDeviceSelectionCallback(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    if (info.argc != ARG_COUNT_TWO) {
        HILOGE("Params not match");
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOGE("Parse cbType failed");
        return engine.CreateUndefined();
    }
    if (cbType != EVENT_CONNECT && cbType != EVENT_DISCONNECT) {
        HILOGE("type: %{public}s not support!", cbType.c_str());
        return engine.CreateUndefined();
    }
    int32_t token = -1;
    if (!ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], token)) {
        HILOGE("Parse token failed");
        return engine.CreateUndefined();
    }
    {
        std::lock_guard<std::mutex> jsCbMapLock(jsCbMapMutex_);
        if (!IsCallbackRegistered(token, cbType)) {
            return engine.CreateUndefined();
        }
        CallbackPair& callbackPair = jsCbMap_[token][cbType];
        auto& listener = callbackPair.second;
        int32_t ret = DistributedAbilityManagerClient::GetInstance().UnregisterDeviceSelectionCallback(token, cbType);
        if (ret == ERR_OK) {
            listener->RemoveCallback(cbType);
            jsCbMap_[token].erase(cbType);
            if (jsCbMap_[token].empty()) {
                jsCbMap_.erase(token);
            }
            HILOGI("UnregisterDeviceSelectionCallback success");
        } else {
            HILOGE("UnregisterDeviceSelectionCallback failed");
        }
    }
    return engine.CreateUndefined();
}

NativeValue *JsContinuationManager::OnUpdateConnectStatus(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    int32_t errCode = 0;
    if (info.argc < ARG_COUNT_THREE) {
        HILOGE("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOGE("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    std::string deviceId;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[ARG_COUNT_ONE], deviceId)) {
        HILOGE("Parse deviceId failed");
        errCode = ERR_NOT_OK;
    }
    DeviceConnectStatus deviceConnectStatus = DeviceConnectStatus::IDLE;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[ARG_COUNT_TWO], deviceConnectStatus)) {
        HILOGE("Parse device connect status failed");
        errCode = ERR_NOT_OK;
    }
    AsyncTask::CompleteCallback complete =
        [token, deviceId, deviceConnectStatus, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        int32_t ret = DistributedAbilityManagerClient::GetInstance().UpdateConnectStatus(
            token, deviceId, deviceConnectStatus);
        if (ret == ERR_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "UpdateConnectStatus failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= ARG_COUNT_THREE) ? nullptr : info.argv[ARG_COUNT_THREE];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsContinuationManager::OnUpdateConnectStatus",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsContinuationManager::OnStartDeviceManager(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOGD("called.");
    int32_t errCode = 0;
    if (info.argc < ARG_COUNT_ONE) {
        HILOGE("Params not match");
        errCode = ERR_NOT_OK;
    }
    int32_t token = -1;
    if (!errCode && !ConvertFromJsValue(engine, info.argv[0], token)) {
        HILOGE("Parse token failed");
        errCode = ERR_NOT_OK;
    }
    decltype(info.argc) unwrapArgc = ARG_COUNT_ONE;
    std::shared_ptr<ContinuationExtraParams> continuationExtraParams = std::make_shared<ContinuationExtraParams>();
    if (info.argc > ARG_COUNT_ONE && info.argv[ARG_COUNT_ONE]->TypeOf() == NATIVE_OBJECT) {
        HILOGI("startDeviceManager options is used.");
        if (!UnWrapContinuationExtraParams(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[ARG_COUNT_ONE]), continuationExtraParams)) {
            HILOGE("Parse continuationExtraParams failed");
            errCode = ERR_NOT_OK;
        }
        unwrapArgc++;
    }
    AsyncTask::CompleteCallback complete =
        [token, continuationExtraParams, unwrapArgc, errCode](NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (errCode != 0) {
            task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
            return;
        }
        int32_t ret = (unwrapArgc == ARG_COUNT_ONE) ?
            DistributedAbilityManagerClient::GetInstance().StartDeviceManager(token) :
            DistributedAbilityManagerClient::GetInstance().StartDeviceManager(token, continuationExtraParams);
        if (ret == ERR_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, ret, "StartDeviceManager failed."));
        }
    };

    NativeValue* lastParam = (info.argc <= unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsContinuationManager::OnStartDeviceManager",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsContinuationManager::OnInitDeviceConnectStateObject(NativeEngine &engine, NativeCallbackInfo &info)
{
    napi_value object;
    napi_env env = reinterpret_cast<napi_env>(&engine);
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "IDLE",
        static_cast<int32_t>(DeviceConnectStatus::IDLE)));
    NAPI_CALL(env, SetEnumItem(env, object, "CONNECTING",
        static_cast<int32_t>(DeviceConnectStatus::CONNECTING)));
    NAPI_CALL(env, SetEnumItem(env, object, "CONNECTED",
        static_cast<int32_t>(DeviceConnectStatus::CONNECTED)));
    NAPI_CALL(env, SetEnumItem(env, object, "DISCONNECTING",
        static_cast<int32_t>(DeviceConnectStatus::DISCONNECTING)));

    return reinterpret_cast<NativeValue*>(object);
}

NativeValue *JsContinuationManager::OnInitContinuationModeObject(NativeEngine &engine, NativeCallbackInfo &info)
{
    napi_value object;
    napi_env env = reinterpret_cast<napi_env>(&engine);
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "COLLABORATION_SINGLE",
        static_cast<int32_t>(ContinuationMode::COLLABORATION_SINGLE)));
    NAPI_CALL(env, SetEnumItem(env, object, "COLLABORATION_MULTIPLE",
        static_cast<int32_t>(ContinuationMode::COLLABORATION_MUTIPLE)));

    return reinterpret_cast<NativeValue*>(object);
}

napi_status JsContinuationManager::SetEnumItem(const napi_env& env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

bool JsContinuationManager::IsCallbackValid(NativeValue* listenerObj)
{
    if (listenerObj == nullptr) {
        HILOGE("listenerObj is nullptr");
        return false;
    }
    if (!listenerObj->IsCallable()) {
        HILOGE("listenerObj is not callable");
        return false;
    }
    return true;
}

bool JsContinuationManager::IsCallbackRegistered(int32_t token, const std::string& cbType)
{
    if (jsCbMap_.empty() || jsCbMap_.find(token) == jsCbMap_.end()) {
        HILOGE("token %{public}d not registered callback!", token);
        return false;
    }
    if (jsCbMap_[token].empty() || jsCbMap_[token].find(cbType) == jsCbMap_[token].end()) {
        HILOGE("cbType %{public}s not registered callback!", cbType.c_str());
        return false;
    }
    HILOGI("callback already registered, token: %{public}d, cbType %{public}s", token, cbType.c_str());
    return true;
}

bool JsContinuationManager::UnWrapContinuationExtraParams(const napi_env& env, const napi_value& options,
    std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    HILOGD("called.");
    if (!IsTypeForNapiValue(env, options, napi_object)) {
        HILOGE("options is invalid.");
        return false;
    }
    std::vector<std::string> deviceTypes;
    if (UnwrapStringArrayByPropertyName(env, options, "deviceType", deviceTypes)) {
        continuationExtraParams->SetDeviceType(deviceTypes);
    }
    std::string targetBundle("");
    if (UnwrapStringByPropertyName(env, options, "targetBundle", targetBundle)) {
        continuationExtraParams->SetTargetBundle(targetBundle);
    }
    std::string description("");
    if (UnwrapStringByPropertyName(env, options, "description", description)) {
        continuationExtraParams->SetDescription(description);
    }
    nlohmann::json filter;
    if (!UnwrapJsonByPropertyName(env, options, "filter", filter)) {
        return false;
    }
    continuationExtraParams->SetFilter(filter.dump());
    int32_t continuationMode = 0;
    if (UnwrapInt32ByPropertyName(env, options, "continuationMode", continuationMode)) {
        continuationExtraParams->SetContinuationMode(static_cast<ContinuationMode>(continuationMode));
    }
    nlohmann::json authInfo;
    if (UnwrapJsonByPropertyName(env, options, "authInfo", authInfo)) {
        continuationExtraParams->SetAuthInfo(authInfo.dump());
    }
    return true;
}

bool JsContinuationManager::UnwrapJsonByPropertyName(const napi_env& env, const napi_value& param,
    const std::string& field, nlohmann::json& jsonObject)
{
    HILOGD("called.");
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOGE("param is invalid.");
        return false;
    }
    napi_value jsonField = nullptr;
    napi_get_named_property(env, param, field.c_str(), &jsonField);
    napi_valuetype jsonFieldType = napi_undefined;
    napi_typeof(env, jsonField, &jsonFieldType);
    if (jsonFieldType != napi_object && jsonFieldType != napi_undefined) {
        HILOGE("field: %{public}s is invalid json.", field.c_str());
        return false;
    }
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_get_property_names(env, jsonField, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);
    if (!PraseJson(env, jsonField, jsProNameList, jsProCount, jsonObject)) {
        HILOGE("PraseJson failed.");
        return false;
    }
    return true;
}

bool JsContinuationManager::PraseJson(const napi_env& env, const napi_value& jsonField,
    const napi_value& jsProNameList, uint32_t jsProCount, nlohmann::json& jsonObject)
{
    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    napi_valuetype jsValueType = napi_undefined;
    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = UnwrapStringFromJS(env, jsProName);
        napi_get_named_property(env, jsonField, strProName.c_str(), &jsProValue);
        napi_typeof(env, jsProValue, &jsValueType);
        switch (jsValueType) {
            case napi_string: {
                std::string elementValue = UnwrapStringFromJS(env, jsProValue);
                HILOGI("Property name=%{public}s, string, value=%{public}s", strProName.c_str(), elementValue.c_str());
                jsonObject[strProName] = elementValue;
                break;
            }
            case napi_boolean: {
                bool elementValue = false;
                napi_get_value_bool(env, jsProValue, &elementValue);
                HILOGI("Property name=%{public}s, boolean, value=%{public}d.", strProName.c_str(), elementValue);
                jsonObject[strProName] = elementValue;
                break;
            }
            case napi_number: {
                int32_t elementValue = 0;
                if (napi_get_value_int32(env, jsProValue, &elementValue) != napi_ok) {
                    HILOGE("Property name=%{public}s, Property int32_t parse error", strProName.c_str());
                } else {
                    HILOGI("Property name=%{public}s, number, value=%{public}d.", strProName.c_str(), elementValue);
                    jsonObject[strProName] = elementValue;
                }
                break;
            }
            default: {
                HILOGE("Property name=%{public}s, value type not support.", strProName.c_str());
                break;
            }
        }
    }
    return true;
}

NativeValue* JsContinuationManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOGD("called.");
    if (engine == nullptr || exportObj == nullptr) {
        HILOGE("Invalid input parameters");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOGE("convertNativeValueTo result is nullptr.");
        return nullptr;
    }

    JsContinuationManager* jsContinuationManager = new JsContinuationManager();
    object->SetNativePointer(jsContinuationManager, JsContinuationManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "register", JsContinuationManager::Register);
    BindNativeFunction(*engine, *object, "unregister", JsContinuationManager::Unregister);
    BindNativeFunction(*engine, *object, "on", JsContinuationManager::RegisterDeviceSelectionCallback);
    BindNativeFunction(*engine, *object, "off", JsContinuationManager::UnregisterDeviceSelectionCallback);
    BindNativeFunction(*engine, *object, "updateConnectStatus", JsContinuationManager::UpdateConnectStatus);
    BindNativeFunction(*engine, *object, "startDeviceManager", JsContinuationManager::StartDeviceManager);
    BindNativeProperty(*object, "DeviceConnectState", JsContinuationManager::InitDeviceConnectStateObject);
    BindNativeProperty(*object, "ContinuationMode", JsContinuationManager::InitContinuationModeObject);

    return engine->CreateUndefined();
}
}  // namespace DistributedSchedule
}  // namespace OHOS