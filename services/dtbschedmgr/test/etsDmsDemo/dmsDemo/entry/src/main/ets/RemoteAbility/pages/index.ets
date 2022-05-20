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
import prompt from '@system.prompt';
import rpc from "@ohos.rpc";
import RemoteDeviceModel from '../../model/RemoteDeviceModel.ets';

var connectedAbility;
var localDeviceId = "";
var mRemote;
var remoteDeviceModel = new RemoteDeviceModel;
var mIndex;
var mDiscoveredSize = 0;
var mDeviceSize = 0;

enum Operation {
  AuthDeviceOperation,
  StartRemoteAbilityOperation,
  ContinueAbilityOperation,
  ConnectRemoteServiceOperation,
}
let mOperation = Operation.AuthDeviceOperation;

function registerDeviceListCallback() {
  console.info('[dmsDemo] registerDeviceListCallback begin');
  remoteDeviceModel.registerDeviceListCallback(() => {
    console.info('[dmsDemo] registerDeviceListCallback, callback entered');
    let discoveredDevices = remoteDeviceModel.discoverList.length;
    let numDevices = remoteDeviceModel.deviceList.length;
    mDiscoveredSize = discoveredDevices;
    mDeviceSize = numDevices;
    console.info('[dmsDemo] mDiscoveredSize =' + mDiscoveredSize);
    console.info('[dmsDemo] mDeviceSize =' + mDeviceSize);
    console.info('[dmsDemo] registerDeviceListCallback on remote device updated, count=' + numDevices);
    if (numDevices === 0) {
      if (discoveredDevices > 0) {
        prompt.showToast({
          message: "registerDeviceListCallback dicovered " + discoveredDevices + " devices, need to authenticate"
        });
      } else {
        prompt.showToast({
          message: "registerDeviceListCallback no device discovered"
        });
      }
    }
    if (numDevices >= 1) {
      prompt.showToast({
        message: "registerDeviceListCallback at least one device is successfully networked"
      });
    }
  });
  console.info('[dmsDemo] registerDeviceListCallback end');
}

function authDevice(index) {
  console.info('[dmsDemo] authDevice begin');
  if (remoteDeviceModel.deviceList.length >= 1 && remoteDeviceModel.discoverList.length == 0) {
    prompt.showToast({
      message: "authDevice all devices networking success, no need authenticate"
    });
    return;
  }
  if (remoteDeviceModel.discoverList.length > 0) {
    remoteDeviceModel.authDevice(remoteDeviceModel.discoverList[index - 1].deviceId, () => {
      console.info('[dmsDemo] authDevice, callback entered');
      prompt.showToast({
        message: "authDevice authenticate success"
      });
    });
  } else {
    prompt.showToast({
      message: "authDevice no device discovered"
    });
  }
  console.info('[dmsDemo] authDevice end');
}

function unregisterDeviceListCallback() {
  console.info('[dmsDemo] unregisterDeviceListCallback begin');
  remoteDeviceModel.unregisterDeviceListCallback();
  console.info('[dmsDemo] unregisterDeviceListCallback end');
}

function onStartLocalAbility() {
  console.info('[dmsDemo] onStartLocalAbility begin');
  console.info('[dmsDemo] onStartLocalAbility deviceId is ' + localDeviceId);
  var wantValue = {
    deviceId: localDeviceId,
    bundleName: 'ohos.dms.etsDemo',
    abilityName: 'ohos.dms.etsDemo.LocalAbility',
  };
  featureAbility.startAbility({
    want: wantValue
  }).then((data) => {
    console.info('[dmsDemo] featureAbility.startAbility finished, ' + JSON.stringify(data));
  });
  console.info('[dmsDemo] onStartLocalAbility end');
}

function onStartRemoteAbility(index) {
  console.info('[dmsDemo] onStartRemoteAbility begin');
  let numDevices = remoteDeviceModel.deviceList.length;
  if (numDevices === 0) {
    prompt.showToast({
      message: "onStartRemoteAbility no device found"
    });
    return;
  }
  let deviceIdForStart = remoteDeviceModel.deviceList[index - 1].deviceId;
  console.info('[dmsDemo] onStartRemoteAbility deviceId is ' + deviceIdForStart);
  var params;
  var wantValue = {
    bundleName: 'ohos.dms.etsDemo',
    abilityName: 'ohos.dms.etsDemo.RemoteAbility',
    deviceId: deviceIdForStart,
    parameters: params
  };
  console.info('[dmsDemo] onStartRemoteAbility want=' + JSON.stringify(wantValue));
  featureAbility.startAbility({
    want: wantValue
  }).then((data) => {
    console.info('[dmsDemo] onStartRemoteAbility finished, ' + JSON.stringify(data));
  });
  console.info('[dmsDemo] onStartRemoteAbility end');
}

