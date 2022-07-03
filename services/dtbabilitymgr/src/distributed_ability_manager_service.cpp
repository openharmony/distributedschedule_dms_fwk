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

#include "distributed_ability_manager_service.h"

#include <chrono>
#include <thread>

#include "adapter/dnetwork_adapter.h"
#include "continuationManager/app_device_callback_stub.h"
#include "continuationManager/device_selection_notifier_proxy.h"
#include "continuationManager/notifier_death_recipient.h"
#include "dlfcn.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "parcel_helper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
namespace {
const std::string TAG = "DistributedAbilityManagerService";
const std::u16string HIPLAY_PANEL_INTERFACE_TOKEN = u"ohos.hiplay.panel";
const std::string TOKEN_KEY = "distributedsched.continuationmanager.token";
const std::string DEFAULT_TOKEN_VALUE = "0";
constexpr int32_t MAX_TOKEN_NUM = 100000000;
constexpr int32_t MAX_REGISTER_NUM = 600;
constexpr int32_t START_DEVICE_MANAGER_CODE = 1;
constexpr int32_t UPDATE_CONNECT_STATUS_CODE = 2;
}

REGISTER_SYSTEM_ABILITY_BY_ID(DistributedAbilityManagerService, DISTRIBUTED_SCHED_SA_ID, true);

DistributedAbilityManagerService::DistributedAbilityManagerService(
    int32_t systemAbilityId, bool runOnCreate) : SystemAbility(systemAbilityId, runOnCreate)
{
}

void DistributedAbilityManagerService::OnStart()
{
    HILOGI("begin");
    DnetworkAdapter::GetInstance()->Init();
    if (!DtbschedmgrDeviceInfoStorage::GetInstance().Init(this)) {
        HILOGW("Init DtbschedmgrDeviceInfoStorage init failed.");
    }
    {
        std::lock_guard<std::mutex> tokenLock(tokenMutex_);
        std::string tokenStr = system::GetParameter(TOKEN_KEY, DEFAULT_TOKEN_VALUE);
        if (!tokenStr.empty()) {
            token_.store(std::stoi(tokenStr));
        }
    }
    notifierDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new NotifierDeathRecipient(this));
    if (continuationHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("continuationManager");
        continuationHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    Publish(this);
}

void DistributedAbilityManagerService::OnStop()
{
    HILOGI("begin");
}

bool DistributedAbilityManagerService::InitDmsImplFunc()
{
    std::lock_guard<std::mutex> lock(libLoadLock_);
    if (isLoaded_) {
        return true;
    }
    HILOGI("begin");
    dmsImplHandle_ = dlopen("libdistributedschedsvr.z.so", RTLD_NOW);
    if (dmsImplHandle_ == nullptr) {
        HILOGE("dlopen libdistributedschedsvr failed with errno:%s!", dlerror());
        return false;
    }

    onRemoteRequestFunc_ = reinterpret_cast<OnRemoteRequestFunc>(dlsym(dmsImplHandle_, "OnRemoteRequest"));
    if (onRemoteRequestFunc_ == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get OnRemoteRequest function error");
        return false;
    }
    deviceOnlineNotifyFunc_ = reinterpret_cast<DeviceOnlineNotifyFunc>(dlsym(dmsImplHandle_, "DeviceOnlineNotify"));
    if (deviceOnlineNotifyFunc_ == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get DeviceOnlineNotify function error");
        return false;
    }
    deviceOfflineNotifyFunc_ = reinterpret_cast<DeviceOfflineNotifyFunc>(dlsym(dmsImplHandle_, "DeviceOfflineNotify"));
    if (deviceOfflineNotifyFunc_ == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get DeviceOfflineNotify function error");
        return false;
    }
    connectAbilityFunc_ = reinterpret_cast<ConnectAbilityFunc>(dlsym(dmsImplHandle_, "ConnectAbility"));
    if (connectAbilityFunc_ == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get ConnectAbility function error");
        return false;
    }
    disconnectAbilityFunc_ = reinterpret_cast<DisconnectAbilityFunc>(dlsym(dmsImplHandle_, "DisconnectAbility"));
    if (disconnectAbilityFunc_ == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get DisconnectAbility function error");
        return false;
    }
    OnStartFunc onStartFunc = reinterpret_cast<OnStartFunc>(dlsym(dmsImplHandle_, "OnStart"));
    if (onStartFunc == nullptr) {
        dlclose(dmsImplHandle_);
        HILOGE("get OnStart function error");
        return false;
    }
    onStartFunc();
    isLoaded_ = true;
    HILOGI("end");
    return true;
}

