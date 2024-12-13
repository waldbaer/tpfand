tpfand
======

Daemon which keeps the CPU on a constant temperature by regulating the fan speed.

## Build

Build the binary:
```
cd build
./b_release.sh
make -j
```

Location of compiled binary: `build/src/tpfand`

## Run it

To see if the regulator works best run the binary with verbose options:
```
$> sudo ./tpfand --verbose
Config options:
Nom.Temp: 65.00°C   kP=-0.1250   kD=0.2500   Sampling time: 1s   Regulation prescaler: 5s
Temp: 63.00  dT: -3.50  regP: -0.25  regD: -0.88  corr: -1.12  fanLevel: 0
Temp: 72.00  dT: 9.00  regP: 0.88  regD: 2.25  corr: 3.12  fanLevel: 3
Temp: 69.00  dT: -3.00  regP: 0.50  regD: -0.75  corr: -0.25  fanLevel: 3
Temp: 68.00  dT: -1.00  regP: 0.38  regD: -0.25  corr: 0.12  fanLevel: 3
Temp: 64.00  dT: -4.00  regP: -0.12  regD: -1.00  corr: -1.12  fanLevel: 2
...
```

## Tweak Configuration

`tpfand` will create a default configuration file at `/etc/tpfand.conf` during first run.
It might be necessary to adapt the sensors or fan device paths.
In addition also the regulator parameters can be adapted.

The default settings have been verified to work with:
- **Ubuntu 24.10**
- **Lenovo T420s**


## Enable ACPI
It might be necessary that man fan control must be enabled with kernel parameters.

If the reported fan level updates of `./tpfand --verbose` can be observed please add the following kernel options file `/etc/modprobe.d/thinkpad_acpi.conf`:
```
options thinkpad_acpi fan_control=1
```

Reboot the system.

## Use as systemd service
```
# Copy the systemd service tpfand.service file to /etc/systemd/system
sudo cp tpfand.service /etc/systemd/system

# Reload systemd services
sudo systemctl daemon-reload

# Enable and start new 'tpfand' service
systemctl enable --now
```

