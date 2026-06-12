# RDK EntServices DeviceSettings - Product Functionality

## Product Overview

The DeviceSettings plugin provides a common service interface for device-level audio, display, host, video, HDMI-in, and front-panel configuration.

## Core Functionality

- Audio port and audio output control.
- Display and video-port configuration.
- HDMI-in and composite-in settings.
- Host and power-related device settings.
- Front-panel style indicator control where supported by the platform.

## API Surface

The primary public surface is exposed through the Thunder `Exchange::IDeviceSettings` interface and its related subinterfaces.

## Deployment

The plugin is packaged and loaded through the Thunder service model and follows the repository-level build configuration.