int32_t DistributedAbilityManagerService::SendRequestToImpl(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    if (InitDmsImplFunc()) {
        return onRemoteRequestFunc_(code, data, reply, option);
    }
    return INVALID_PARAMETERS_ERR;
}

void DistributedAbilityManagerService::DeviceOnlineNotify(const std::string& deviceId)
{
    if (InitDmsImplFunc()) {
        deviceOnlineNotifyFunc_(deviceId);
    }
}

void DistributedAbilityManagerService::DeviceOfflineNotify(const std::string& deviceId)
{
    if (InitDmsImplFunc()) {
        deviceOfflineNotifyFunc_(deviceId);
    }
}

int32_t DistributedAbilityManagerService::ConnectAbility(const sptr<DmsNotifier>& dmsNotifier, int32_t token,
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    if (InitDmsImplFunc()) {
        return connectAbilityFunc_(dmsNotifier, token, continuationExtraParams);
    }
    return INVALID_PARAMETERS_ERR;
}

int32_t DistributedAbilityManagerService::DisconnectAbility()
{
    if (InitDmsImplFunc()) {
        return disconnectAbilityFunc_();
    }
    return INVALID_PARAMETERS_ERR;
}

int32_t DistributedAbilityManagerService::Register(
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token)
{
    HILOGD("called");
    if (continuationExtraParams != nullptr) {
        ContinuationMode continuationMode = continuationExtraParams->GetContinuationMode();
        if (!IsContinuationModeValid(continuationMode)) {
            return INVALID_CONTINUATION_MODE;
        }
    }
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (IsExceededRegisterMaxNum(accessToken)) {
        return REGISTER_EXCEED_MAX_TIMES;
    }
    int32_t tToken = -1;
    {
        std::lock_guard<std::mutex> tokenLock(tokenMutex_);
        tToken = token_.load();
        if (++tToken > MAX_TOKEN_NUM) {
            tToken = 1;
        }
        token_.store(tToken);
        // save at parameters
        system::SetParameter(TOKEN_KEY, std::to_string(tToken));
    }
    // update tokenMap_
    {
        std::lock_guard<std::mutex> tokenMapLock(tokenMapMutex_);
        tokenMap_[accessToken].emplace_back(tToken);
    }
    token = tToken;
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::Unregister(int32_t token)
{
    HILOGD("called");
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenRegistered(accessToken, token)) {
        return TOKEN_HAS_NOT_REGISTERED;
    }
    // remove death recipient and update callbackMap_ by token
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        if (IfNotifierRegistered(token)) {
            auto& notifierMap = callbackMap_[token]->GetNotifierMap();
            for (auto iter = notifierMap.begin(); iter != notifierMap.end(); iter++) {
                iter->second->RemoveDeathRecipient(notifierDeathRecipient_);
            }
            callbackMap_.erase(token);
        }
    }
    // update tokenMap_ by token
    {
        std::lock_guard<std::mutex> tokenMapLock(tokenMapMutex_);
        for (auto iter = tokenMap_.begin(); iter != tokenMap_.end();) {
            iter->second.erase(std::remove_if(iter->second.begin(), iter->second.end(),
                [token](int32_t n) { return n == token; }), iter->second.end());
            if (iter->second.empty()) {
                tokenMap_.erase(iter++);
                break;
            } else {
                iter++;
            }
        }
    }
    // disconnect to app when third-party app called unregister
    (void)HandleDisconnectAbility();
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::RegisterDeviceSelectionCallback(
    int32_t token, const std::string& cbType, const sptr<IRemoteObject>& notifier)
{
    HILOGD("called");
    if (cbType != EVENT_CONNECT && cbType != EVENT_DISCONNECT) {
        HILOGE("type: %{public}s not support!", cbType.c_str());
        return UNKNOWN_CALLBACK_TYPE;
    }
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenRegistered(accessToken, token)) {
        return TOKEN_HAS_NOT_REGISTERED;
    }
    if (IfNotifierRegistered(token, cbType)) {
        return CALLBACK_HAS_REGISTERED;
    }
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        auto iter = callbackMap_.find(token);
        if (iter != callbackMap_.end()) { // registered at least once
            if (iter->second == nullptr) {
                return ERR_NULL_OBJECT;
            }
            iter->second->SetNotifier(cbType, notifier);
        } else { // never registered
            std::unique_ptr<NotifierInfo> notifierInfo = std::make_unique<NotifierInfo>();
            notifierInfo->SetNotifier(cbType, notifier);
            callbackMap_[token] = std::move(notifierInfo);
            notifier->AddDeathRecipient(notifierDeathRecipient_);
        }
        HILOGD("token %{public}d register success", token);
    }
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType)
{
    HILOGD("called");
    if (cbType != EVENT_CONNECT && cbType != EVENT_DISCONNECT) {
        HILOGE("type: %{public}s not support!", cbType.c_str());
        return UNKNOWN_CALLBACK_TYPE;
    }
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenRegistered(accessToken, token)) {
        return TOKEN_HAS_NOT_REGISTERED;
    }
    if (!IfNotifierRegistered(token, cbType)) {
        return CALLBACK_HAS_NOT_REGISTERED;
    }
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        auto iter = callbackMap_.find(token);
        if (iter != callbackMap_.end()) {
            auto& notifierMap = iter->second->GetNotifierMap();
            auto it = notifierMap.find(cbType);
            if (it != notifierMap.end()) {
                it->second->RemoveDeathRecipient(notifierDeathRecipient_);
                notifierMap.erase(it);
            }
        }
    }
    HILOGD("token %{public}d unregister success", token);
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::UpdateConnectStatus(int32_t token, const std::string& deviceId,
    const DeviceConnectStatus& deviceConnectStatus)
{
    HILOGD("called");
    if (deviceId.empty()) {
        HILOGE("deviceId is empty");
        return ERR_NULL_OBJECT;
    }
    if (!IsConnectStatusValid(deviceConnectStatus)) {
        return INVALID_CONNECT_STATUS;
    }
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenRegistered(accessToken, token)) {
        return TOKEN_HAS_NOT_REGISTERED;
    }
    if (!IfNotifierRegistered(token)) {
        return CALLBACK_HAS_NOT_REGISTERED;
    }
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        std::shared_ptr<ConnectStatusInfo> connectStatusInfo =
            std::make_shared<ConnectStatusInfo>(deviceId, deviceConnectStatus);
        callbackMap_[token]->SetConnectStatusInfo(connectStatusInfo);
    }
    // sendRequest status(token, connectStatusInfo) to app by app proxy when appProxy_ is not null.
    {
        std::lock_guard<std::mutex> appProxyLock(appProxyMutex_);
        if (appProxy_ != nullptr) {
            HandleUpdateConnectStatus(token, deviceId, deviceConnectStatus);
        }
    }
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::StartDeviceManager(
    int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    HILOGD("called");
    if (continuationExtraParams != nullptr) {
        ContinuationMode continuationMode = continuationExtraParams->GetContinuationMode();
        if (!IsContinuationModeValid(continuationMode)) {
            return INVALID_CONTINUATION_MODE;
        }
    }
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!IsTokenRegistered(accessToken, token)) {
        return TOKEN_HAS_NOT_REGISTERED;
    }
    if (!IfNotifierRegistered(token)) {
        return CALLBACK_HAS_NOT_REGISTERED;
    }
    // 1. connect to app and get the app proxy if appProxy_ is null, otherwise start device manager directly.
    {
        std::lock_guard<std::mutex> appProxyLock(appProxyMutex_);
        if (appProxy_ != nullptr) {
            HandleStartDeviceManager(token, continuationExtraParams);
            return ERR_OK;
        }
    }
    int32_t errCode = ConnectAbility(this, token, continuationExtraParams);
    if (errCode != ERR_OK) {
        HILOGE("token %{public}d connect to app failed", token);
        return CONNECT_ABILITY_FAILED;
    }
    // 2. sendRequest data(token, filter, dmsStub, connectStatusInfo) to app by app proxy when connect callback.
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::OnDeviceConnect(int32_t token,
    const std::vector<ContinuationResult>& continuationResults)
{
    // device connect callback to napi
    if (!HandleDisconnectAbility()) {
        return DISCONNECT_ABILITY_FAILED;
    }
    if (!IfNotifierRegistered(token, EVENT_CONNECT)) {
        return CALLBACK_HAS_NOT_REGISTERED;
    }
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        auto& notifierMap = callbackMap_[token]->GetNotifierMap();
        auto it = notifierMap.find(EVENT_CONNECT);
        if (it != notifierMap.end()) {
            if (!HandleDeviceConnect(it->second, continuationResults)) {
                return INVALID_PARAMETERS_ERR;
            }
        }
    }
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds)
{
    // device disconnect callback to napi
    if (!HandleDisconnectAbility()) {
        return DISCONNECT_ABILITY_FAILED;
    }
    if (!IfNotifierRegistered(token, EVENT_DISCONNECT)) {
        return CALLBACK_HAS_NOT_REGISTERED;
    }
    {
        std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
        auto& notifierMap = callbackMap_[token]->GetNotifierMap();
        auto it = notifierMap.find(EVENT_DISCONNECT);
        if (it != notifierMap.end()) {
            if (!HandleDeviceDisconnect(it->second, deviceIds)) {
                return INVALID_PARAMETERS_ERR;
            }
        }
    }
    return ERR_OK;
}

