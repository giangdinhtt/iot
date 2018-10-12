#!/bin/bash
#apt-get install git
#git clone https://github.com/giangdinhtt/iot.git
apt-get update
apt-get upgrade

# install node-red and depedencies
eval "bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)"
npm install -g npm
sudo systemctl enable nodered.service
cd ~/.node-red/
npm install -g node-red-node-arduino --unsafe-perm
npm install -g node-red-contrib-gpio --unsafe-perm
npm install -g node-red-dashboard --unsafe-perm
npm install -g node-red-contrib-msg-queue --unsafe-perm
apt-get install python-pip python-virtualenv
python -m pip install --upgrade pip setuptools wheel
pip install Adafruit_DHT

# Start node-red
node-red-start
