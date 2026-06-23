# DISPatch

DISPatch is a small Qt5/C++ DIS6 Simulation Management controller. It sends
state-transition commands over UDP and displays received component responses.
The UI defaults to a dark theme and includes dark, light, and Gruvbox themes.

## Build Dependencies

On RHEL 8 or newer, install the Qt5 development package and CMake toolchain:

```bash
sudo dnf install cmake gcc-c++ qt5-qtbase-devel
```

Then build:

```bash
cmake -S . -B build
cmake --build build
```

Tests are optional and use Catch2:

```bash
cmake -S . -B build-tests -DDISPATCH_WITH_TESTS=ON
cmake --build build-tests
ctest --test-dir build-tests
```

CLion can import the included `CMakePresets.json`. Select the `tests` preset
to configure with `DISPATCH_WITH_TESTS=ON`; CLion will discover the CTest
tests, and the `check` build preset/target builds and runs them with failure
output enabled.

The project also includes a Conan 2 recipe. The `tests` option controls the
same CMake variable:

```bash
conan build . -o tests=True
```

## DIS6 Command Mapping

The application uses standard DIS6 Simulation Management PDU layouts:

- `Initialize`: Action Request PDU, default action ID `39` for initialize internal parameters
- `Start`: Start/Resume PDU
- `Pause`: Stop/Freeze PDU, reason `recess`
- `Stop`: Stop/Freeze PDU, reason `termination`
- `Reset`: Stop/Freeze PDU, reason `stop_for_reset`

Responses received on the configured listen address and port are decoded enough
to show the sender, PDU type, request ID, and a summary. Acknowledge and Action
Response PDUs are matched back to the request ID sent by the manager.

Set command defaults in `DISPatch_config.json` so they match the simulation
component interface control document. The Start command supports
`realWorldTimeOffsetSeconds` and `simulationTimeOffsetSeconds`, which default
to `0` and schedule the Start/Resume PDU clock-time fields relative to the
current UTC time. Set `useLiteralZero` to `true` in the `start` block when a
zero Start/Resume offset should be written as a literal zero clock-time value
instead of the current UTC time.

## Configuration

At startup, DISPatch looks for `DISPatch_config.json` in the current working
directory and then next to the executable. You can pass an explicit path with
`--config path/to/DISPatch_config.json`.

The config file supplies startup defaults for theme, network addresses and
ports, DIS entity IDs, command settings, and frozen behavior. The theme can be
`dark`, `light`, or `gruvbox`.

The network section also controls UDP socket behavior. `shareAddress` and
`reuseAddress` allow multiple processes to bind the same UDP port on one
machine when the platform supports it. `interfaceName` can pin socket binding
and multicast sends/joins to a specific network interface; when it is blank,
DISPatch selects a usable IPv4 interface and shows that selection in the UI.
`multicastInterfaceName` is still accepted as a legacy alias for
`interfaceName`. `joinMulticast` makes the receive socket join the configured
`multicastGroupAddress`; when that field is blank, DISPatch uses
`destinationAddress` if it is multicast. `multicastLoopback` keeps local
multicast traffic visible on the same machine, which is useful when several
federates are running locally on one DIS port.

The Network section also has Broadcast and Localhost destination modes. They
are mutually exclusive shortcuts that set the destination to `255.255.255.255`
or `127.0.0.1`, select an appropriate interface, and adjust UDP bind flags for
the selected mode.

The message table is a socket-level PDU trace. It shows manager and in-process
test federate sends, and every datagram each socket actually receives. No
self-looped multicast or localhost packets are hidden or synthesized.

The optional `log` section can mirror the UI logs to files. `logLevel` can be
`debug`, `warn`, or `error`; event log entries below that level are hidden from
the UI log and event log file. Warnings and errors are highlighted in the UI.
Set `logs` to true to append the filtered event log to `logFile`, and set
`messageLogs` to true to append the unfiltered PDU trace to `messageLogFile`.
Relative file paths are resolved next to the loaded config file.

Config validation warnings are written to the application log at startup.
DISPatch reports unknown JSON keys, invalid address strings, invalid multicast
groups, unknown multicast interfaces, and suspicious network combinations such
as joining multicast while binding to a specific listen address or sharing one
UDP port without address reuse enabled.

## Local Test Federate

Set `testFederate.enabled` in `DISPatch_config.json` to run an in-process UDP
responder for local testing. When enabled, the UI shows a Test Federate status
line with the bind state and editable site/application/entity controls. The
entity ID comes from `testFederate.entityId`. It listens on the configured
destination address and port, accepts DIS6 Simulation Management
state-transition requests addressed to that entity ID, and sends accepted
responses back to the manager:

- `Initialize`: Action Response PDU
- `Start`, `Pause`, `Stop`, and `Reset`: Acknowledge PDU