int32_t DistributedAbilityManagerService::OnDeviceCancel()
{
    // disconnect to app when app close the window.
    if (!HandleDisconnectAbility()) {
        return DISCONNECT_ABILITY_FAILED;
    }
    return ERR_OK;
}

bool DistributedAbilityManagerService::HandleDisconnectAbility()
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return false;
    }
    auto func = [this]() {
        HILOGD("HandleDisconnectAbility called.");
        int32_t errCode = DisconnectAbility();
        if (errCode != ERR_OK) {
            return;
        }
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return false;
    }
    return true;
}

bool DistributedAbilityManagerService::IsExceededRegisterMaxNum(uint32_t accessToken)
{
    std::lock_guard<std::mutex> tokenMapLock(tokenMapMutex_);
    auto iter = tokenMap_.find(accessToken);
    if (iter != tokenMap_.end() && iter->second.size() >= MAX_REGISTER_NUM) {
        HILOGE("accessToken %{public}u registered too much times", accessToken);
        return true;
    }
    return false;
}

bool DistributedAbilityManagerService::IsContinuationModeValid(ContinuationMode continuationMode)
{
    if (static_cast<int32_t>(continuationMode) < static_cast<int32_t>(ContinuationMode::COLLABORATION_SINGLE) ||
        static_cast<int32_t>(continuationMode) > static_cast<int32_t>(ContinuationMode::COLLABORATION_MUTIPLE)) {
        HILOGE("continuationMode is invalid");
        return false;
    }
    return true;
}

