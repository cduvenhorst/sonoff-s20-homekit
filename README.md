# sonoff-s20-homekit

An alternative firmware for the (10€) itead Sonoff S20 Socket. This firmware supports the Apple HomeKit framework and the QR-Code pairing for it (PoC).

Prerequisites for building OTA-Update-Images is the presence of [openssl](https://www.openssl.org) and the [esptool2](https://github.com/raburton/esptool2) on your system.

## Usage

1. Install [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk), build it with `make toolchain esptool libhal STANDALONE=n`, then edit your PATH and add the generated toolchain bin directory. The path will be something like /$HOME/path/to/esp-open-sdk/xtensa-lx106-elf/bin.

2. Checkout [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos) and set SDK_PATH environment variable pointing to it. E.g. export SDK_PATH=$HOME/espressif/esp-open-rtos

3. Initialize and sync all submodules (recursively):
```shell
git submodule update --init --recursive
```

4. Install required python modules for QR-Code creation:
```shell
pip install pyqrcode pypng
```

5. Create a new QR-Code for HomeKit pairing. Every device needs an unique QR-Code for identification on your network. Print it and stick it to the Sonoff S20 casing. WARNING: Since the information is update-persistent, you won`t be able to pair the device without reflashing after future OTA updates, if you don't save it or print it.:
```shell
make homekitSettings
```
6. Set your environment variables to match your needs:

  * Set ESPPORT environment variable pointing to USB device your ESP8266 is attached
     to (assuming your device is at /dev/tty.SLAB_USBtoUART):
     ```shell
     export ESPPORT=/dev/tty.SLAB_USBtoUART
     ```
  * Set your OTA-Update server environment variables.
    * Set the server

      ```shell
      export OTA_UPDATE_SERVER="192.168.1.2"
      ```
    * Set the port

      ```shell
      export OTA_UPDATE_PORT="8080"
      ```
    * Set the path on the server to the firmware files:

      ```shell
      export OTA_UPDATE_PATH="/ota/sonoff-s20"
      ```
    * Name your firmware files (defaults to "latest"):
      ```shell
      export OTA_UPDATE_FIRMWARE_NAME="sonoff-s20-v1.x"
      ```

7. To prevent any effects from previous firmware, it is highly recommended to erase flash:
    ```shell
        make erase_flash
    ```
    or, if you didn't set the ESPPORT environment variable:
    ```shell
        make erase_flash ESPPORT=/dev/tty.SLAB_USBtoUART
    ```
8. Flash the firmware on the Sonoff S20
```shell
    make -j4 test
```
  or
```shell
    make -j4 flash
    make monitor
```

### OTA-Updates
Again: Prerequisites for building OTA-Update-Images is the presence of [openssl](https://www.openssl.org) and the [esptool2](https://github.com/raburton/esptool2) on your system.

If you just (later) want to compile the OTA firmware images just enter
```shell
    make -j4 ota-images
```
and copy the compiled, linked and hashed firmware images
to your update server (using the path you have chosen at compilation time).

You'll find the files in the firmware/ota directory. Copy ALL files to your webserver directory accessible via your chosen update server path.

To initiate an OTA update of the device, just long-press the device button (4 secs and on) on an already flashed device.
