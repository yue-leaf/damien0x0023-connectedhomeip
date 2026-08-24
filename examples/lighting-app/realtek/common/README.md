# Matter Lighting Example Application

This reference application implements an On/Off Light device type. You can use
this example as a reference for creating your own application.

The example is based on:

-   [Matter](https://github.com/project-chip/connectedhomeip)
-   [RTL87x2G github SDK](https://github.com/rtkconnectivity/rtl87x2g_sdk)
-   [OT-REALTEK](https://github.com/rtkconnectivity/ot-realtek)

## Supported devices

-   [RTL8777G]

## Introduction

The application showcases a light bulb device that communicates with clients
over a low-power, 802.15.4 Thread network.

It can be commissioned into an existing Matter network using a controller such
as `chip-tool`.

This example implements a `Standard commissioning flow`, meaning devices not
connected to the network automatically broadcast advertising when powered on.
The initial commissioning is done through `ble-thread` pairing method.

The Thread network dataset will be transferred on the device using a secure
session over Bluetooth LE.To pair successfully, the commissioner must know the
commissioning information corresponding to the device: setup passcode and
discriminator. This data is usually encoded within a QR code or printed to the
UART console.

## Prerequisites for building

In order to build the example, it is recommended to use a Linux distribution.
Please visit the supported Operating Systems list in
[BUILDING.md](../../../../docs/guides/BUILDING.md#prerequisites).

-   Make sure that below prerequisites are correctly installed (as described in
    [BUILDING.md](../../../../docs/guides/BUILDING.md#prerequisites))

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

-   Step 1: checkout Realtek specific submodules only

    ```bash
    $ scripts/checkout_submodules.py --shallow --platform realtek --recursive
    ```

-   Step 1: Init REALTEK SDK

    ```bash
    $ git submodule update --init --recursive third_party/openthread/ot-realtek
    ```

-   Step 3: activate local environment

    ```bash
    $ source ./scripts/activate.sh
    ```

    If the script says the environment is out of date, you can update it by
    running the following command:

    ```bash
    $ source ./scripts/bootstrap.sh
    ```

## Building

-   To build the demo application:

    ```bash
    $ ./scripts/build/build_examples.py --target realtek-rtl8777g-lighting build
    ```

    You can find the target built file called **matter-cli-ftd** under the
    `out/realtek-rtl8777g-lighting/bin` directory.

### Production DAC build requirement

Production images that use a DAC key persisted by the TrustZone Secure App
must configure `matter_require_persistent_dac_key=ON`. The Realtek CMake bridge
propagates the matching Matter GN argument automatically, so the application
and platform library cannot select different DAC paths. The build must also set
`matter_secure_dac_cmse_import_library` to the reviewed RTL8777G Secure App CMSE
import library; configuration fails if it is missing. This selects Secure App
ABI operation `0x110`. The build must also enable TrustZone and Factory Data. In
this profile Factory Data contains the DAC certificate, PAI, certification
declaration, and commissioning data, but no DAC private key. The raw-key path
is retained only for development images. Existing `CONFIG_DAC_KEY_ENC` builds
continue to use the legacy encrypted-key Secure App import path when persistent
on-chip provisioning is not selected.

## More information

For more information on our product line and support options, please visit
[www.realmcu.com](https://www.realmcu.com/)