bool DistributedAbilityManagerService::IsConnectStatusValid(DeviceConnectStatus deviceConnectStatus)
{
    if (static_cast<int32_t>(deviceConnectStatus) < static_cast<int32_t>(DeviceConnectStatus::IDLE) ||
        static_cast<int32_t>(deviceConnectStatus) > static_cast<int32_t>(DeviceConnectStatus::DISCONNECTING)) {
        HILOGE("deviceConnectStatus is invalid");
        return false;
    }
    return true;
}

bool DistributedAbilityManagerService::IsTokenRegistered(uint32_t accessToken, int32_t token)
{
    std::lock_guard<std::mutex> tokenMapLock(tokenMapMutex_);
    auto iter = tokenMap_.find(accessToken);
    if (iter == tokenMap_.end()) {
        HILOGE("accessToken %{public}u has not registered", accessToken);
        return false;
    }
    for (auto it = iter->second.begin(); it != iter->second.end(); it++) {
        if (*it == token) {
            return true;
        }
    }
    HILOGE("token %{public}d has not registered", token);
    return false;
}

bool DistributedAbilityManagerService::IfNotifierRegistered(int32_t token)
{
    // must be in callbackMapLock scope
    auto iter = callbackMap_.find(token);
    if (iter == callbackMap_.end()) {
        HILOGE("never registered, token: %{public}d ", token);
        return false;
    }
    if (iter->second == nullptr) {
        HILOGE("notifierInfo is nullptr, token: %{public}d ", token);
        return false;
    }
    return true;
}