async function onContinueLocalAbility() {
  console.info('[dmsDemo] onContinueLocalAbility begin');
  console.info('[dmsDemo] onContinueLocalAbility deviceId is ' + localDeviceId);
  let continueAbilityOptions = {
    reversible: false,
    deviceId: localDeviceId,
  };
  function ContinueAbilityCallback(err, data) {
    console.info("[dmsDemo] onContinueLocalAbility ContinueAbilityCallback, result err = " + JSON.stringify(err));
    console.info("[dmsDemo] onContinueLocalAbility ContinueAbilityCallback, result data= " + JSON.stringify(data));
  };
  await featureAbility.continueAbility(continueAbilityOptions, ContinueAbilityCallback);
  console.info('[dmsDemo] onContinueLocalAbility end');
}

async function onContinueAbility(index) {
  console.info('[dmsDemo] onContinueAbility begin');
  let numDevices = remoteDeviceModel.deviceList.length;
  if (numDevices === 0) {
    prompt.showToast({
      message: "onContinueAbility no device found"
    });
    return;
  }
  let deviceIdForContinue = remoteDeviceModel.deviceList[index - 1].deviceId;
  console.info('[dmsDemo] onContinueAbility deviceId is ' + deviceIdForContinue);
  let continueAbilityOptions = {
    reversible: false,
    deviceId: deviceIdForContinue,
  };
  function ContinueAbilityCallback(err, data) {
    console.info("[dmsDemo] onContinueAbility ContinueAbilityCallback, result err = " + JSON.stringify(err));
    console.info("[dmsDemo] onContinueAbility ContinueAbilityCallback, result data= " + JSON.stringify(data));
  };
  await featureAbility.continueAbility(continueAbilityOptions, ContinueAbilityCallback);
  console.info('[dmsDemo] onContinueAbility end');
}

