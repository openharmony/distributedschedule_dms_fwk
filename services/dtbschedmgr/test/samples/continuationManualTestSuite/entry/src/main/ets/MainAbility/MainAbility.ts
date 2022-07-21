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

import Ability from '@ohos.application.Ability'
import AbilityConstant from '@ohos.application.AbilityConstant'
import accessControl from "@ohos.abilityAccessCtrl";
import bundle from '@ohos.bundle';
import distributedObject from '@ohos.data.distributedDataObject';
import Logger from './Logger';

const BUNDLE_NAME = "com.ohos.continuationManualTestSuite.hmservice"
const PERMISSION_REJECT = -1

class ContentStorage {
}

var g_object = distributedObject.createDistributedObject({name:undefined});

export default class MainAbility extends Ability {

    contentStorage : ContentStorage
    sessionId : string

    handleParam(want, launchParam) {
        if (launchParam.launchReason == AbilityConstant.LaunchReason.CONTINUATION) {
            this.sessionId = want.parameters.session
            Logger.info(`continuation sessionId:  ${this.sessionId}`)

            g_object.name = undefined;
            Logger.info(`set g_object.name undefined`)

            g_object.setSessionId(this.sessionId); // set session id, so it will sync data from remote device
            Logger.info(`g_object_name = ${g_object.name}`);

            AppStorage.SetOrCreate<string>('ContinueStudy', g_object.name)
            let workInput = want.parameters.work // get user data from want params
            Logger.info(`work input ${workInput}`)
            AppStorage.SetOrCreate<string>('ContinueWork', workInput)
            this.contentStorage = new ContentStorage();
            Logger.info('ready to restore');
            this.context.restoreWindowStage(this.contentStorage);
        }
    }

    onCreate(want, launchParam) {
        Logger.info("[Demo] MainAbility onCreate")
        globalThis.abilityWant = want;
        this.handleParam(want, launchParam)
    }

    onContinue(wantParam : {[key: string]: any}) {
        Logger.info(`onContinue version = ${wantParam.version}, targetDevice: ${wantParam.targetDevice}`)
        let workInput = AppStorage.Get<string>('ContinueWork');
        Logger.info(`onContinue work input = ${workInput}`);

        if (g_object.__sessionId === undefined) {
            this.sessionId = distributedObject.genSessionId()
            Logger.info(`onContinue generate new sessionId`)
        }
        else {
            this.sessionId = g_object.__sessionId;
        }

        wantParam["session"] = this.sessionId
        g_object.name = AppStorage.Get<string>('ContinueStudy');
        Logger.info(`onContinue sessionId = ${this.sessionId}, name = ${g_object.name}`)
        g_object.setSessionId(this.sessionId);
        g_object.save(wantParam.targetDevice, (result, data)=>{
            Logger.info("save callback");
            Logger.info("save sessionId " + data.sessionId);
            Logger.info("save version " + data.version);
            Logger.info("save deviceId " + data.deviceId);
        });
        wantParam["work"] = workInput // set user input data into want params
        return 0;
    }

    onNewWant(want, launchParam) {
        Logger.info("MainAbility onNewWant")
        this.handleParam(want, launchParam)
    }

    requestPermissions = async () => {
        let permissions: Array<string> = [
            "ohos.permission.DISTRIBUTED_DATASYNC"
        ];
        let needGrantPermission = false
        let accessManger = accessControl.createAtManager()
        Logger.info("app permission get bundle info")
        let bundleInfo = await bundle.getApplicationInfo(BUNDLE_NAME, 0, 100)
        Logger.info(`app permission query permission ${bundleInfo.accessTokenId.toString()}`)
        for (const permission of permissions) {
            Logger.info(`app permission query grant status ${permission}`)
            try {
                let grantStatus = await accessManger.verifyAccessToken(bundleInfo.accessTokenId, permission)
                if (grantStatus === PERMISSION_REJECT) {
                    needGrantPermission = true
                    break;
                }
            } catch (err) {
                Logger.error(`app permission query grant status error ${permission} ${JSON.stringify(err)}`)
                needGrantPermission = true
                break;
            }
        }
        if (needGrantPermission) {
            Logger.info("app permission needGrantPermission")
            try {
                await this.context.requestPermissionsFromUser(permissions)
            } catch (err) {
                Logger.error(`app permission ${JSON.stringify(err)}`)
            }
        } else {
            Logger.info("app permission already granted")
        }
    }

    onDestroy() {
        Logger.info("MainAbility onDestroy")
    }

    onWindowStageCreate(windowStage) {
        // Main window is created, set main page for this ability
        Logger.info("MainAbility onWindowStageCreate")
        this.requestPermissions()
        windowStage.setUIContent(this.context, "pages/index", null)
    }

    onWindowStageRestore(windowStage) {
        Logger.info("MainAbility onWindowStageRestore")
        this.requestPermissions()
    }

    onWindowStageDestroy() {
        // Main window is destroyed, release UI related resources
        Logger.info("MainAbility onWindowStageDestroy")
    }

    onForeground() {
        // Ability has brought to foreground
        Logger.info("MainAbility onForeground")
    }

    onBackground() {
        // Ability has back to background
        Logger.info("MainAbility onBackground")
    }
};