bool DistributedAbilityManagerService::IfNotifierRegistered(int32_t token, const std::string& cbType)
{
    std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
    if (!IfNotifierRegistered(token)) {
        return false;
    }
    auto& notifierMap = callbackMap_[token]->GetNotifierMap();
    auto it = notifierMap.find(cbType);
    if (it != notifierMap.end()) {
        HILOGD("token: %{public}d cbType:%{public}s has already registered", token, cbType.c_str());
        return true;
    }
    HILOGE("token: %{public}d cbType:%{public}s has not registered", token, cbType.c_str());
    return false;
}

bool DistributedAbilityManagerService::HandleDeviceConnect(const sptr<IRemoteObject>& notifier,
    const std::vector<ContinuationResult>& continuationResults)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return false;
    }
    auto func = [notifier, continuationResults]() {
        HILOGD("HandleDeviceConnect called.");
        auto proxy = std::make_unique<DeviceSelectionNotifierProxy>(notifier);
        proxy->OnDeviceConnect(continuationResults);
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return false;
    }
    return true;
}

bool DistributedAbilityManagerService::HandleDeviceDisconnect(const sptr<IRemoteObject>& notifier,
    const std::vector<std::string>& deviceIds)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return false;
    }
    auto func = [notifier, deviceIds]() {
        HILOGD("HandleDeviceDisconnect called.");
        auto proxy = std::make_unique<DeviceSelectionNotifierProxy>(notifier);
        proxy->OnDeviceDisconnect(deviceIds);
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return false;
    }
    return true;
}

void DistributedAbilityManagerService::ScheduleStartDeviceManager(const sptr<IRemoteObject>& appProxy, int32_t token,
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    std::lock_guard<std::mutex> appProxyLock(appProxyMutex_);
    appProxy_ = appProxy;
    if (appProxy_ == nullptr) {
        return;
    }
    HandleStartDeviceManager(token, continuationExtraParams);
    return;
}

void DistributedAbilityManagerService::HandleStartDeviceManager(int32_t token,
    const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return;
    }
    auto func = [this, token, continuationExtraParams]() {
        HILOGD("HandleStartDeviceManager called.");
        MessageParcel data;
        if (!data.WriteInterfaceToken(HIPLAY_PANEL_INTERFACE_TOKEN)) {
            HILOGE("WriteInterfaceToken failed");
            return;
        }
        PARCEL_WRITE_HELPER_NORET(data, Int32, token);
        if (continuationExtraParams == nullptr) {
            PARCEL_WRITE_HELPER_NORET(data, Int32, VALUE_NULL);
        } else {
            PARCEL_WRITE_HELPER_NORET(data, Int32, VALUE_OBJECT);
            PARCEL_WRITE_HELPER_NORET(data, Parcelable, continuationExtraParams.get());
        }
        sptr<AppDeviceCallbackStub> callback = new AppDeviceCallbackStub(this);
        PARCEL_WRITE_HELPER_NORET(data, RemoteObject, callback);
        // query whether the connect status needs to be send
        {
            std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
            if (IfNotifierRegistered(token)) {
                std::shared_ptr<ConnectStatusInfo> connectStatusInfo = callbackMap_[token]->GetConnectStatusInfo();
                if (connectStatusInfo == nullptr) {
                    PARCEL_WRITE_HELPER_NORET(data, Int32, VALUE_NULL);
                } else {
                    PARCEL_WRITE_HELPER_NORET(data, Int32, VALUE_OBJECT);
                    // use u16string, because send to app
                    PARCEL_WRITE_HELPER_NORET(data, String16, Str8ToStr16(connectStatusInfo->GetDeviceId()));
                    PARCEL_WRITE_HELPER_NORET(data, Int32,
                        static_cast<int32_t>(connectStatusInfo->GetDeviceConnectStatus()));
                }
            } else {
                PARCEL_WRITE_HELPER_NORET(data, Int32, VALUE_NULL);
            }
        }
        MessageParcel reply;
        MessageOption option;
        int32_t result = appProxy_->SendRequest(START_DEVICE_MANAGER_CODE, data, reply, option);
        HILOGD("result is %{public}d", result);
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return;
    }
}

