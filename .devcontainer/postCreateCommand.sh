# Install mosquitto client
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa -y
sudo apt-get update && sudo apt-get install mosquitto-clients mosquitto ninja-build libmosquitto-dev libjson-c-dev -y

#Install step cli
wget https://github.com/smallstep/cli/releases/download/v0.24.4/step-cli_0.24.4_amd64.deb
sudo dpkg -i step-cli_0.24.4_amd64.deb
rm step-cli_0.24.4_amd64.deb