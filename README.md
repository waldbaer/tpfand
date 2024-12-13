tpfand
======

Daemon which keeps the CPU on a constant temperature by regulating the fan speed.

## Build

Build the binary:
```
cd build
./b_release.sh
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

## Use as systemd service
```
# Copy the systemd service tpfand.service file to /etc/systemd/system
sudo cp tpfand.service /etc/systemd/system

# Reload systemd services
sudo systemctl daemon-reload

# Enable and start new 'tpfand' service
systemctl enable --now
```

