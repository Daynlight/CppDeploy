<div allign=center>

# CPP Deploy

</div>


## About


## TOC


## Installation
```bash
vcpkg install --x-install-root=./vendor
```
```bash
cmake -S . -B build -G Ninja && cmake --build build/ && sudo build/cpp_deploy/cpp_deploy 
```
```bash
cmake -S . -B build -G Ninja && cmake --build build/ && sudo cmake --install build
```
```bash
sudo systemctl daemon-reload
sudo systemctl enable cpp_deploy
sudo systemctl start cpp_deploy
```
```bash
sudo systemctl status cpp_deploy.service
sudo journalctl -u cpp_deploy -f
```

## Usage

