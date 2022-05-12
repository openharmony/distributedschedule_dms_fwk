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

import featureAbility from '@ohos.ability.featureAbility';
import RemoteDeviceModel from '../../../model/RemoteDeviceModel.js';
import rpc from "@ohos.rpc";
import prompt from '@system.prompt';

var mRemote;
var DEVICE_LIST_LOCALHOST;
var connectedAbility;
var localDeviceId = "";

export default {
    data: {
        title: '',
        remoteDeviceModel: new RemoteDeviceModel,
        deviceList: [],
        btnPlaySrc: '/common/media/ic_play.svg',
        albumSrc: '/common/media/album.png',
        isDialogShowing: false,
        isSwitching: false,
        hasInitialized: false,
    },
    onInit() {
        DEVICE_LIST_LOCALHOST = {
            name: this.$t('localhost'),
            id: 'localhost',
        };
        this.deviceList = [DEVICE_LIST_LOCALHOST];
        let self = this;
        this.remoteDeviceModel.registerDeviceListCallback(() => {
            console.info('registerDeviceListCallback, callback entered');
            var list = [];
            list[0] = DEVICE_LIST_LOCALHOST;
            var deviceList;
            if (self.remoteDeviceModel.discoverList.length > 0) {
                deviceList = self.remoteDeviceModel.discoverList;
            } else {
                deviceList = self.remoteDeviceModel.deviceList;
            }
            console.info('on remote device updated, count=' + deviceList.length);
            for (var i = 0; i < deviceList.length; i++) {
                console.info('device ' + i + '/' + deviceList.length + ' deviceId='
                + deviceList[i].deviceId + ' deviceName=' + deviceList[i].deviceName + ' deviceType='
                + deviceList[i].deviceType);
                list[i + 1] = {
                    name: deviceList[i].deviceName,
                    id: deviceList[i].deviceId,
                };
            }
            self.deviceList = list;
        });
        console.info('onInit end');
    },
    onDestroy() {
        console.info('onDestroy begin');
        this.remoteDeviceModel.unregisterDeviceListCallback();
        console.info('onDestroy end');
    },

    onStartLocalAbilityClick() {
        console.info('[dmsDemo] onStartLocalAbilityClick begin');
        console.info('[dmsDemo] onStartLocalAbilityClick deviceId is ' + localDeviceId);
        var wantValue = {
            bundleName: 'ohos.dms.jsDemo',
            abilityName: 'ohos.dms.jsDemo.LocalAbility',
            deviceId: localDeviceId,
        };
        featureAbility.startAbility({
            want: wantValue
        }).then((data) => {
            console.info('[dmsDemo] featureAbility.startAbility finished, ' + JSON.stringify(data));
        });
        console.info('[dmsDemo] onStartLocalAbilityClick end');
    },

    onStartAbilityClick() {
        console.info('[dmsDemo] onStartAbilityClick begin');
        this.$element('dialogForStartAbility').show();
        this.isDialogShowing = true;
        console.info('[dmsDemo] onStartAbilityClick end');
    },

    onRadioChangeForStart(inputValue, e) {
        console.info('[dmsDemo] onRadioChangeForStart ' + inputValue + ', ' + e.value);
        if (inputValue === e.value) {
            if (e.value === 'localhost') {
                this.$element('dialogForStartAbility').close();
                return;
            }
            if (this.remoteDeviceModel.discoverList.length > 0) {
                console.info('[dmsDemo] start to  device');
                var name = null;
                for (var i = 0; i < this.remoteDeviceModel.discoverList.length; i++) {
                    if (this.remoteDeviceModel.discoverList[i].deviceId === e.value) {
                        name = this.remoteDeviceModel.discoverList[i].deviceName;
                        break;
                    }
                }
                if (name == null) {
                    console.error('[dmsDemo] onRadioChangeForStart failed, can not get name from discoverList');
                    return;
                }
                console.info('[dmsDemo] onRadioChangeForStart name=' + name);

                let self = this;
                this.remoteDeviceModel.authDevice(e.value, () => {
                    console.info('[dmsDemo] onRadioChangeForStart auth and online finished');
                    for (i = 0; i < self.remoteDeviceModel.deviceList.length; i++) {
                        if (self.remoteDeviceModel.deviceList[i].deviceName === name) {
                            this.startRemoteAbility(self.remoteDeviceModel.deviceList[i].deviceId,
                                self.remoteDeviceModel.deviceList[i].deviceName);
                        }
                    }
                });
            } else {
                console.info('[dmsDemo] onRadioChangeForStart start to authed device');
                for (i = 0; i < this.remoteDeviceModel.deviceList.length; i++) {
                    if (this.remoteDeviceModel.deviceList[i].deviceId === e.value) {
                        this.startRemoteAbility(this.remoteDeviceModel.deviceList[i].deviceId,
                            this.remoteDeviceModel.deviceList[i].deviceName);
                    }
                }
            }
        }
    },

    startRemoteAbility(deviceId, deviceName) {
        this.$element('dialogForStartAbility').close();
        var params;
        console.info('[dmsDemo] featureAbility.startAbility deviceId=' + deviceId
        + ' deviceName=' + deviceName);
        var wantValue = {
            bundleName: 'ohos.dms.jsDemo',
            abilityName: 'ohos.dms.jsDemo.RemoteAbility',
            deviceId: deviceId,
            parameters: params
        };
        featureAbility.startAbility({
            want: wantValue
        }).then((data) => {
            console.info('[dmsDemo] featureAbility.startAbility finished, ' + JSON.stringify(data));
        });
        console.info('[dmsDemo] featureAbility.startAbility want=' + JSON.stringify(wantValue));
        console.info('[dmsDemo] featureAbility.startAbility end');
    },

    async onContinueLocalAbilityClicked(){
        console.info('[dmsDemo] ContinueLocalAbility begin');
        console.info('[dmsDemo] ContinueLocalAbility deviceId is ' + localDeviceId);
        let continueAbilityOptions = {
            reversible: false,
            deviceId: localDeviceId,
        }
        function ContinueAbilityCallback(err, data) {
            console.info("[dmsDemo] ContinueAbilityCallback, result err = " + JSON.stringify(err));
            console.info("[dmsDemo] ContinueAbilityCallback, result data= " + JSON.stringify(data));
        }
        await featureAbility.continueAbility(continueAbilityOptions, ContinueAbilityCallback);
        done();
        console.info('[dmsDemo] ContinueLocalAbility end');
    },

    onContinueAbilityClicked(){
        console.info('[dmsDemo] ContinueAbility begin');
        this.$element('dialogForContinueAbility').show();
        this.isDialogShowing = true;
        console.info('[dmsDemo] ContinueAbility end');
    },

    onRadioChangeForContinue(inputValue, e) {
        console.info('[dmsDemo] onRadioChangeForContinue ' + inputValue + ', ' + e.value);
        if (inputValue === e.value) {
            if (e.value === 'localhost') {
                this.$element('dialogForContinueAbility').close();
                return;
            }
            if (this.remoteDeviceModel.discoverList.length > 0) {
                console.info('[dmsDemo] onRadioChangeForContinue to  device');
                var name = null;
                for (var i = 0; i < this.remoteDeviceModel.discoverList.length; i++) {
                    if (this.remoteDeviceModel.discoverList[i].deviceId === e.value) {
                        name = this.remoteDeviceModel.discoverList[i].deviceName;
                        break;
                    }
                }
                if (name == null) {
                    console.error('[dmsDemo] onRadioChangeForContinue failed, can not get name from discoverList');
                    return;
                }
                console.info('[dmsDemo] onRadioChangeForContinue name=' + name);

                let self = this;
                this.remoteDeviceModel.authDevice(e.value, () => {
                    console.info('[dmsDemo] onRadioChangeForContinue auth and online finished');
                    for (i = 0; i < self.remoteDeviceModel.deviceList.length; i++) {
                        if (self.remoteDeviceModel.deviceList[i].deviceName === name) {
                            this.startContinueAbility(self.remoteDeviceModel.deviceList[i].deviceId,
                                self.remoteDeviceModel.deviceList[i].deviceName);
                        }
                    }
                });
            } else {
                console.info('[dmsDemo] onRadioChangeForContinue continue to authed device');
                for (i = 0; i < this.remoteDeviceModel.deviceList.length; i++) {
                    if (this.remoteDeviceModel.deviceList[i].deviceId === e.value) {
                        this.startContinueAbility(this.remoteDeviceModel.deviceList[i].deviceId,
                            this.remoteDeviceModel.deviceList[i].deviceName);
                    }
                }
            }
        }
    },

    async startContinueAbility(deviceId, deviceName) {
        this.$element('dialogForContinueAbility').close();
        console.info('[dmsDemo] featureAbility.startContinueAbility deviceId=' + deviceId
        + ' deviceName=' + deviceName);
        let continueAbilityOptions = {
            reversible: false,
            deviceId: deviceId,
        }
        function ContinueAbilityCallback(err, data) {
            console.info("[dmsDemo] ContinueAbilityCallback, result err = " + JSON.stringify(err));
            console.info("[dmsDemo] ContinueAbilityCallback, result data= " + JSON.stringify(data));
        }
        await featureAbility.continueAbility(continueAbilityOptions, ContinueAbilityCallback);
        done();
        console.info('[dmsDemo] featureAbility.startContinueAbility end');
    },

    async ConnectLocalService(){
        console.log('[dmsDemo] ConnectLocalService begin');
        async function onConnectCallback(element, remote){
            console.log('[dmsDemo] ConnectLocalService onConnectDone element: ' + element);
            console.log('[dmsDemo] ConnectLocalService onConnectDone remote: ' + remote);
            mRemote = remote;
            if (mRemote == null) {
                prompt.showToast({
                    message: "not connected yet"
                });
                return;
            }
            let option = new rpc.MessageOption();
            let data = new rpc.MessageParcel();
            let reply = new rpc.MessageParcel();
            data.writeInt(1);
            data.writeInt(99);
            await mRemote.sendRequest(1, data, reply, option);
            let msg = reply.readInt();
            prompt.showToast({
                message: "connect result: " + msg,
                duration: 3000
            });
        }
        function onDisconnectCallback(element){
            console.log('[dmsDemo] ConnectLocalService onDisconnectDone element: ' + element);
        }
        function onFailedCallback(code){
            console.log('[dmsDemo] ConnectLocalService onFailed errCode: ' + code)
            prompt.showToast({
                message: "ConnectLocalService onFailed: " + code
            });
        }

        console.info('[dmsDemo] ConnectLocalAbility deviceId is: ' + localDeviceId)
        connectedAbility = featureAbility.connectAbility(
            {
                deviceId: localDeviceId,
                bundleName: "ohos.dms.jsDemo",
                abilityName: "ohos.dms.jsDemo.ServiceAbility",
            },
            {
                onConnect: onConnectCallback,
                onDisconnect: onDisconnectCallback,
                onFailed: onFailedCallback,
            },
        );
        console.info('[dmsDemo] ConnectLocalService is: ' + connectedAbility)
    },

    onConnectRemoteServiceClicked() {
        console.info('[dmsDemo] ConnectRemoteService begin');
        this.$element('dialogForConnectService').show();
        this.isDialogShowing = true;
        console.info('[dmsDemo] ConnectRemoteService end');
    },

    onRadioChangeForConnect(inputValue, e) {
        console.info('[dmsDemo] onRadioChangeForConnect ' + inputValue + ', ' + e.value);
        if (inputValue === e.value) {
            if (e.value === 'localhost') {
                this.$element('dialogForConnectService').close();
                return;
            }
            if (this.remoteDeviceModel.discoverList.length > 0) {
                console.info('[dmsDemo] onRadioChangeForConnect to  device');
                var name = null;
                for (var i = 0; i < this.remoteDeviceModel.discoverList.length; i++) {
                    if (this.remoteDeviceModel.discoverList[i].deviceId === e.value) {
                        name = this.remoteDeviceModel.discoverList[i].deviceName;
                        break;
                    }
                }
                if (name == null) {
                    console.error('[dmsDemo] onRadioChangeForConnect failed, can not get name from discoverList');
                    return;
                }
                console.info('[dmsDemo] onRadioChangeForConnect name=' + name);

                let self = this;
                this.remoteDeviceModel.authDevice(e.value, () => {
                    console.info('[dmsDemo] onRadioChangeForConnect auth and online finished');
                    for (i = 0; i < self.remoteDeviceModel.deviceList.length; i++) {
                        if (self.remoteDeviceModel.deviceList[i].deviceName === name) {
                            this.connectRemoteService(self.remoteDeviceModel.deviceList[i].deviceId,
                                self.remoteDeviceModel.deviceList[i].deviceName);
                        }
                    }
                });
            } else {
                console.info('[dmsDemo] onRadioChangeForConnect continue to authed device');
                for (i = 0; i < this.remoteDeviceModel.deviceList.length; i++) {
                    if (this.remoteDeviceModel.deviceList[i].deviceId === e.value) {
                        this.connectRemoteService(this.remoteDeviceModel.deviceList[i].deviceId,
                            this.remoteDeviceModel.deviceList[i].deviceName);
                    }
                }
            }
        }
    },

    async connectRemoteService(deviceId, deviceName){
        this.$element('dialogForConnectService').close();
        console.info('[dmsDemo] featureAbility.connectAbility deviceId=' + deviceId
        + ' deviceName=' + deviceName);
        async function onConnectCallback(element, remote){
            console.log('[dmsDemo] connectRemoteService onConnectDone element: ' + element);
            console.log('[dmsDemo] connectRemoteService onConnectDone remote: ' + remote);
            mRemote = remote;
            if (mRemote == null) {
                prompt.showToast({
                    message: "not connected yet"
                });
                return;
            }
            let option = new rpc.MessageOption();
            let data = new rpc.MessageParcel();
            let reply = new rpc.MessageParcel();
            data.writeInt(1);
            data.writeInt(99);
            await mRemote.sendRequest(1, data, reply, option);
            let msg = reply.readInt();
            prompt.showToast({
                message: "connect result: " + msg,
                duration: 3000
            });
        }
        function onDisconnectCallback(element){
            console.log('[dmsDemo] connectRemoteService onDisconnectDone element: ' + element);
        }
        function onFailedCallback(code){
            console.log('[dmsDemo] connectRemoteService onFailed errCode: ' + code)
            prompt.showToast({
                message: "connectRemoteService onFailed: " + code
            });
        }

        connectedAbility = featureAbility.connectAbility(
            {
                deviceId: deviceId,
                bundleName: "ohos.dms.jsDemo",
                abilityName: "ohos.dms.jsDemo.ServiceAbility",
            },
            {
                onConnect: onConnectCallback,
                onDisconnect: onDisconnectCallback,
                onFailed: onFailedCallback,
            },
        );
        console.info('[dmsDemo] ConnectedAbility is: ' + connectedAbility)
    },

    async DisConnectService() {
        console.log('[dmsDemo] DisConnectService begin');
        if (connectedAbility == null) {
            prompt.showToast({
                message: "not connected yet"
            });
            return;
        }
        await featureAbility.disconnectAbility(connectedAbility);
        connectedAbility = null;
        prompt.showToast({
            message: "disconnect done"
        });
    },

    cancelDialog(e) {
        this.remoteDeviceModel.unregisterDeviceListCallback();
        this.isDialogShowing = false;
        this.$element('dialogForStartAbility').close();
        this.$element('dialogForContinueAbility').close();
        this.$element('dialogForConnectService').close();
    },
    onDismissDialogForStartClicked(e) {
        this.dismissDialogForStart();
    },
    dismissDialogForStart() {
        this.$element('dialogForStartAbility').close();
        this.remoteDeviceModel.unregisterDeviceListCallback();
        this.isDialogShowing = false;
    },
    onDismissDialogForContinueClicked(e) {
        this.dismissDialogForContinue();
    },
    dismissDialogForContinue() {
        this.$element('dialogForContinueAbility').close();
        this.remoteDeviceModel.unregisterDeviceListCallback();
        this.isDialogShowing = false;
    },
    onDismissDialogForConnectClicked(e) {
        this.dismissDialogForConnect();
    },
    dismissDialogForConnect() {
        this.$element('dialogForConnectService').close();
        this.remoteDeviceModel.unregisterDeviceListCallback();
        this.isDialogShowing = false;
    },

};
