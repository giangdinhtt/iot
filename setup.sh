#!/bin/bash
apt-get update
apt-get upgrade

apt-get install git
git clone https://github.com/giangdinhtt/iot.git
cd iot

# install node-red and depedencies
eval "bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)"
npm install -g npm --unsafe-perm
sudo systemctl enable nodered.service
cd ~/.node-red/
#npm install -g node-red-node-arduino --unsafe-perm
npm install -g node-red-contrib-gpio --unsafe-perm
npm install -g node-red-dashboard --unsafe-perm

# DS18b20 sensor
#npm install -g node-red-contrib-ds18b20-sensor --unsafe-perm
npm install -g node-red-contrib-sensor-ds18b20 --unsafe-perm

# DHT sensor
#npm install -g node-red-contrib-dht-sensor --unsafe-perm

# SRF04 or SRF05 range finder
npm install -g ode-red-node-pisrf --unsafe-perm

apt-get install python-pip python-virtualenv
python -m pip install --upgrade pip setuptools wheel
pip install Adafruit_DHT

# Free swap
#swapoff -a && swapon -a
# Rescan usb serial devices
#/etc/init.d/udev restart

# Start node-red
node-red-pi


#sudo dpkg -P hplip; sudo apt-get update; sudo apt-get install hplip