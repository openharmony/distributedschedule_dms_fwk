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

import rpc from "@ohos.rpc";

class FirstServiceAbilityStub extends rpc.RemoteObject {
    constructor(des) {
        if (typeof des === 'string') {
            super(des);
        } else {
            return null;
        }
    }

    onRemoteRequest(code, data, reply, option) {
        console.log("FirstServiceAbilityStub onRemoteRequest called");
        if (code === 1) {
            let op1 = data.readInt();
            let op2 = data.readInt();
            console.log("op1 = " + op1 + ", op2 = " + op2);
            reply.writeInt(op1 + op2);
        } else {
            console.log("FirstServiceAbilityStub unknown request code");
        }
        return true;
    }
}

export default {
    onStop() {
        console.log('FirstServiceAbilityStub onStop');
    },
    onStart() {
        console.log("FirstServiceAbilityStub onStart");
    },
    onConnect(want) {
        console.log("FirstServiceAbilityStub onConnect");
        try {
            let value = JSON.stringify(want);
            console.log("FirstServiceAbilityStub want:" + value);
        } catch (error) {
            console.log("FirstServiceAbilityStub error:" + error);
        }
        return new FirstServiceAbilityStub("first js service stub");
    },
    onDisconnect(want) {
        console.log("FirstServiceAbilityStub onDisconnect");
        let value = JSON.stringify(want);
        console.log("FirstServiceAbilityStub want:" + value);
    },
    onCommand(want, startId) {
        console.log("FirstServiceAbilityStub onCommand");
        let value = JSON.stringify(want);
        console.log("FirstServiceAbilityStub want:" + value);
        console.log("FirstServiceAbilityStub startId:" + startId);
    }
};
