/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "dtbschedmgr_device_info_storage.h"

#include <chrono>
#include <thread>

#include "distributed_device_node_listener.h"
#include "dtbschedmgr_log.h"

#include "ipc_object_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace std;
namespace OHOS {
namespace DistributedSchedule {
using namespace std::chrono_literals;

namespace {
constexpr int32_t RETRY_TIMES = 30;
constexpr int32_t CONNECT_SOFTBUS_RETRY_TIMES = 60;
const std::string TAG = "DtbschedmgrDeviceInfoStorage";
}

IMPLEMENT_SINGLE_INSTANCE(DtbschedmgrDeviceInfoStorage);

bool DtbschedmgrDeviceInfoStorage::Init()
{
    if (initHandler_ == nullptr) {
        auto deviceInfoStorageRunner = AppExecFwk::EventRunner::Create("DmsDeviceInfoStorageManager");
        initHandler_ = std::make_shared<AppExecFwk::EventHandler>(deviceInfoStorageRunner);
    }

    auto func = [this]() {
        HILOGI("begin connect softbus");
        for (int32_t retryTimes = 0; retryTimes <= CONNECT_SOFTBUS_RETRY_TIMES; retryTimes++) {
            if (ConnectSoftbus()) {
                return;
            }
            HILOGE("retry connect softbus %{public}d times", retryTimes);
            std::this_thread::sleep_for(1s);
        }
        HILOGE("connect softbus 60times * 30s, error!!");
    };
    if (!initHandler_->PostTask(func)) {
        HILOGE("Init handler postTask failed");
        return false;
    }
    return true;
}

bool DtbschedmgrDeviceInfoStorage::Init(sptr<DmsNotifier> listener)
{
    listener_ = listener;
    return Init();
}

bool DtbschedmgrDeviceInfoStorage::ConnectSoftbus()
{
    ClearAllDevices();
    bool isReady = WaitForDnetworkReady();
    if (!isReady) {
        HILOGE("ConnectSoftbus wait Dnetwork failed!");
        return false;
    }
    std::shared_ptr<DnetworkAdapter> dnetworkAdapter = DnetworkAdapter::GetInstance();
    if (dnetworkAdapter == nullptr) {
        HILOGE("DnetworkAdapter::GetInstance() null");
        return false;
    }
    if (!InitNetworkIdManager(dnetworkAdapter)) {
        HILOGE("InitNetworkIdManager failed");
        return false;
    }
    HILOGI("ConnectSoftbus success");
    return true;
}

bool DtbschedmgrDeviceInfoStorage::InitNetworkIdManager(std::shared_ptr<DnetworkAdapter> dnetworkAdapter)
{
    if (networkIdMgrHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("DmsNetworkIdManager");
        networkIdMgrHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }

    deviceNodeListener_ = std::make_shared<DistributedDeviceNodeListener>();
    if (!dnetworkAdapter->AddDeviceChangeListener(deviceNodeListener_)) {
        deviceNodeListener_ = nullptr;
        HILOGE("AddDeviceChangeListener failed!");
        return false;
    }
    return true;
}

void DtbschedmgrDeviceInfoStorage::Stop()
{
    ClearAllDevices();
    if (deviceNodeListener_ != nullptr) {
        DnetworkAdapter::GetInstance()->RemoveDeviceChangeListener(deviceNodeListener_);
        deviceNodeListener_ = nullptr;
    }
}

bool DtbschedmgrDeviceInfoStorage::WaitForDnetworkReady()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOGE("WaitForDnetworkReady failed to get samgr!");
        return false;
    }
    int32_t retryTimeout = RETRY_TIMES;
    do {
        auto dnetwork = samgr->CheckSystemAbility(SOFTBUS_SERVER_SA_ID);
        if (dnetwork != nullptr) {
            IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(dnetwork.GetRefPtr());
            // make sure the proxy is not dead
            if (proxy != nullptr && !proxy->IsObjectDead()) {
                return true;
            }
        }
        HILOGI("Waiting for dnentwork service...");
        std::this_thread::sleep_for(1s);
        if (--retryTimeout <= 0) {
            HILOGI("Waiting for dnentwork service timeout(30)s");
            return false;
        }
    } while (true);
    return false;
}

void DtbschedmgrDeviceInfoStorage::RegisterUuidNetworkIdMap(const std::string& networkId)
{
    std::string uuid = DnetworkAdapter::GetInstance()->GetUuidByNetworkId(networkId);
    {
        std::lock_guard<std::mutex> autoLock(uuidNetworkIdLock_);
        uuidNetworkIdMap_[uuid] = networkId;
    }
}

void DtbschedmgrDeviceInfoStorage::UnregisterUuidNetworkIdMap(const std::string& networkId)
{
    std::string uuid = DnetworkAdapter::GetInstance()->GetUuidByNetworkId(networkId);
    {
        std::lock_guard<std::mutex> autoLock(uuidNetworkIdLock_);
        uuidNetworkIdMap_.erase(uuid);
    }
}

void DtbschedmgrDeviceInfoStorage::GetDeviceIdSet(std::set<std::string>& deviceIdSet)
{
    deviceIdSet.clear();
    lock_guard<mutex> autoLock(deviceLock_);
    for (const auto& device : remoteDevices_) {
        deviceIdSet.emplace(device.first);
    }
}

