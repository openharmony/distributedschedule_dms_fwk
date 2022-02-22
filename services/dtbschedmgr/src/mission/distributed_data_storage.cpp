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

#include "mission/distributed_data_storage.h"

#include <thread>
#include <unistd.h>
#include "datetime_ex.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"
#include "ipc_object_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "mission/distributed_sched_mission_manager.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::DistributedKv;

namespace OHOS {
namespace DistributedSchedule {
namespace {
const string TAG = "DistributedDataStorage";
const string APP_ID = "DistributedSchedule";
const string STORE_ID = "SnapshotInfoDataStorage";
constexpr int32_t RETRY_TIMES_WAIT_KV_DATA = 30;
constexpr int32_t RETRY_TIMES_GET_KVSTORE = 5;
}

DistributedDataStorage::DistributedDataStorage()
{
    appId_.appId = APP_ID;
    storeId_.storeId = STORE_ID;
}

bool DistributedDataStorage::Init()
{
    HILOGD("begin.");
    if (kvStoreDeathRecipient_ == nullptr) {
        kvStoreDeathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new KvStoreDeathRecipient());
    }
    bool ret = InitHandler();
    if (!ret) {
        HILOGE("InitHandler failed!");
        return false;
    }
    ret = InitKvDataService();
    if (!ret) {
        HILOGE("InitKvDataService failed!");
        return false;
    }
    return true;
}

bool DistributedDataStorage::InitKvDataService()
{
    auto waitTask = [this]() {
        if (!WaitKvDataService()) {
            HILOGE("get kvDataService failed!");
            return;
        }
        InitDistributedDataStorage();
        distributedDataChangeListener_ = make_unique<DistributedDataChangeListener>();
        SubscribeDistributedDataStorage();
    };
    if (!dmsDataStorageHandler_->PostTask(waitTask)) {
        HILOGE("post task failed!");
        return false;
    }
    return true;
}

bool DistributedDataStorage::WaitKvDataService()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("get samgrProxy failed!");
        return false;
    }
    int32_t retryTimes = RETRY_TIMES_WAIT_KV_DATA;
    do {
        auto kvDataSvr = samgrProxy->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
        if (kvDataSvr != nullptr) {
            IPCObjectProxy* proxy = reinterpret_cast<IPCObjectProxy*>(kvDataSvr.GetRefPtr());
            if (proxy != nullptr && !proxy->IsObjectDead()) {
                HILOGI("get service success!");
                proxy->AddDeathRecipient(kvStoreDeathRecipient_);
                return true;
            }
        }
        HILOGD("waiting for service...");
        this_thread::sleep_for(1s);
        if (--retryTimes <= 0) {
            HILOGE("waiting service timeout(30)s.");
            return false;
        }
    } while (true);
    return false;
}

void DistributedDataStorage::InitDistributedDataStorage()
{
    int64_t begin = GetTickCount();
    unique_lock<shared_mutex> writeLock(initLock_);
    bool result = TryGetKvStore();
    int64_t end = GetTickCount();
    HILOGI("TryGetKvStore %{public}s, spend %{public}" PRId64 " ms", result ? "success" : "failed", end - begin);
}

bool DistributedDataStorage::TryGetKvStore()
{
    int32_t retryTimes = 0;
    while (retryTimes < RETRY_TIMES_GET_KVSTORE) {
        if (GetKvStore() == Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        HILOGD("retry get kvstore...");
        this_thread::sleep_for(500ms);
        retryTimes++;
    }
    if (kvStorePtr_ == nullptr) {
        return false;
    }
    return true;
}

Status DistributedDataStorage::GetKvStore()
{
    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
    };
    Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    if (status != Status::SUCCESS) {
        HILOGE("GetSingleKvStore failed, status = %{public}d.", status);
    }
    HILOGI("GetSingleKvStore success!");
    return status;
}

void DistributedDataStorage::SubscribeDistributedDataStorage()
{
    int64_t begin = GetTickCount();
    shared_lock<shared_mutex> readLock(initLock_);
    if (kvStorePtr_ == nullptr) {
        HILOGW("kvStorePtr is null!");
        return;
    }
    SubscribeType subscribeType = SubscribeType::DEFAULT;
    if (distributedDataChangeListener_ != nullptr) {
        HILOGD("SubscribeKvStore start.");
        Status status = kvStorePtr_->SubscribeKvStore(subscribeType, move(distributedDataChangeListener_));
        HILOGI("[PerformanceTest] SubscribeKvStore spend %{public}" PRId64 " ms", GetTickCount() - begin);
        if (status != Status::SUCCESS) {
            HILOGE("SubscribeKvStore failed! status = %{public}d.", status);
            return;
        }
    }
}

