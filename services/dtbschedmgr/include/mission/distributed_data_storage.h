/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_DATA_STORAGE_H
#define DISTRIBUTED_DATA_STORAGE_H

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <set>
#include "deviceManager/dms_device_info.h"
#include "distributed_kv_data_manager.h"
#include "event_handler.h"
#include "mission/distributed_data_change_listener.h"

namespace OHOS {
namespace DistributedSchedule {
class DistributedDataStorage {
public:
    DistributedDataStorage() = default;
    ~DistributedDataStorage() = default;

    /**
     * Init DistributedDataStorage.
     *
     * @return Returns true if Init successfully.
     */
    bool Init();

    /**
     * Stop DistributedDataStorage.
     *
     * @return Returns true if Stop successfully.
     */
    bool Stop();

    /**
     * Insert deviceId + missionId to kvStore.
     *
     * @param deviceId, the deviceId to insert
     * @param missionId, the missionId to insert
     * @param byteStream, byte stream for file conversion
     * @param len, length of the byte stream
     * @return Returns true if Insert successfully.
     */
    bool Insert(const std::string& deviceId, int32_t missionId, const uint8_t* byteStream, size_t len);

    /**
     * Delete deviceId + missionId in kvStore.
     *
     * @param deviceId, the deviceId to delete
     * @param missionId, the missionId to delete
     * @return Returns true if Delete successfully.
     */
    bool Delete(const std::string& deviceId, int32_t missionId);

    /**
     * FuzzyDelete deviceId in kvStore.
     *
     * @param deviceId, the deviceId to delete
     * @return Returns true if Delete successfully.
     */
    bool FuzzyDelete(const std::string& deviceId);

    /**
     * Query deviceId + missionId in kvStore.
     *
     * @param deviceId, the deviceId to query
     * @param missionId, the missionId to query
     * @param value, if success return the value
     * @return Returns true if query successfully.
     */
    bool Query(const std::string& deviceId, int32_t missionId, DistributedKv::Value& value) const;

    /**
     * PushSync in kvStore.
     *
     * @param deviceId, push sync to the deviceId
     * @return Returns true if sync successfully.
     */
    bool Sync(const std::string& deviceId);
private:
    bool InitKvDataManager();
    void SubscribeDistributedDataStorage();
    bool UninitKvDataManager();
    bool InsertInnerLocked(const std::string& deviceId, int32_t missionId, const uint8_t* byteStream, size_t len);
    bool DeleteInnerLocked(const std::string& deviceId, int32_t missionId);
    bool FuzzyDeleteInnerLocked(const std::string& deviceId);
    bool QueryInnerLocked(const std::string& deviceId, int32_t missionId, DistributedKv::Value& value) const;
    bool Sync();
    bool SyncInnerLocked(const std::vector<std::string>& deviceVec, const DistributedKv::SyncMode& mode,
        uint32_t syncCount);
    void RetrySync(const std::vector<std::string>& deviceVec, const DistributedKv::SyncMode& mode,
        uint32_t syncCount);
    void InitHandler();
    static void GenerateKey(const std::string& deviceId, int32_t missionId, DistributedKv::Key& key);
    static void GenerateKey(const std::string& deviceId, DistributedKv::Key& key);
    static void GenerateValue(const uint8_t* byteStream, size_t len, DistributedKv::Value& value);

    mutable std::shared_mutex initLock_;
    std::shared_ptr<AppExecFwk::EventHandler> dmsDataStorageHandler_;
    std::shared_ptr<DistributedKv::DistributedKvDataManager> appDataMgr_; // protected by initLock_
    std::unique_ptr<DistributedKv::KvStore> appKvStorePtr_; // protected by initLock_
    std::unique_ptr<DistributedDataChangeListener> appDistributedDataChangeListener_;
};
} // DistributedSchedule
} // OHOS

#endif // DISTRIBUTED_DATA_STORAGE_H
