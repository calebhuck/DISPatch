# DISPatch

DISPatch is a small Qt/C++ DIS6 Simulation Management controller. It sends
state-transition commands over UDP and displays received component responses.
The UI defaults to a dark theme and includes dark, light, Gruvbox, One Dark,
VS Code default, Tokyo Night, and Dracula themes.

## Build Dependencies

Install the Qt development package and CMake toolchain. DISPatch defaults to
Qt6 when both Qt5 and Qt6 are available:

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel
```

Then build:

```bash
cmake -S . -B build
cmake --build build
```

To build against Qt5 for backward compatibility, set `DISPATCH_QT_MAJOR=5`:

```bash
sudo dnf install cmake gcc-c++ qt5-qtbase-devel
cmake -S . -B build-qt5 -DDISPATCH_QT_MAJOR=5
cmake --build build-qt5
```

Install with the default CMake prefix:

```bash
sudo cmake --install build
```

This installs `dispatch` to `/usr/local/bin`, the passive default
`dispatch.json` to `/usr/local/etc`, and a local-testing
`dispatch_debug.json` next to it.

Tests are optional and use Catch2:

```bash
cmake -S . -B build-tests -DDISPATCH_WITH_TESTS=ON
cmake --build build-tests
ctest --test-dir build-tests
```

CLion can import the included `CMakePresets.json`. The `default` and `tests`
presets use Qt6. Select `qt5-debug` for a Qt5 debug build profile, or `qt5` and
`qt5-tests` for the generic Qt5 profiles. Select the `tests` preset to
configure with `DISPATCH_WITH_TESTS=ON`; CLion will discover the CTest tests,
and the `check` build preset/target builds and runs them with failure output
enabled.

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

Set command defaults in `dispatch.json` so they match the simulation
component interface control document. The Start command supports
The Start command supports `realWorldTimeOffsetSeconds` and
`simulationTimeOffsetSeconds`, which both default to `0`. Real-world time is
encoded as an absolute DIS timestamp using the current UTC time plus
`realWorldTimeOffsetSeconds`. Simulation time is encoded as a relative DIS
timestamp starting from zero plus `simulationTimeOffsetSeconds`.

The `frozenBehavior` values in the `pause`, `stop`, and `reset` blocks are
written directly into the Stop/Freeze PDU Frozen Behavior field. DISPatch does
not interpret that byte locally; it tells receiving federates what behavior is
requested while frozen, according to their DIS/interface-control-document rules.

## Configuration

At startup, DISPatch first uses an explicit `--config path/to/dispatch.json` or
`--config=path/to/dispatch.json` when provided. Without `--config`, it looks for
`dispatch.json` in your home directory, then the configured system config
directory such as `/usr/local/etc`, then the current working directory, and then
next to the executable.

The config file supplies startup defaults for theme, network addresses and
ports, DIS entity IDs, command settings, and frozen behavior. The theme can be
`dark`, `light`, `gruvbox`, `onedark`, `vscode`, `tokyonight`, or `dracula`.

The network section also controls UDP socket behavior. `shareAddress` and
`reuseAddress` allow multiple processes to bind the same UDP port on one
machine when the platform supports it. `interfaceName` can pin socket binding
and multicast sends/joins to a specific network interface; when it is blank,
DISPatch selects a usable IPv4 interface and shows that selection in the UI.
The default destination is broadcast mode, so DISPatch sends to the selected
interface's IPv4 broadcast address unless the config or command line overrides
the destination.
`multicastInterfaceName` is still accepted as a legacy alias for
`interfaceName`. `joinMulticast` makes the receive socket join the configured
`multicastGroupAddress`; when that field is blank, DISPatch uses
`destinationAddress` if it is multicast. `multicastLoopback` controls whether
multicast sent by this host is looped back to local sockets. It defaults to
`false`; enable it for same-machine multicast testing, including multiple local
federates or the built-in test federate on one multicast DIS port.

The Network section also has Broadcast and Localhost destination modes. They
are mutually exclusive shortcuts that set the destination to the selected
interface's IPv4 broadcast address or `127.0.0.1`, select an appropriate
interface, and adjust UDP bind flags for the selected mode. In Broadcast mode,
changing the interface immediately updates the displayed destination address.

Network settings that are not exposed in the UI can be overridden from the
command line after the config is loaded:

```bash
dispatch --config /usr/local/etc/dispatch.json \
  --multicast-group 239.1.2.3 \
  --no-join-multicast \
  --multicast-loopback \
  --share-address \
  --reuse-address