bool DistributedDataStorage::InitHandler()
{
    if (dmsDataStorageHandler_ == nullptr) {
        shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("dmsDataStorageHandler");
        dmsDataStorageHandler_ = make_shared<AppExecFwk::EventHandler>(runner);
    }
    if (dmsDataStorageHandler_ == nullptr) {
        HILOGW("dmsDataStorageHandler_ is null!");
        return false;
    }
    return true;
}

void DistributedDataStorage::NotifyRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOGD("begin.");
    if (kvStoreDeathRecipient_ != nullptr) {
        remote->RemoveDeathRecipient(kvStoreDeathRecipient_);
    }
}

bool DistributedDataStorage::Stop()
{
    HILOGD("begin.");
    dmsDataStorageHandler_ = nullptr;
    bool ret = UninitDistributedDataStorage();
    if (!ret) {
        HILOGE("UninitDistributedDataStorage failed!");
        return false;
    }
    HILOGD("Stop success!");
    return true;
}

bool DistributedDataStorage::UninitDistributedDataStorage()
{
    int64_t begin = GetTickCount();
    Status status;
    if (distributedDataChangeListener_ != nullptr && kvStorePtr_ != nullptr) {
        SubscribeType subscribeType = SubscribeType::DEFAULT;
        status = kvStorePtr_->UnSubscribeKvStore(subscribeType, move(distributedDataChangeListener_));
        HILOGI("[PerformanceTest] UnSubscribeKvStore spend %{public}" PRId64 " ms", GetTickCount() - begin);
        if (status != Status::SUCCESS) {
            HILOGE("UnSubscribeKvStore failed! status = %{public}d.", status);
            return false;
        }
        distributedDataChangeListener_ = nullptr;
    }
    if (kvStorePtr_ != nullptr) {
        status = dataManager_.CloseKvStore(appId_, storeId_);
        if (status != Status::SUCCESS) {
            HILOGE("CloseKvStore failed! status = %{public}d.", status);
            return false;
        }
        kvStorePtr_ = nullptr;
    }
    status = dataManager_.DeleteKvStore(appId_, storeId_);
    if (status != Status::SUCCESS) {
        HILOGE("DeleteKvStore failed! status = %{public}d.", status);
        return false;
    }
    return true;
}

