sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git
git clone https://github.com/giangdinhtt/iot.git
wget https://nodejs.org/dist/v8.12.0/node-v8.12.0-linux-armv7l.tar.xz
tar -xvf node-v8.12.0-linux-armv7l.tar.xz
cd node-v8.12.0-linux-armv7l/
sudo cp -R * /usr/local/
sudo npm install -g --unsafe-perm node-red
cd ~/.node-red/
npm install node-red-node-arduino
sudo apt-get install python-pip python-virtualenv
sudo python -m pip install --upgrade pip setuptools wheel
sudo pip install Adafruit_DHT

