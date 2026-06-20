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

## DIS6 Command Mapping

The application uses standard DIS6 Simulation Management PDU layouts:

- `Startup`: Action Request PDU, default action ID `1`
- `Standby`: Stop/Freeze PDU, default reason `recess`
- `Operate`: Start/Resume PDU
- `Shutdown`: Action Request PDU, default action ID `2`

Responses received on the configured listen address and port are decoded enough
to show the sender, PDU type, request ID, and a summary. Acknowledge and Action
Response PDUs are matched back to the request ID sent by the manager.

Set the Startup and Shutdown action IDs and the Standby Stop/Freeze reason in
`DISPatch_config.json` so they match the simulation component interface control
document.

## Configuration

At startup, DISPatch looks for `DISPatch_config.json` in the current working
directory and then next to the executable. You can pass an explicit path with
`--config path/to/DISPatch_config.json`.

The config file supplies startup defaults for theme, network addresses and
ports, DIS entity IDs, action IDs, and the Standby Stop/Freeze reason and
frozen behavior. The sample config uses `"reason": "recess"`, but the reason
can also be a numeric DIS value. The theme can be `dark`, `light`, or
`gruvbox`.

The network section also controls UDP socket behavior. `shareAddress` and
`reuseAddress` allow multiple processes to bind the same UDP port on one
machine when the platform supports it. `joinMulticast` makes the receive
socket join the configured `multicastGroupAddress`; when that field is blank,
DISPatch uses `destinationAddress` if it is multicast. `multicastInterfaceName`
can pin multicast joins to a specific network interface; leave it blank to let
the OS choose. `multicastLoopback` keeps local multicast traffic visible on the
same machine, which is useful when several federates are running locally on one
DIS port.

Config validation warnings are written to the application log at startup.
DISPatch reports unknown JSON keys, invalid address strings, invalid multicast
groups, unknown multicast interfaces, and suspicious network combinations such
as joining multicast while binding to a specific listen address or sharing one
UDP port without address reuse enabled.

## Local Test Federate

Set `testFederate.enabled` in `DISPatch_config.json` to run an in-process UDP
responder for local testing. When enabled, the UI shows a Test Federate status
line with the bind state and editable site/application/entity controls. The
startup ID comes from `testFederate.entityId`. It listens on the configured
destination address and port, accepts DIS6 Simulation Management
state-transition requests addressed to that entity ID, and sends accepted
responses back to the manager:

- `Startup` and `Shutdown`: Action Response PDU
- `Standby` and `Operate`: Acknowledge PDU
