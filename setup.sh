#!/bin/bash
apt-get update
apt-get -y upgrade

apt-get install -y chrony git vim
git clone https://github.com/giangdinhtt/iot.git
cd iot

# install node-red and depedencies
#eval "bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)"
#npm install -g npm --unsafe-perm
#systemctl enable nodered.service
#cd ~/.node-red/
#npm install -g node-red-node-arduino --unsafe-perm
#npm install -g node-red-contrib-gpio --unsafe-perm
#npm install -g node-red-dashboard --unsafe-perm

# DS18b20 sensor
#npm install -g node-red-contrib-ds18b20-sensor --unsafe-perm
#npm install -g node-red-contrib-sensor-ds18b20 --unsafe-perm

# DHT sensor
#npm install -g node-red-contrib-dht-sensor --unsafe-perm

# SRF04 or SRF05 range finder
#npm install -g ode-red-node-pisrf --unsafe-perm

# Python depedencies
apt-get install -y python-pip python-virtualenv
python -m pip install --upgrade pip setuptools wheel
#pip install Adafruit_DHT

# Utilities
#apt-get install -y vim

# Mosquitto
apt-get install -y mosquitto mosquitto-clients
echo "listener 1883" >> /etc/mosquitto/conf.d/websocket.conf
echo "listener 1884" >> /etc/mosquitto/conf.d/websocket.conf
echo "protocol websockets" >> /etc/mosquitto/conf.d/websocket.conf
service mosquitto restart

# Free swap
#swapoff -a && swapon -a
# Rescan usb serial devices
#/etc/init.d/udev restart

# Start node-red
#node-red-pi


#sudo dpkg -P hplip; sudo apt-get update; sudo apt-get install hplip
