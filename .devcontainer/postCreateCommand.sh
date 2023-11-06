# Install mosquitto client
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa -y
sudo apt-get update && sudo apt-get install mosquitto-clients mosquitto ninja-build libmosquitto-dev uuid-dev libjson-c-dev libprotobuf-c-dev ca-certificates gpg wget -y

# Install CMake (https://apt.kitware.com/)
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt-get update
sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
sudo apt-get install kitware-archive-keyring
sudo apt-get install cmake -y

#Install step cli
wget https://github.com/smallstep/cli/releases/download/v0.24.4/step-cli_0.24.4_amd64.deb
sudo dpkg -i step-cli_0.24.4_amd64.deb
rm step-cli_0.24.4_amd64.deb