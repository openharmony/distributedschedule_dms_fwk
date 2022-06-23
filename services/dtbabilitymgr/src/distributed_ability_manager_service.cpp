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
#include "dlfcn.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedSchedule {
const std::string TAG = "DistributedAbilityManagerService";

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
} // namespace DistributedSchedule
} // namespace OHOS
