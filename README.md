# Distributed Scheduler<a name="EN-US_TOPIC_0000001115719369"></a>

## Introduction<a name="section11660541593"></a>

Distributed Scheduler is used for cross-device component management. It allows the local device to access or control remote components, and enables app collaboration in distributed scenarios. Its main functions are as follows:

-   Remote ability startup: Starts an ability on a remote device.
-   Remote ability migration: Migrates an ability to a remote device.
-   Remote ability binding: Binds an ability on a remote device.

## System Architecture<a name="section13587185873516"></a>

**Figure 1**  Distributed Scheduler architecture<a name="fig4460722185514"></a>


![](figures/dms-architecture.png)

## Directory Structure<a name="section161941989596"></a>

```
/foundation/ability
├── dmsfwk                      # DMS framework (DMS is short for Distributed Manager Service, also called the Distributed Scheduler Service)
├── dms_fwk_lite                # Lightweight DMS framework
```

## Repositories Involved<a name="section1371113476307"></a>

**Distributed Scheduler**

ability\_dms\_fwk

ability\_dms\_fwk\_lite