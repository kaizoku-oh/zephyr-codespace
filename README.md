# zephyr-online-workspace
A Zephyr RTOS codespace based on devcontainer

## Usage

### Uncomment these lines to flash your device

```jsonc
"runArgs": [
    // Access USB port from the docker container
    "--device=/dev/bus/usb/"
  ]
```

### If flashing doesn't work restart the docker service and try again

```bash
sudo systemctl restart docker
```

```bash
# Make sure the build is clean
user@480c36b20b00:/workdir$ rm -rf /workdir/app/build

# Build the app
user@480c36b20b00:/workdir$ west build /workdir/app -d /workdir/app/build -p auto -b nucleo_f767zi

# Flash the app
user@480c36b20b00:/workdir$ west flash -d /workdir/app/build

# Debug the app with gdb
user@480c36b20b00:/workdir$ west debug --build-dir app/build/

# west debug openocd commands
/opt/toolchains/zephyr-sdk-0.16.8/sysroots/x86_64-pokysdk-linux/usr/bin/openocd
    -s /workdir/zephyr/boards/arm/nucleo_f767zi/support
    -s /opt/toolchains/zephyr-sdk-0.16.8/sysroots/x86_64-pokysdk-linux/usr/share/openocd/scripts
    -f /workdir/zephyr/boards/arm/nucleo_f767zi/support/openocd.cfg
    -c 'tcl_port 6333'
    -c 'telnet_port 4444'
    -c 'gdb_port 3333'
    -c '$_TARGETNAME configure -rtos Zephyr' '-c init' '-c targets' '-c halt'

/opt/toolchains/zephyr-sdk-0.16.8/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb
    -ex 'target extended-remote :3333' app/build/zephyr/zephyr.elf
    -ex load

# cortex-debug openocd command
/opt/toolchains/zephyr-sdk-0.16.8/sysroots/x86_64-pokysdk-linux/usr/bin/openocd
    -c "gdb_port 50000"
    -c "tcl_port 50001"
    -c "telnet_port 50002"
    -s /workdir/zephyr/boards/arm/nucleo_f767zi/support
    -f /home/user/.vscode-server/extensions/marus25.cortex-debug-1.12.1/support/openocd-helpers.tcl
    -f /workdir/zephyr/boards/arm/nucleo_f767zi/support/openocd.cfg
```

## Windows 11

Install docker on WSL2 following this link https://learn.microsoft.com/en-us/windows/wsl/tutorials/wsl-containers

## Limitation
`ZEPHYR_BASE` environment variable is hard coded in the docker image

## ✅ ToDo

- [x] Build an STM32 app

- [x] Flash app using openocd

- [x] Debug app using vscode cortex-debug

- [x] Restructure workspace to be compatible with the hard coded `ZEPHYR_BASE` path
