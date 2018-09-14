sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git
git clone https://github.com/giangdinhtt/iot.git
# install node-red and depedencies
bash <(curl -sL https://raw.githubusercontent.com/node-red/raspbian-deb-package/master/resources/update-nodejs-and-nodered)
sudo systemctl enable nodered.service
cd ~/.node-red/
npm install node-red-node-arduino
sudo apt-get install python-pip python-virtualenv
sudo python -m pip install --upgrade pip setuptools wheel
sudo pip install Adafruit_DHT

