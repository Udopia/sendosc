# Lightweight Algorithm Sonification

**SendOSC** is a lightweight, header-only implementation of the [Open Sound Control (OSC)](https://opensoundcontrol.stanford.edu/spec-1_0.html) protocol designed for real-time sonification of algorithmic data streams over UDP.

## 📌 Overview

Sonification is the process of translating data into auditory representations. SendOSC enables algorithm designers, software engineers, and researchers to analyze, debug, and monitor algorithm performance through continuous real-time auditory feedback.

## 🎧 Motivation: Why Sonify Algorithms?

Algorithm executions produce **time-series data** composed of internal state transitions, loop iterations, memory accesses, and dynamic events.
Unless recorded, these internal states are typically unobserved, as the computational result is the main focus for the majority of algorithm executions.

Engineers typically use textual logging or debugging tools to observe the execution traces of algorithmic implementations in their analyses.
Visualisations can map higher densities of internal state abstractions onto proximate perception, allowing us to observe qualitative and global behaviour.

Sonifications of internal state are tailored towards the human auditory system, which can have several advantages.
Providing auditory feedback can enrich online interactions with algorithmic systems by improving understanding and control.

Through its high temporal resolution, the human auditory system excels at distinguishing high-frequency temporal patterns that visual displays would miss.

## 🎯 Scope & Design

* Provides a typed, fluent interface for constructing OSC messages from OSC value types.
* Encodes OSC bundles and sends them over UDP in a lightweight, header-only, zero-dependency implementation.
* Leaves sound synthesis to external audio systems, such as SuperCollider, Pure Data, and Max/MSP.

## Command-line interface (mostly here for testing it)

The `sendosc` executable accepts a destination IPv4 address and UDP port followed
by one or more `ADDRESS TYPE VALUE` triples:

```sh
./sendosc 127.0.0.1 57120 /program/state i 1
./sendosc 127.0.0.1 57120 /program/state i 1 /program/progress f 0.5 /program/name s "hello world"
```

With no arguments, `./sendosc` retains the defaults: it sends `/test` with the
string `"abrakadabra"` and integer `42` to `127.0.0.1:5000`.

Each triple creates a separate OSC message with one argument. All messages are
sent together in a single OSC bundle. Supported types are `i` (signed 32-bit
decimal integer), `f` (finite 32-bit float, including scientific notation), and
`s` (string). Quote strings containing spaces; `""` sends an empty string.
Ports must be between 1 and 65535.

Use `./sendosc --help` to display usage. Invalid arguments or a bundle exceeding
the stream's 2048-byte buffer produce an error on standard error and a nonzero
exit status, without sending a partial bundle. UDP transmission does not
guarantee delivery to a listening receiver.

## ⚙️ The Sonification Pipeline

Translating algorithm execution dynamics into meaningful soundscapes requires a two-step separation of concerns:

```
+----------------------------+        OSC Stream        +-------------------------------+
|    Algorithm + SendOSC     |   ====================>  |   Digital Sound Synthesizer   |
|     (Data Selection)       |       (UDP Network)      |      (Acoustic Modeling)      |
+----------------------------+                          +-------------------------------+
```

1. **Data Selection (`SendOSC`)**
   * Identify internal data points, metrics, and event triggers during runtime.
   * Package and broadcast execution state as structured OSC messages over UDP.
2. **Acoustic Modeling (External Audio Engine)**
   * Receive incoming OSC streams.
   * Map incoming execution state messages to acoustic parameters (e.g., frequency, pitch, velocity, timbre, panning) using specialized software (e.g., SuperCollider, Pure Data, Max/MSP).
