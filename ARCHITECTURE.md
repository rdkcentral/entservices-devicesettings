# RDK EntServices DeviceSettings - Architecture

## Overview

The DeviceSettings component is a Thunder plugin that exposes device settings and front-panel related functionality through the WPEFramework service model.

## System Architecture

```text
Client Applications
  -> JSON-RPC / COM-RPC
  -> Thunder Core
  -> DeviceSettings plugin layer
  -> DeviceSettings implementation layer
  -> Helper / DS / IARM / HAL layer
  -> Hardware and system services
```

## Core Components

### Plugin Layer

- `plugin/DeviceSettings/DeviceSettings.cpp` owns activation, deactivation, and external interface acquisition.
- `plugin/DeviceSettings/Module.cpp` and `Module.h` define the Thunder module identity.

### Implementation Layer

- `plugin/DeviceSettings/DeviceSettingsImplementation.cpp` owns the `Exchange::IDeviceSettings` contract.
- The component-specific implementation files delegate to the lower-level helpers and HAL adapters.

### Helper Layer

- `plugin/DeviceSettings/Audio.cpp`, `Display.cpp`, `Host.cpp`, `VideoPort.cpp`, `VideoDevice.cpp`, `HdmiIn.cpp`, and `CompositeIn.cpp` provide the device-specific logic.
- `DSController.cpp` and `DSPwrEventListener.cpp` coordinate system and power-state behavior.

## Build Model

The repository is structured as separate build targets for the Thunder shell and the implementation library, with the plugin configured from the repository root.

## Integration Points

- Thunder plugin lifecycle and service registration.
- COM-RPC access to the `Exchange::IDeviceSettings` interface and its subinterfaces.
- DS and IARM integration for hardware-backed operations.

## Testing

The component should be validated with the same layered approach used by the other entservices repositories: unit-style coverage for helper logic and integration coverage for the plugin entry point.