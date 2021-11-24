# Distributed Scheduler<a name="EN-US_TOPIC_0000001162308327"></a>

-   [Introduction](#section11660541593)
-   [System Architecture](#section13587185873516)
-   [Directory Structure](#section161941989596)
-   [Constraints](#section119744591305)
-   [Usage](#section10729231131110)
-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

Distributed Scheduler is used for cross-device component management. It allows the local device to access or control remote components, and enables app collaboration in distributed scenarios. Its main functions are as follows:

-   Remote ability startup: Starts an ability on a remote device.
-   Remote ability migration: Migrates an ability to a remote device.
-   Remote ability binding: Binds an ability on a remote device.


## System Architecture<a name="section13587185873516"></a>

![](figures/dms-architecture.png)

**Figure 1** Distributed Scheduler architecture<a name="fig4460722185514"></a>

## Directory Structure<a name="section161941989596"></a>

The main code directory structure of Distributed Scheduler is as follows:

```
/foundation/distributedschedule/dmsfwk
├── interfaces                              # API definition
├── services                                # Core functions
│   └── dtbschedmgr
│       ├── include
│       │   ├── ability_connection_wrapper_stub.h  # Connection callback wrapper class
│       │   ├── adapter
│       │   │   └── dnetwork_adapter.h             # DSoftBus adaptation layer
│       │   ├── bundle
│       │   │   └── bundle_manager_internal.h      # BMS function wrapper class
│       │   ├── caller_info.h                      # Basic information about the caller
│       │   ├── connect_death_recipient.h          # APIs that listen for connection callback death events
│       │   ├── continuation_callback_death_recipient.h # APIs that listen for continuation callback death events
│       │   ├── deviceManager
│       │   │   └── dms_device_info.h              # APIs for defining and obtaining device information
│       │   ├── distributed_device_node_listener.h # APIs for listening for device online/offline events
│       │   ├── distributed_sched_ability_shell.h  # Continuation callback management APIs
│       │   ├── distributed_sched_adapter.h        # DMS adaptation layer
│       │   ├── distributed_sched_continuation.h   # Continuation token management APIs
│       │   ├── distributed_sched_dumper.h         # Dump APIs
│       │   ├── distributed_sched_interface.h      # External APIs
│       │   ├── distributed_sched_permission.h     # Permission verification APIs
│       │   ├── distributed_sched_proxy.h          # Proxy APIs
│       │   ├── distributed_sched_service.h        # Service APIs
│       │   ├── distributed_sched_stub.h           # Stub APIs
│       │   ├── dtbschedmgr_device_info_storage.h  # Device information storage management
│       │   ├── dtbschedmgr_log.h                  # Log module
│       │   ├── parcel_helper.h                    # Definition of auxiliary macros for serialization/deserialization
│       │   └── uri.h
│       ├── src
│       │   ├── ability_connection_wrapper_stub.cpp  # Implementation of the connection callback wrapper class
│       │   ├── adapter
│       │   │   └── dnetwork_adapter.cpp             # DSoftBus adaptation layer
│       │   ├── bundle
│       │   │   └── bundle_manager_internal.cpp      # Implementation of the BMS function wrapper class
│       │   ├── connect_death_recipient.cpp          # Implementation of the listening for connection callback death events
│       │   ├── continuation_callback_death_recipient.cpp # Implementation of the continuation callback death events
│       │   ├── deviceManager
│       │   │   └── dms_device_info.cpp              # Implementation of the APIs for obtaining device information
│       │   ├── distributed_device_node_listener.cpp # Implementation of the APIs for device online/offline event listening
│       │   ├── distributed_sched_ability_shell.cpp  # Implementation of continuation callback management
│       │   ├── distributed_sched_adapter.cpp        # Implementation of the DMS adaptation layer
│       │   ├── distributed_sched_continuation.cpp   # Implementation of continuation token management 
│       │   ├── distributed_sched_dumper.cpp         # Dump Implementation
│       │   ├── distributed_sched_permission.cpp     # Permission verification implementation
│       │   ├── distributed_sched_proxy.cpp          # Proxy implementation
│       │   ├── distributed_sched_service.cpp        # Server implementation
│       │   ├── distributed_sched_stub.cpp           # Stub implementation
│       │   └── dtbschedmgr_device_info_storage.cpp  # Implementation of device information storage management
│       └──── BUILD.gn
├── sa_profile
├── utils
├── LICENSE
```

## Constraints<a name="section119744591305"></a>

-   The devices between which you want to set up a connection must be in the same LAN.
-   Before setting up a connection between two devices, you must bind the devices. For details about the binding process, see relevant descriptions in the Security subsystem readme file.

## Usage<a name="section10729231131110"></a>

-   **Compiling and Building Distributed Scheduler**

The code of Distributed Scheduler is stored in the following directory:

```
foundation/distributedschedule/dmsfwk
```


## Repositories Involved<a name="section1371113476307"></a>

Distributed Scheduler

[distributedschedule\_dms\_fwk](https://https://gitee.com/openharmony/distributedschedule_dms_fwk)