function onConnectLocalService() {
  console.info('[dmsDemo] onConnectLocalService begin');
  console.info('[dmsDemo] onConnectLocalService deviceId is ' + localDeviceId);
  async function onConnectCallback(element, remote) {
    console.log('[dmsDemo] onConnectLocalService onConnectDone element: ' + element);
    console.log('[dmsDemo] onConnectLocalService onConnectDone remote: ' + remote);
    mRemote = remote;
    if (mRemote == null) {
      prompt.showToast({
        message: "onConnectLocalService not connected yet"
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
      message: "onConnectLocalService connect result: " + msg,
      duration: 3000
    });
  }

  function onDisconnectCallback(element) {
    console.log('[dmsDemo] onConnectLocalService onDisconnectDone element: ' + element);
  }

  function onFailedCallback(code) {
    console.log('[dmsDemo] onConnectLocalService onFailed errCode: ' + code)
    prompt.showToast({
      message: "onConnectLocalService onFailed: " + code
    });
  }

  connectedAbility = featureAbility.connectAbility(
    {
      deviceId: localDeviceId,
      bundleName: "ohos.dms.js",
      abilityName: "ohos.dms.js.ServiceAbility",
    },
    {
      onConnect: onConnectCallback,
      onDisconnect: onDisconnectCallback,
      onFailed: onFailedCallback,
    },
  );
  console.log('[dmsDemo] onConnectLocalService connectedAbility: ' + connectedAbility)
}

function onConnectRemoteService(index) {
  console.info('[dmsDemo] onConnectRemoteService begin');
  async function onConnectCallback(element, remote) {
    console.log('[dmsDemo] onConnectRemoteService onConnectDone element: ' + element);
    console.log('[dmsDemo] onConnectRemoteService onConnectDone remote: ' + remote);
    mRemote = remote;
    if (mRemote == null) {
      prompt.showToast({
        message: "onConnectRemoteService not connected yet"
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
      message: "onConnectRemoteService connect result: " + msg,
      duration: 3000
    });
  }

  function onDisconnectCallback(element) {
    console.log('[dmsDemo] onConnectRemoteService onDisconnectDone element: ' + element);
  }

  function onFailedCallback(code) {
    console.log('[dmsDemo] onConnectRemoteService onFailed errCode: ' + code)
    prompt.showToast({
      message: "onConnectRemoteService onFailed: " + code
    });
  }

  let numDevices = remoteDeviceModel.deviceList.length;
  if (numDevices === 0) {
    prompt.showToast({
      message: "onConnectRemoteService no device found"
    });
    return;
  }
  let deviceIdForConnect = remoteDeviceModel.deviceList[index - 1].deviceId;
  console.info('[dmsDemo] onConnectRemoteService deviceId is ' + deviceIdForConnect);
  connectedAbility = featureAbility.connectAbility(
    {
      deviceId: deviceIdForConnect,
      bundleName: "ohos.dms.js",
      abilityName: "ohos.dms.js.ServiceAbility",
    },
    {
      onConnect: onConnectCallback,
      onDisconnect: onDisconnectCallback,
      onFailed: onFailedCallback,
    },
  );
  console.log('[dmsDemo] onConnectRemoteService connectedAbility: ' + connectedAbility)
}

async function onDisconnectService() {
  console.log('[dmsDemo] onDisconnectService begin');
  if (connectedAbility == null) {
    prompt.showToast({
      message: "onDisconnectService not connected yet"
    });
    return;
  }
  await featureAbility.disconnectAbility(connectedAbility);
  connectedAbility = null;
  prompt.showToast({
    message: "onDisconnectService disconnect done"
  });
}

@CustomDialog
struct CustomDialogExample {
  controller: CustomDialogController
  cancel: () => void
  confirm: () => void

  build() {
    Column() {
      TextInput({placeholder:'please input device index', text:''})
        .type(InputType.Number)
        .enterKeyType(EnterKeyType.Send)
        .onChange((value) => {
          console.log('[dmsDemo] TextInput onChange:' + value);
          mIndex = value;
        }).onSubmit((enterKey) => {
        console.log('[dmsDemo] TextInput onSubmit:' + enterKey);
      }).onEditChanged((isEditing) => {
        console.log('[dmsDemo] TextInput onEditChanged:' + isEditing);
      }).width(250).height(50)
      Flex({ justifyContent: FlexAlign.SpaceAround }) {
        Button('cancel')
          .onClick(() => {
            this.controller.close()
            this.cancel()
          }).backgroundColor(0xffffff).fontColor(Color.Black)
        Button('submit')
          .onClick(() => {
            this.controller.close()
            this.confirm()
          }).backgroundColor(0xffffff).fontColor(Color.Red)
      }.margin({ bottom: 10 })
    }
  }
}

@Entry
@Component
struct Index {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: CustomDialogExample({ cancel: this.onCancel, confirm: this.onAccept }),
    cancel: this.existApp,
    autoCancel: true
  })

  onCancel() {
    console.info('[dmsDemo] Callback when onCancel button is clicked');
  }
  onAccept() {
    console.info('[dmsDemo] Callback when onAccept button is clicked, mOperation is ' + mOperation);
    if (mIndex <= 0) {
      prompt.showToast({
        message: "Please input correct device index."
      });
      return;
    }
    if (mOperation === Operation.AuthDeviceOperation) {
      console.info('[dmsDemo] call authDevice, device index: ' + mIndex);
      if (mIndex > mDiscoveredSize) {
        prompt.showToast({
          message: "Please input correct device index."
        });
        return;
      }
      authDevice(mIndex);
    } else if (mOperation === Operation.StartRemoteAbilityOperation) {
      console.info('[dmsDemo] call onStartRemoteAbility, device index: ' + mIndex);
      if (mIndex > mDeviceSize) {
        prompt.showToast({
          message: "Please input correct device index."
        });
        return;
      }
      onStartRemoteAbility(mIndex);
    } else if (mOperation === Operation.ContinueAbilityOperation) {
      console.info('[dmsDemo] call onContinueAbility, device index: ' + mIndex);
      if (mIndex > mDeviceSize) {
        prompt.showToast({
          message: "Please input correct device index."
        });
        return;
      }
      onContinueAbility(mIndex);
    } else if (mOperation === Operation.ConnectRemoteServiceOperation){
      console.info('[dmsDemo] call onConnectRemoteService, device index: ' + mIndex);
      if (mIndex > mDeviceSize) {
        prompt.showToast({
          message: "Please input correct device index."
        });
        return;
      }
      onConnectRemoteService(mIndex);
    }
  }
  existApp() {
    console.info('[dmsDemo] Click the callback in the blank area');
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Button() {
        Text('RegisterDeviceListCallback')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        registerDeviceListCallback();
      })

      Button() {
        Text('AuthDevice')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        mOperation = Operation.AuthDeviceOperation;
        this.dialogController.open();
      })

      Button() {
        Text('UnregisterDeviceListCallback')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        unregisterDeviceListCallback();
      })

      Button() {
        Text('StartLocalAbility')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        onStartLocalAbility();
      })

      Button() {
        Text('StartRemoteAbility')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        mOperation = Operation.StartRemoteAbilityOperation;
        console.info('[dmsDemo] StartRemoteAbility onClick, mOperation is ' + mOperation);
        this.dialogController.open();
      })

      Button() {
        Text('ContinueLocalAbility')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        onContinueLocalAbility();
      })

      Button() {
        Text('ContinueAbility')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        mOperation = Operation.ContinueAbilityOperation;
        this.dialogController.open();
      })

      Button() {
        Text('ConnectLocalService')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        onConnectLocalService();
      })

      Button() {
        Text('ConnectRemoteService')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        mOperation = Operation.ConnectRemoteServiceOperation;
        this.dialogController.open();
      })

      Button() {
        Text('DisConnectService')
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
      }.type(ButtonType.Capsule)
      .margin({
        top: 20
      })
      .backgroundColor('#0D9FFB')
      .onClick(() => {
        onDisconnectService();
      })

    }
    .width('100%')
    .height('100%')
  }
}