void DistributedAbilityManagerService::HandleUpdateConnectStatus(int32_t token, std::string deviceId,
    const DeviceConnectStatus& deviceConnectStatus)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return;
    }
    auto func = [this, token, deviceId, deviceConnectStatus]() {
        HILOGD("HandleUpdateConnectStatus called.");
        MessageParcel data;
        if (!data.WriteInterfaceToken(HIPLAY_PANEL_INTERFACE_TOKEN)) {
            HILOGE("WriteInterfaceToken failed");
            return;
        }
        PARCEL_WRITE_HELPER_NORET(data, Int32, token);
        // use u16string, because send to app
        PARCEL_WRITE_HELPER_NORET(data, String16, Str8ToStr16(deviceId));
        PARCEL_WRITE_HELPER_NORET(data, Int32, static_cast<int32_t>(deviceConnectStatus));
        MessageParcel reply;
        MessageOption option;
        int32_t result = appProxy_->SendRequest(UPDATE_CONNECT_STATUS_CODE, data, reply, option);
        HILOGD("result is %{public}d", result);
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return;
    }
}

bool DistributedAbilityManagerService::QueryTokenByNotifier(const sptr<IRemoteObject>& notifier, int32_t& token)
{
    std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end(); iter++) {
        if (iter->second == nullptr) {
            return false;
        }
        auto& notifierMap = iter->second->GetNotifierMap();
        for (auto it = notifierMap.begin(); it != notifierMap.end(); it++) {
            if (it->second == notifier) {
                token = iter->first;
                return true;
            }
        }
    }
    return false;
}

void DistributedAbilityManagerService::ProcessNotifierDied(const sptr<IRemoteObject>& notifier)
{
    // update cache when third-party app died
    if (notifier == nullptr) {
        return;
    }
    HandleNotifierDied(notifier);
}

void DistributedAbilityManagerService::HandleNotifierDied(const sptr<IRemoteObject>& notifier)
{
    if (continuationHandler_ == nullptr) {
        HILOGE("continuationHandler_ is nullptr");
        return;
    }
    auto func = [this, notifier] () {
        HILOGD("HandleNotifierDied called.");
        // query token in callbackMap_ by notifier
        int32_t token = -1;
        if (!QueryTokenByNotifier(notifier, token)) {
            HILOGE("QueryTokenByNotifier failed");
            return;
        }
        // remove death recipient and update callbackMap_ by token
        {
            std::lock_guard<std::mutex> callbackMapLock(callbackMapMutex_);
            if (IfNotifierRegistered(token)) {
                auto& notifierMap = callbackMap_[token]->GetNotifierMap();
                for (auto iter = notifierMap.begin(); iter != notifierMap.end(); iter++) {
                    iter->second->RemoveDeathRecipient(notifierDeathRecipient_);
                }
                callbackMap_.erase(token);
            }
        }
        // update tokenMap_ by token
        {
            std::lock_guard<std::mutex> tokenMapLock(tokenMapMutex_);
            for (auto iter = tokenMap_.begin(); iter != tokenMap_.end();) {
                iter->second.erase(std::remove_if(iter->second.begin(), iter->second.end(),
                    [token](int32_t n) { return n == token; }), iter->second.end());
                if (iter->second.empty()) {
                    tokenMap_.erase(iter++);
                    break;
                } else {
                    iter++;
                }
            }
        }
        // disconnect to app when third-party app died
        (void)HandleDisconnectAbility();
    };
    if (!continuationHandler_->PostTask(func)) {
        HILOGE("continuationHandler_ postTask failed");
        return;
    }
}
} // namespace DistributedSchedule
} // namespace OHOS