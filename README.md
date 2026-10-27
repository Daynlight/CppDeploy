<div align=center>

# CPP Deploy

</div>


## About
This is simple cpp deployment daemon for hosting dockers container on server, observe them, rerun and rollback.


## TOC
- [About](#about)
- [Installation](#installation)
  - [Install prerequistes](#install-prerequistes)
  - [Vcpkg dependencies](#vcpkg-dependencies)
  - [Build and run](#build-and-run)
  - [Build and install daemon](#build-and-install-daemon)
  - [Setting Action-Runner](#setting-action-runner)
  - [Privilages](#privilages)
- [Usage](#usage)
- [Prerequisites](#prerequisites)
- [License](#license) 


## Installation
### Install prerequistes
```bash
sudo apt install cmake ninja-build build-essential 
```
also install 
- [vcpkg](https://vcpkg.io/en/)
- [dokcer and docker compose](https://docs.docker.com/compose/install/)
### Vcpkg dependencies
```bash
vcpkg install --x-install-root=./vendor
```
### Build and run
```bash
cmake -S . -B build -G Ninja && cmake --build build/ && sudo build/cpp_deploy/cpp_deploy 
```
### Build and install daemon
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
### Setting Action-Runner
```bash
```
### Privilages
```bash
```


## Usage


## Prerequisites
- cmake
- ninja-build
- build-essential
- vcpkg
  - Boost.Assio
  - Gtests
  - Fmt
- docker
- dokcer compose
- github action-runner


## License
[GNU General v3](LICENSE)
