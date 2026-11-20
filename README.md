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

### Configured handle validation

VideoPort, Audio, Display, and VideoDevice keep their existing `int32_t` handle
interfaces. Handle-dependent operations resolve incoming values against
configured type/index pairs (configuration-vector indices for VideoDevice)
before accessing HAL or handle-specific state, then forward the matched native
`intptr_t` handle. Unknown handles, failed configured lookups, and ambiguous
native/truncated-handle collisions fail closed. Acquisition methods, including
`GetVideoPort`, retain their existing narrowing behavior.

Display connection/surround queries use video-port handles; EDID, aspect ratio,
ALLM, and AVI operations use display handles. Only an explicit unsupported
display lookup is skipped. VideoDevice accepts zero when HAL returns it for a
configured device.

Audio validation is selective: global mixing, fader, language, format, and
Atmos-output operations retain their existing behavior. Mixer levels,
enable-persistence settings, and HDMI ARC port-ID queries remain unchanged.
The zero-handle Atmos-capability query still requests host capability; nonzero
port-capability queries are validated.

Audio, VideoPort, Display, and VideoDevice cache configured identities and
native handles after successful initialization of their owning HAL module.
Repeated validation scans only in-memory entries, with no configuration reload
or handle-acquisition HAL calls. A failed cache build publishes no partial
results and is retried on the next handle-based operation. Display skips only
explicit unsupported display lookups; a successfully enumerated all-unsupported
display domain remains cached as unavailable. Its video-port queries share
VideoPort's owning cache rather than keeping potentially stale copies.
VideoPort's platform-initialized flag is stored on the same instance as its
cache and mutex, so Display callers in other translation units observe the
owning instance's initialization state rather than a header-local flag.
Cache access and invalidation are mutex-protected; termination clears each
owning module's cache, and reinitialization rebuilds it. This relies on handles
and configuration remaining stable within one plugin/HAL initialization
lifetime. Unknown and ambiguous handles remain rejected, VideoDevice native
zero remains valid, and acquisition APIs are unchanged.

Audio level getters/setters and VideoPort resolution getters/setters support
index zero only because their legacy cache/persistence state is not indexed.
These operations reject configured nonzero indices with an error log and
`ERROR_GENERAL` after handle resolution, before operation-specific HAL calls,
caller-output writes, shared cache updates, persistence, or notifications.
The guards apply even when persistence is disabled: volume reads/writes update
shared last-volume state, and resolution reads can use shared fallback caches.
Other handle-dependent operations retain configured nonzero-index resolution;
this is not full per-port state or persistence support.

Focused resolver regressions are in the existing L1 test source. They have not
been built or run for this reapplication. Membership checks do not provide
caller authorization or atomic protection against concurrent HAL termination.

### HDMI-input port bounds (RDKEMW-25487)

EDID ALLM and VRR cache getters/setters reject ports below `dsHDMI_IN_PORT_0`
or at/above `dsHDMI_IN_PORT_MAX` before indexing per-port arrays. The EDID-version
setter validates before HAL/persistence changes, and the VRR helper validates
before its capability-cache read. Invalid getter outputs remain unchanged;
valid-port behavior is preserved. Focused bounds regressions are included in
the existing L1 test source but have not been built or run.

### SoC/HAL failure handling

The RDKEMW-25488 EDID success gate already exists: failed `dsGetEDID` output is
not filtered, marked cache-valid, dumped, or converted for the caller.

The approved cross-module follow-up also checks HDMI EDID-version reads,
Audio capability/ducking-level queries, LE initialization, and live FPD
brightness reads. Failed reads do not become successful default results;
speaker current-level reads are required only during active ducking. Ordinary
volume writes go directly to the setter without a prerequisite getter call;
persisted FPD brightness queries retain their existing cached-value behavior.
Failed HDMI initialization/input-count lookup leaves initialization incomplete,
and failures applying the persisted force-disable-4K setting are logged.

Display, CompositeIn, VideoPort, and VideoDevice mark callback setup complete
only after their required registrations succeed. Other independent callbacks
are still attempted after one fails. A later callback-setup call can retry;
there is no new background retry loop.

Existing explicit legacy policies for DFC/HDR persistence, post-change events,
optional symbols, and cached fallbacks are not replaced with blanket
return-on-error behavior. These follow-up changes have only been
source/whitespace inspected, not built or HAL-failure-injection tested.

### Remediation logging

Basic logs are always enabled: `DSLOG_INFO` reports module handle-cache builds,
successful enumeration, and invalidation; existing `DSLOG_ERR`/`DSLOG_WARN`
messages report rejected handles/ports, failed HAL calls, and incomplete setup.
Successful per-call validation stays debug-only to avoid normal-path log spam.

Define `DEBUG_LOGGING` when compiling to enable the existing `DEBUG_LOG` macro.
It reports cache readiness and in-memory reuse, enumeration progress, matching
and collision decisions, resolved type/index, valid HDMI bounds, HAL-output
handling, and callback setup details. The existing macro emits through
`DSLOG_INFO` when enabled and compiles out otherwise; no new runtime switch or
build option is introduced. Added logs omit native handle addresses and payload
contents.