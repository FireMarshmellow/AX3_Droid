# AX3 — The Anxiety Bot 3000

Meet AX3 — my custom-built, 3D-printed Star Wars droid. The name? When I powered it on for the first time, the head immediately started jittering like it had full-blown anxiety. Hence: **Anxiety Bot 3000** → **AX3**.

---

## 🧠 What's in the Repo

This repo contains:

* **All the code** for the **remote control** and the **driver board**
* The **Gerber PCB files** for both boards

> ⚠️ Note: There are a couple of hardware issues with the PCBs (see below for fixes), and the design files are no longer available — only the final Gerbers.

### Videos:

* [Video 1 – Overview & First Look](https://youtu.be/6mtj2sq3ZnQ)
* [Video 2 – Detailed Walkthrough](https://youtu.be/tR4utJliNfc)
* [Video 3 – WIP: ESP-NOW & Current Electronics](https://youtu.be/)

---
## 🎨 Paint Job

Painted by **Jaime Serendipity**, who is working on a video about the process (no pressure!).

Find Jaime at:

* [YouTube – @](https://www.youtube.com/@_the_freak_factory_)*[the\_freak\_factory](https://www.youtube.com/@_the_freak_factory_)*
* [Instagram – @jaime\_serendipity](https://www.instagram.com/jaime_serendipity/)

---

### 🧊 3D Model

The model was designed by **Michael Baddeley** and is available via his [Patreon](https://patreon.com/mrbaddeley) as part of his **Microdroids Collection**.

---

## 🛠️ Hardware Breakdown

### 🎮 Remote Controller

A custom-built controller with:

* ESP32-C3 Beetle (DFRobot)
* Joystick module
* 5x 3mm push buttons
* 5x 10kΩ resistors
* 3.3V Li-ion battery
* Power switch
* 4x small screws

#### 🛠 Known Issues (Remote PCB)

* **Power switch doesn't cut battery power**:

  * ✅ **Fix**: Cut the trace and solder the switch to the ESP32-C3 Beetle's EN (enable) pin.

---

### 🤖 Inside the Droid

What’s packed in there:

* 4x 18650 batteries (still on the first charge – sorcery?)
* ESP32 dev board
* Mini 3A DC-DC step-down module
* 2x DC N20 motors (yep, they spin at different speeds... character!)
* DRV8833 dual-channel motor driver
* 9g servo motor
* RGB LED
* Voice module (integrated MP3 player + speaker)
* On/off switch

#### 🛠 Known Issues (Driver Board PCB)

* **No power to ESP32 from buck converter**:

  * ✅ **Fix**: Manually solder wires from the 5V and GND rails of the buck converter to the ESP32.

> 🗂️ The **AX3_Borad_and_remote.zip** for both PCBs is included in the repo root.

---

## 🔌 Software Overview

* Uses ESP-NOW for communication
* Remote sends:

  * Joystick XY values
  * Button state as bitfield
* No fancy optimizations — but it works

---