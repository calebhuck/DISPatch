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
- `Standby`: Stop/Freeze PDU
- `Operate`: Start/Resume PDU
- `Shutdown`: Action Request PDU, default action ID `2`

Responses received on the configured listen address and port are decoded enough
to show the sender, PDU type, request ID, and a summary. Acknowledge and Action
Response PDUs are matched back to the request ID sent by the manager.

The Startup and Shutdown action IDs are editable in the UI so they can be
aligned with the simulation component interface control document when needed.

## Local Test Federate

Enable `Dummy simulation federate` in the UI to run an in-process UDP responder
for local testing. It listens on the configured destination address and port,
accepts DIS6 Simulation Management state-transition requests, and sends accepted
responses back to the manager:

- `Startup` and `Shutdown`: Action Response PDU
- `Standby` and `Operate`: Acknowledge PDU
