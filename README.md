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
cmake -S . -B build -G Ninja -DCPP_DEPLOY_INSTALL=ON  && cmake --build build/ && sudo cmake --install build
```
```bash
sudo systemctl daemon-reload
sudo systemctl enable cpp_deploy
sudo systemctl restart cpp_deploy
```
```bash
sudo systemctl status cpp_deploy.service
sudo journalctl -u cpp_deploy -f
```


## Usage
Add your service in `/usr/local/etc/cpp_deploy/services` folder
```txt
NAME = Service Name;
DESCRIPTION = Description of your Service;
LOCAL_PATH = Path to service versioning;
ENV_PATH = Path to env file;
REPO_URL = URL to github repository;
BRANCH = Branch;
GITHUB_TOKEN = Github Access Tokken;
RUN_CMD = cd backend/ && docker compose -f docker-compose-prod.yaml up -d --build;
STOP_CMD = cd backend/ && docker compose -f docker-compose-prod.yaml down;
STATUS_CMD = cd backend/ && [ -n "$(docker compose -f docker-compose-prod.yaml ps -q)" ] && [ -z "$(docker compose -f docker-compose-prod.yaml ps --filter status=exited -q)" ];
TEST_CMD = cd backend/backend/ && npm install && npm test;
```
Update happends after api call on `localhost:9923/api/update` the best way is settings `github action-runner` to minimize server exposure.


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