bool DistributedDataStorage::Insert(const string& networkId, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    if (networkId.empty()) {
        HILOGW("networkId is empty!");
        return false;
    }
    if (missionId < 0) {
        HILOGW("missionId is invalid!");
        return false;
    }
    string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        HILOGW("uuid is empty!");
        return false;
    }
    {
        unique_lock<shared_mutex> writeLock(initLock_);
        bool ret = InsertInnerLocked(uuid, missionId, byteStream, len);
        if (!ret) {
            HILOGE("Insert uuid = %{public}s + missionId = %{public}d failed!",
                DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
            return false;
        }
    }
    HILOGI("Insert uuid = %{public}s + missionId = %{public}d successful!",
        DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
    return true;
}

bool DistributedDataStorage::InsertInnerLocked(const string& uuid, int32_t missionId,
    const uint8_t* byteStream, size_t len)
{
    HILOGD("called.");
    int64_t begin = GetTickCount();
    if (kvStorePtr_ == nullptr) {
        HILOGW("kvStorePtr is null!");
        return false;
    }
    Key key;
    Value value;
    GenerateKey(uuid, missionId, key);
    GenerateValue(byteStream, len, value);
    auto status = kvStorePtr_->Put(key, value);
    HILOGI("[PerformanceTest] Put Snapshot spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (status != Status::SUCCESS) {
        HILOGE("kvStorePtr Put failed! status = %{public}d.", status);
        return false;
    }
    return true;
}

bool DistributedDataStorage::Delete(const string& networkId, int32_t missionId)
{
    if (networkId.empty()) {
        HILOGW("networkId is empty!");
        return false;
    }
    if (missionId < 0) {
        HILOGW("missionId is invalid!");
        return false;
    }
    string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        HILOGW("uuid is empty!");
        return false;
    }
    {
        unique_lock<shared_mutex> writeLock(initLock_);
        bool ret = DeleteInnerLocked(uuid, missionId);
        if (!ret) {
            HILOGE("Delete uuid = %{public}s + missionId = %{public}d failed!",
                DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
            return false;
        }
    }
    HILOGI("Delete uuid = %{public}s + missionId = %{public}d successful!",
        DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
    return true;
}

bool DistributedDataStorage::DeleteInnerLocked(const string& uuid, int32_t missionId)
{
    HILOGD("called.");
    int64_t begin = GetTickCount();
    if (kvStorePtr_ == nullptr) {
        HILOGW("kvStorePtr is null!");
        return false;
    }
    Key key;
    GenerateKey(uuid, missionId, key);
    auto status = kvStorePtr_->Delete(key);
    HILOGI("[PerformanceTest] Delete Snapshot spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (status != Status::SUCCESS) {
        HILOGE("kvStorePtr Delete failed! status = %{public}d.", status);
        return false;
    }
    return true;
}

bool DistributedDataStorage::FuzzyDelete(const string& networkId)
{
    if (networkId.empty()) {
        HILOGW("networkId is empty!");
        return false;
    }
    {
        unique_lock<shared_mutex> writeLock(initLock_);
        bool ret = FuzzyDeleteInnerLocked(networkId);
        if (!ret) {
            HILOGW("FuzzyDelete networkId = %{public}s failed!", DnetworkAdapter::AnonymizeDeviceId(networkId).c_str());
            return false;
        }
    }
    HILOGI("FuzzyDelete networkId = %{public}s successful!", DnetworkAdapter::AnonymizeDeviceId(networkId).c_str());
    return true;
}

bool DistributedDataStorage::FuzzyDeleteInnerLocked(const string& networkId)
{
    HILOGD("called.");
    int64_t begin = GetTickCount();
    if (kvStorePtr_ == nullptr) {
        HILOGW("kvStorePtr is null!");
        return false;
    }
    auto status = kvStorePtr_->RemoveDeviceData(networkId);
    HILOGI("[PerformanceTest] RemoveDeviceData Snapshot spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (status != Status::SUCCESS) {
        HILOGE("kvStorePtr RemoveDeviceData failed! status = %{public}d.", status);
        return false;
    }
    return true;
}

bool DistributedDataStorage::Query(const string& networkId, int32_t missionId, Value& value) const
{
    if (networkId.empty()) {
        HILOGW("networkId is empty!");
        return false;
    }
    if (missionId < 0) {
        HILOGW("missionId is invalid!");
        return false;
    }
    string uuid = DtbschedmgrDeviceInfoStorage::GetInstance().GetUuidByNetworkId(networkId);
    if (uuid.empty()) {
        HILOGW("uuid is empty!");
        return false;
    }
    {
        shared_lock<shared_mutex> readLock(initLock_);
        bool ret = QueryInnerLocked(uuid, missionId, value);
        if (!ret) {
            HILOGE("Query uuid = %{public}s + missionId = %{public}d failed!",
                DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
            return false;
        }
    }
    HILOGI("Query uuid = %{public}s + missionId = %{public}d successful!",
        DnetworkAdapter::AnonymizeDeviceId(uuid).c_str(), missionId);
    return true;
}

bool DistributedDataStorage::QueryInnerLocked(const string& uuid, int32_t missionId, Value& value) const
{
    HILOGD("called.");
    int64_t begin = GetTickCount();
    if (kvStorePtr_ == nullptr) {
        HILOGW("kvStorePtr is null!");
        return false;
    }
    Key key;
    GenerateKey(uuid, missionId, key);
    auto status = kvStorePtr_->Get(key, value);
    HILOGI("[PerformanceTest] Get Snapshot spend %{public}" PRId64 " ms", GetTickCount() - begin);
    if (status != Status::SUCCESS) {
        HILOGE("kvStorePtr Get failed! status = %{public}d.", status);
        return false;
    }
    return true;
}

void DistributedDataStorage::GenerateKey(const string& uuid, int32_t missionId, Key& key)
{
    string keyString;
    keyString.append(uuid).append("_").append(to_string(missionId));
    key = keyString;
}

void DistributedDataStorage::GenerateValue(const uint8_t* byteStream, size_t len, Value& value)
{
    Value valueString((char *)byteStream, len);
    value = valueString;
}
} // DistributedSchedule
} // namespace OHOS