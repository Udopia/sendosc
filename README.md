# Lightweight Algorithm Sonification

**SendOSC** is a lightweight, header-only implementation of the Open Sound Control (OSC) protocol designed for real-time sonification of algorithmic data streams over UDP.

## 📌 Overview

Sonification is the process of translating data into auditory representations. SendOSC enables algorithm designers, software engineers, and researchers to analyze, debug, and monitor algorithm performance through continuous real-time auditory feedback.

## 🎧 Motivation: Why Sonify Algorithms?

Algorithm executions naturally form **time-series data** composed of internal state transitions, loop iterations, memory accesses, and dynamic events. 

* **High Temporal Resolution:** The human auditory system excels at distinguishing high-frequency events and rapid temporal patterns that visual displays often miss.
* **Pattern & Anomaly Detection:** Real-time sonification makes subtle structural behavior, performance bottlenecks, dynamic phase transitions, and unexpected edge-case loops immediately perceptible.

## ⚙️ The Sonification Pipeline

Translating algorithm execution dynamics into meaningful soundscapes requires a two-step separation of concerns:

```
+---------------------------------+        OSC Stream        +----------------------------------+
|      Algorithm + SendOSC        |   ====================>  |     Digital Sound Synthesizer    |
|  (Data Selection & Extraction)  |       (UDP Network)      |   (Acoustic Modeling & Render)   |
+---------------------------------+                          +----------------------------------+
```

1. **Data Selection & Telemetry (`SendOSC`)**
   * Identify internal data points, metrics, and event triggers during runtime.
   * Package and broadcast execution state as structured OSC messages over UDP.
2. **Acoustic Modeling & Synthesis (External Audio Engine)**
   * Receive incoming OSC streams.
   * Map incoming execution state messages to acoustic parameters (e.g., frequency, pitch, velocity, timbre, panning) using specialized software (e.g., SuperCollider, Pure Data, Max/MSP).

## 🎯 Scope & Design

* Zero-dependency implementation designed for direct embedding into performance-critical C++ or C codebases with minimal overhead.
* Handles protocol encoding and network transfer, delegating acoustic rendering entirely to modern sound synthesis systems.
