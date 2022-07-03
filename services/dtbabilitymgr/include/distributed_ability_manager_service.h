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

#ifndef OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H
#define OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H

#include "continuationManager/notifier_info.h"
#include "distributed_ability_manager_stub.h"
#include "dms_notifier.h"
#include "event_handler.h"
#include "single_instance.h"
#include "system_ability.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedAbilityManagerService : public SystemAbility, public DistributedAbilityManagerStub,
    public DmsNotifier {
    DECLARE_SYSTEM_ABILITY(DistributedAbilityManagerService);

public:
    DistributedAbilityManagerService(int32_t systemAbilityId, bool runOnCreate);
    ~DistributedAbilityManagerService() = default;
    void OnStart() override;
    void OnStop() override;
    int32_t SendRequestToImpl(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override;
    void DeviceOnlineNotify(const std::string& deviceId) override;
    void DeviceOfflineNotify(const std::string& deviceId) override;
    void ProcessNotifierDied(const sptr<IRemoteObject>& notifier) override;
    void ScheduleStartDeviceManager(const sptr<IRemoteObject>& appProxy, int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr) override;
    int32_t OnDeviceConnect(int32_t token, const std::vector<ContinuationResult>& continuationResults) override;
    int32_t OnDeviceDisconnect(int32_t token, const std::vector<std::string>& deviceIds) override;
    int32_t OnDeviceCancel() override;

    int32_t Register(
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams, int32_t& token) override;
    int32_t Unregister(int32_t token) override;
    int32_t RegisterDeviceSelectionCallback(
        int32_t token, const std::string& cbType, const sptr<IRemoteObject>& notifier) override;
    int32_t UnregisterDeviceSelectionCallback(int32_t token, const std::string& cbType) override;
    int32_t UpdateConnectStatus(int32_t token, const std::string& deviceId,
        const DeviceConnectStatus& deviceConnectStatus) override;
    int32_t StartDeviceManager(
        int32_t token, const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr) override;

private:
    bool IsExceededRegisterMaxNum(uint32_t accessToken);
    bool IsContinuationModeValid(ContinuationMode continuationMode);
    bool IsConnectStatusValid(DeviceConnectStatus deviceConnectStatus);
    bool IsTokenRegistered(uint32_t accessToken, int32_t token);
    bool IfNotifierRegistered(int32_t token);
    bool IfNotifierRegistered(int32_t token, const std::string& cbType);
    bool QueryTokenByNotifier(const sptr<IRemoteObject>& notifier, int32_t& token);
    bool HandleDeviceConnect(const sptr<IRemoteObject>& notifier,
        const std::vector<ContinuationResult>& continuationResults);
    bool HandleDeviceDisconnect(const sptr<IRemoteObject>& notifier, const std::vector<std::string>& deviceIds);
    int32_t ConnectAbility(const sptr<DmsNotifier>& dmsNotifier, int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams);
    int32_t DisconnectAbility();
    bool HandleDisconnectAbility();
    void HandleNotifierDied(const sptr<IRemoteObject>& notifier);
    void HandleStartDeviceManager(int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams = nullptr);
    void HandleUpdateConnectStatus(int32_t token, std::string deviceId,
        const DeviceConnectStatus& deviceConnectStatus);

    bool InitDmsImplFunc();
    using LibHandle = void*;
    LibHandle dmsImplHandle_ = nullptr;

    using OnStartFunc = void(*)();
    using OnRemoteRequestFunc = int32_t(*)(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option);
    using DeviceOnlineNotifyFunc = void(*)(const std::string& deviceId);
    using DeviceOfflineNotifyFunc = void(*)(const std::string& deviceId);
    using ConnectAbilityFunc = int32_t(*)(const sptr<DmsNotifier>& dmsNotifier, int32_t token,
        const std::shared_ptr<ContinuationExtraParams>& continuationExtraParams);
    using DisconnectAbilityFunc = int32_t(*)();
    OnRemoteRequestFunc onRemoteRequestFunc_ = nullptr;
    DeviceOnlineNotifyFunc deviceOnlineNotifyFunc_ = nullptr;
    DeviceOfflineNotifyFunc deviceOfflineNotifyFunc_ = nullptr;
    ConnectAbilityFunc connectAbilityFunc_ = nullptr;
    DisconnectAbilityFunc disconnectAbilityFunc_ = nullptr;

    std::atomic_bool isLoaded_ = false;
    std::mutex libLoadLock_;
    std::mutex tokenMutex_;
    std::atomic<int32_t> token_ {0};
    std::mutex tokenMapMutex_;
    std::map<uint32_t, std::vector<int32_t>> tokenMap_;
    std::mutex callbackMapMutex_;
    std::map<int32_t, std::unique_ptr<NotifierInfo>> callbackMap_;
    sptr<IRemoteObject::DeathRecipient> notifierDeathRecipient_;
    std::mutex appProxyMutex_;
    sptr<IRemoteObject> appProxy_;
    std::shared_ptr<AppExecFwk::EventHandler> continuationHandler_;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_ABILITY_MANAGER_SERVICE_H
