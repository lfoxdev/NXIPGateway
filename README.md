# NetworX IP Gateway

A DIY module based on ESP32 that connects to the bus of NX series alarm panels (NetworX/Interlogix/Caddx...) and acts as a gateway between panel and LAN.
Connection is made directly to the panel's keypad&expansion bus with a simple circuit, NX-584 card or built-in RS232 are not required!

<img width="1024" height="768" alt="NXIPGW" src="https://github.com/user-attachments/assets/489383a2-5efe-4988-bf5b-43e31587b3d5" />

# Features

 - **Web Interface** for configuring the module, monitoring panel status, viewing panel system log and performing commonly used keypad functions
 - **MQTT** support:
	 - State topics which are published on system, partition and zones state changes 
	 - System log topic updating on new records
	 - Command topics for arming, disarming, panic, bypassing (compatible with Home Assistant MQTT Alarm control panel template)
 - Supports **NX-584** protocol (binary mode, over TCP) for integration with existing home automation applications (*not all commands are supported currently*)
 - Accepts **DL-900** encrypted TCP/IP connections (NX-590E emulation) for panel programming

# NetworX protocol

This project includes a [description of the NetworX bus protocol](Doc/NetworX%20Protocol%20Description.pdf), derived through reverse engineering and cross-referencing with the original [NX-584 protocol specification](Doc/Caddx_NX-584_Communication_Protocol.pdf). Keypad command codes and certain annotations are sourced from the ELK Magic Module documentation.
All commands were obtained from the NX-6v2 panel (version 1.13), but are likely compatible with at least the entire NXv2 series (4, 6, and 8).
> [!NOTE]
> The list of commands, their purpose, and field descriptions are incomplete. If you have a panel of other models or expanders, you can help the project by recording and providing a snippet of traffic for analysis.

# Required hardware

 - ESP32 with minimum of 4 MB flash
	 > [!TIP]
	 > WT32-ETH01 module with Ethernet is recommended. Modules with Wi-Fi only should also be supported, but may not be sufficiently tested.
	> [!NOTE]
	> ESP8266 will not be supported as the code requires more memory and uses FreeRTOS capabilities

 - [Simple circuit for coupling NetworX bus with UART](Doc/Interface.pdf)
 - DC/DC step down converter to power up module from panel POS terminal
	 > [!TIP]
	 > It is recommended to power ESP module directly with 3.3V rather than 5V due to low load capacity of the onboard LDO.

# Firmware
The firmware is provided as a source code for the PlatformIO environment, as well as in the form of binary files (WT32-ETH01 version with Ethernet support and general ESP32 version for modules without Ethernet).

## First installation

### Flashing ESP32

 1. Connect ESP32 module to PC with built-in USB or 3.3V UART-to-USB adapter (e.g., FT232RL, CP2102, or CH340G).
 2. Install the corresponding driver. You should see a COM port device in Device Manager.
 3. Enter flashing (bootloader) mode:
	 - Modules with USB usually enter flashing mode automatically. If module does not, but has `BOOT` and `RST` keys, then press and hold `BOOT`, press and release `RST` and finally release `BOOT`.
	 - For modules without USB and keys connect `IO0` pin to `GND` and then power cycle module.
4. After flashing, disconnect `IO0` from `GND` (if connected) and reboot the board with `RST` key or power cycle it.

### Installation from pre-compiled binaries (via Web Browser, no software required)

1.  Download full firmware image (file named `xxx_full.bin`) from the Releases page.
2.  Open  [ESP Web Tool](https://esptool.spacehuhn.com/) in your browser and follow instructions. If prompted for a flash address (offset), enter **`0x0`**.


### Installation from source code

1. Install [**VS Code**](https://code.visualstudio.com/download).
2. Install [**PlatformIO**](https://docs.platformio.org/en/latest/integration/ide/vscode.html) extension.
3. Clone project folder and open it with PlatformIO (Alien icon on the left sidebar -> Open project).
4. Wait for the platform and libraries to be installed
5. On the bottom panel, select the target environment (`wt32-eth01` or `esp32dev`), choose the appropriate COM port and click **Upload**.

## Setup

 - Ethernet version
	1. Connect module directly to PC's ethernet port
	2. Switch PC's ethernet adapter IPv4 address to manual mode, set IP to `192.168.1.100` and mask to `255.255.255.0`
	3. Go to http://192.168.1.200. Default credentials are admin/caddx.
- WiFi version
	1. Connect to access point `NX Gateway`
	2. Go to http://192.168.4.1 (or open the suggested web page). Default credentials are admin/caddx.
	> [!NOTE]
	> Currently, Ethernet always uses static IP defined in WebUI, and WiFi always uses DHCP

## Updating

You can update module firmware via web interface. Download the latest OTA firmware image (`xxx_ota.bin`),  press Gear icon on the bottom of the web interface navigation panel, select downloaded file and click **Update**.
If you are building from source, upload file `.pio/build/XXX/firmware.bin`.
Arduino OTA is also available (password is the same as in web interface)

# License
### 1. Non-Commercial Use (Free) 
For personal, educational, hobbyist, and other non-commercial purposes, this firmware and its source code are licensed under the **PolyForm Noncommercial License 1.0.0**. 
* You are free to download, compile, modify, and flash this firmware onto your own devices for non-commercial use. 
*  You **may not** use this software, its source code, or pre-compiled binaries in commercial products, paid services, or within commercial organizations without obtaining a separate commercial license. See the [LICENSE](LICENSE.md) file for the full text of the PolyForm Noncommercial License 1.0.0. 

### 2. Commercial Use
If you intend to use this firmware in a commercial product, pre-flash it onto hardware sold to customers, or use it in a commercial environment, please contact the author.
### Third-Party Components 
This repository includes modified versions of the following open-source libraries, which retain their respective original licenses: 
* [**CRMui3**](https://github.com/WonderCRM/CRMui3) — Licensed under the Apache License 2.0 (original copyright belongs to WonderCRM; modified for this project). 
* [**3DES Implementation**](https://github.com/B-Con/crypto-algorithms) — by Brad Conte, released into the Public Domain. 

Other external dependencies defined in `platformio.ini` are governed by their respective licenses.

Documentation for NetworX original panels and modules is provided for informational purposes, the rights to it belong to its authors.
