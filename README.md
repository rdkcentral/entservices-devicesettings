# entservices-devicesettings

This repository contains the Thunder plugin for `DeviceSettings`.

## Layout

- `plugin/DeviceSettings/` contains the Thunder shell, the implementation library, and the DS helper classes.
- `cmake/` contains the local find-modules needed by the component build.
- `build_dependencies.sh` bootstraps the local Thunder build dependencies used by this repository.
- `cov_build.sh` runs the coverage-oriented configuration used by CI.

## Build Flow

The repository follows the same split as the frontpanel component architecture:

1. The Thunder plugin layer owns activation, service registration, and JSON-RPC wiring.
2. The implementation layer exposes the `Exchange::IDeviceSettings` surface.
3. The helper layer wraps the DS / IARM / HAL-specific logic.

## Notes

- The component is built from the repository root through the top-level `CMakeLists.txt`.
- New plugin flags or workflow changes should be mirrored in the build scripts when the component matrix changes.
- Thunder notification payloads for HDMI/composite video-mode updates and resolution changes are value-initialized before their fields are populated and delivered to registered clients. This safely initializes strings and scalar members without clearing C++ objects with `memset`.
- HDCP system-state broadcasts zero the entire IARM event structure before setting the HDCP state ID and state, so unused payload bytes are not sent as uninitialized data.