# DISPatch

DISPatch is a small Qt5/C++ DIS6 Simulation Management controller. It sends
state-transition commands over UDP and displays received component responses.
The UI defaults to a dark theme and includes a light theme selector.

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

The Startup and Shutdown action IDs are editable in the UI so they can be
aligned with the simulation component interface control document when needed.
The Standby Stop/Freeze reason and frozen behavior fields are also editable.

## Configuration

At startup, DISPatch looks for `DISPatch_config.json` in the current working
directory and then next to the executable. You can pass an explicit path with
`--config path/to/DISPatch_config.json`.

The config file supplies the startup defaults for the editable UI fields:
theme, network addresses and ports, DIS entity IDs, action IDs, and the
Standby Stop/Freeze reason and frozen behavior. The sample config uses
`"reason": "recess"`, but the reason can also be a numeric DIS value.

## Local Test Federate

Set `testFederate.enabled` in `DISPatch_config.json` to run an in-process UDP
responder for local testing. When enabled, the UI shows a Test Federate status
line with the bind state. It listens on the configured destination address and
port, accepts DIS6 Simulation Management state-transition requests, and sends
accepted responses back to the manager:

- `Startup` and `Shutdown`: Action Response PDU
- `Standby` and `Operate`: Acknowledge PDU
