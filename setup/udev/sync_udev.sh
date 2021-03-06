#!/bin/bash
set -e

if [ -z $BLOCKS_HOME ]; then
    echo "BLOCKS_HOME must be defined as an environment variable"
    exit 1
fi

echo "Copying udev rules..."
echo "   99-blocks.rules"
sudo cp $BLOCKS_HOME/setup/udev/99-blocks.rules /etc/udev/rules.d/

echo "Copying latency fix..."
sudo mkdir -p /etc/udev/scripts
echo "   set_ftdi_low_latency.sh"
sudo cp $BLOCKS_HOME/setup/udev/set_ftdi_low_latency.sh /etc/udev/scripts/

echo "Reloading UDev rules..."
sudo udevadm control --reload-rules

echo "DONE!"
echo "Please unplug and replug all your devices for changes to take effect."