bool DtbschedmgrDeviceInfoStorage::GetLocalDeviceId(std::string& deviceId)
{
    return GetLocalDeviceFromDnet(deviceId);
}

bool DtbschedmgrDeviceInfoStorage::GetLocalDeviceFromDnet(std::string& deviceId)
{
    auto dnetworkAdapter = DnetworkAdapter::GetInstance();
    if (dnetworkAdapter == nullptr) {
        HILOGE("GetLocalDeviceFromDnet dnetworkAdapter null");
        return false;
    }
    std::shared_ptr<NodeBasicInfo> localNode = dnetworkAdapter->GetLocalBasicInfo();
    if (localNode != nullptr) {
        deviceId = localNode->networkId;
        HILOGI("get local deviceId from DnetworkAdapter, deviceId = %{public}s",
            DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
        return true;
    }
    HILOGE("GetLocalDeviceFromDnet localNode null");
    return false;
}

void DtbschedmgrDeviceInfoStorage::ClearAllDevices()
{
    lock_guard<mutex> autoLock(deviceLock_);
    remoteDevices_.clear();
}

std::shared_ptr<DmsDeviceInfo> DtbschedmgrDeviceInfoStorage::GetDeviceInfoById(const string& deviceId)
{
    lock_guard<mutex> autoLock(deviceLock_);
    auto iter = remoteDevices_.find(deviceId);
    if (iter == remoteDevices_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::string DtbschedmgrDeviceInfoStorage::GetUuidByNetworkId(const std::string& networkId)
{
    if (networkId.empty()) {
        HILOGW("GetUuidByNetworkId networkId empty!");
        return "";
    }
    {
        std::lock_guard<std::mutex> autoLock(uuidNetworkIdLock_);
        auto iter = uuidNetworkIdMap_.begin();
        while (iter != uuidNetworkIdMap_.end()) {
            if (iter->second == networkId) {
                return iter->first;
            } else {
                ++iter;
            }
        }
    }
    std::string uuid = DnetworkAdapter::GetInstance()->GetUuidByNetworkId(networkId);
    return uuid;
}

std::string DtbschedmgrDeviceInfoStorage::GetNetworkIdByUuid(const std::string& uuid)
{
    if (uuid.empty()) {
        HILOGW("GetNetworkIdByUuid uuid empty!");
        return "";
    }
    {
        std::lock_guard<std::mutex> autoLock(uuidNetworkIdLock_);
        auto iter = uuidNetworkIdMap_.find(uuid);
        if (iter != uuidNetworkIdMap_.end()) {
            return iter->second;
        }
        return "";
    }
}
void DtbschedmgrDeviceInfoStorage::DeviceOnlineNotify(const std::shared_ptr<DmsDeviceInfo> devInfo)
{
    if (devInfo == nullptr) {
        HILOGE("DeviceOnlineNotify devInfo null");
        return;
    }
    std::string devId = devInfo->GetDeviceId();
    HILOGI("deviceId = %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(devId).c_str());

    if (networkIdMgrHandler_ == nullptr) {
        HILOGE("networkIdMgrHandler null");
        return;
    }
    auto nodeOnline = [this, devInfo]() {
        std::string deviceId = devInfo->GetDeviceId();
        RegisterUuidNetworkIdMap(deviceId);
        std::string uuid = GetUuidByNetworkId(deviceId);
        HILOGI("deviceId = %{public}s, uuid = %{public}s, deviceName = %{public}s",
            DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str(),
            DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), devInfo->GetDeviceName().c_str());
        {
            lock_guard<mutex> autoLock(deviceLock_);
            remoteDevices_[deviceId] = devInfo;
        }
        if (listener_!= nullptr) {
            listener_->DeviceOnlineNotify(deviceId);
        }
    };
    if (!networkIdMgrHandler_->PostTask(nodeOnline)) {
        HILOGE("DeviceOnlineNotify handler postTask failed");
    }
}

void DtbschedmgrDeviceInfoStorage::DeviceOfflineNotify(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOGE("DeviceOfflineNotify deviceId empty");
        return;
    }
    HILOGI("DeviceOfflineNotify deviceId = %{public}s",
        DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str());
    if (networkIdMgrHandler_ == nullptr) {
        HILOGE("DeviceOfflineNotify networkIdMgrHandler null");
        return;
    }
    auto nodeOffline = [this, deviceId]() {
        std::string uuid = GetUuidByNetworkId(deviceId);
        HILOGI("DeviceOfflineNotify process deviceId = %{public}s, uuid = %{public}s",
            DnetworkAdapter::AnonymizeDeviceId(deviceId).c_str(), DnetworkAdapter::AnonymizeDeviceId(uuid).c_str());
        if (listener_!= nullptr) {
            listener_->DeviceOfflineNotify(deviceId);
        }
        UnregisterUuidNetworkIdMap(deviceId);
        lock_guard<mutex> autoLock(deviceLock_);
        remoteDevices_.erase(deviceId);
    };
    if (!networkIdMgrHandler_->PostTask(nodeOffline)) {
        HILOGE("DeviceOfflineNotify handler postTask failed");
    }
}

void DtbschedmgrDeviceInfoStorage::OnDeviceInfoChanged(const std::string& deviceId, DeviceInfoType type)
{
    HILOGI("OnDeviceInfoChanged called");
}

void DnetServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGI("OnRemoteDied dnetwork service died");
    DtbschedmgrDeviceInfoStorage::GetInstance().Init();
}
}
}
