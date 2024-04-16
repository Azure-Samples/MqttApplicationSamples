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

#Install rust
# curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh
sudo apt install libssl-dev build-essential cmake

export K3D_FIX_MOUNTS=1

k3d registry create registry.localhost --port 5500

k3d cluster create -i ghcr.io/jlian/k3d-nfs:v1.25.3-k3s1 \
--registry-use k3d-registry.localhost:5500 \
-p '2883:2883@loadbalancer' \
-p '8883:8883@loadbalancer' 