```

The boolean forms are `--share-address`/`--no-share-address`,
`--reuse-address`/`--no-reuse-address`,
`--join-multicast`/`--no-join-multicast`, and
`--multicast-loopback`/`--no-multicast-loopback`.
`--multicast-group ADDRESS` and `--multicast-group-address ADDRESS` are
equivalent.

The optional `heartbeat` block enables liveness tracking:

```json
"heartbeat": {
  "enabled": true,
  "timeout": 5
}
```

`timeout` is measured in seconds and must be at least `1`. When heartbeat
tracking is enabled, each received Entity State PDU updates the status of its
entity ID. A Comment PDU does the same when its receiving entity matches the
manager ID. Live entities appear with a heart that pulses on each update and
changes to a skull when no update arrives before the timeout. Entity State and
Comment PDUs are ignored when heartbeat tracking is disabled. Entity State PDUs
do not contain a receiving entity, so receipt on the configured DIS network is
treated as being addressed to this manager.

The message table traces commands sent by this manager and matched federate
responses for requests sent in the current session. Unexpected PDUs, unmatched
responses, and Entity State or Comment PDUs used as heartbeats are omitted from
the table and message log. A response that otherwise looks intended for this
manager but has a bad or unknown request ID is warned in the event log.

The optional `log` section can mirror the UI logs to files. `logLevel` can be
`debug`, `warn`, or `error`; event log entries below that level are hidden from
the UI log and event log file. Warnings and errors are highlighted in the UI.
Set `logs` to true to append the filtered event log to `logFile`, and set
`messageLogs` to true to append the filtered PDU trace to `messageLogFile`.
Relative file paths are resolved under `--log-dir` when provided, otherwise
under the current working directory. The installed default config leaves file
logging disabled.

Config validation warnings are written to the application log at startup.
DISPatch reports unknown JSON keys, invalid address strings, invalid multicast
groups, unknown multicast interfaces, and suspicious network combinations such
as joining multicast while binding to a specific listen address or sharing one
UDP port without address reuse enabled.

## Local Test Federate

Set `testFederate.enabled` in `dispatch.json` to run in-process UDP responders
for local testing. The installed default config keeps them disabled; use
`--config /usr/local/etc/dispatch_debug.json` as a starting point for local
testing. When enabled, the UI shows a Test Federates status line with the bind
state and editable site/application/entity controls for each configured
federate. The entity IDs come from `testFederate.entityIds`. The responders
listen on the configured destination address and port, accept DIS6 Simulation
Management state-transition requests addressed to one of those IDs or to an
entity ID with `65535` wildcard fields, and send accepted responses back to the
manager:

- `Initialize`: Action Response PDU
- `Start`, `Pause`, `Stop`, and `Reset`: Acknowledge PDU

When both test federates and heartbeat tracking are enabled, each configured
federate periodically sends an empty Comment PDU addressed to the manager.
Select `Stop heartbeat` in the Test Federate box to stop those messages and
exercise the configured timeout; clear it to resume heartbeats.

When localhost mode puts the manager and test federate on the same unicast
address and port, DISPatch routes test-federate requests through the manager
socket. This avoids platform `SO_REUSEPORT` behavior that can otherwise deliver
a localhost datagram to only one of the two sockets.
