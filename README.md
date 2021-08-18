# Veides SDK for C

[![Build Status](https://travis-ci.com/Veides/veides-sdk-c.svg?branch=master)](https://travis-ci.com/Veides/veides-sdk-c)

This repository contains code of Veides SDK for C language. It allows C developers to easily connect and interact with Veides platform.

**Jump to**:

* [Requirements](#Requirements)
* [Build](#Build)
* [Installation](#Installation)
* [Samples](#Samples)
* [Features](#Features)

## Requirements

* OpenSSL development package
* [CMake](https://cmake.org/) 3.5.0 or later
* [libcurl](https://github.com/curl/curl)
* [Paho MQTT C](https://github.com/eclipse/paho.mqtt.c) - optionally built and installed by this library

## Build

### Options available

* `VEIDES_WITH_STREAM_HUB_CLIENT` - Include building of Stream Hub client library. Default: `ON`
* `VEIDES_WITH_API_CLIENT` - Include building of API client library. Default: `ON`
* `VEIDES_BUILD_DEPENDENCIES` - Either build dependent libraries or use system libraries. Default: `OFF`
* `VEIDES_INSTALL_DEPENDENCIES` - Whether dependencies should be installed in the system or not. Default: `OFF`
* `VEIDES_BUILD_TESTS` - Set to ON to build the tests. Default: `OFF`
* `VEIDES_BUILD_SAMPLES` - To build samples or not to build. Default: `ON`

You can pass above configuration settings as command line options, for example to enable building dependencies and tests:

```bash
cmake .. -DVEIDES_BUILD_DEPENDENCIES=ON -DVEIDES_BUILD_TESTS=ON
```

### Build steps

To build Veides SDk from source, follow steps below:

```bash
git clone https://github.com/Veides/veides-sdk-c.git
cd veides-sdk-c
mkdir build && cd build
# Pass configuration settings if needed
cmake ..
make
```

## Installation

When library is built, you can install Veides SDK with:

```bash
sudo make install
```

To uninstall Veides SDK:

```bash
sudo make uninstall
```

> NOTE: If `VEIDES_INSTALL_DEPENDENCIES` option is set to `ON`, dependencies will automatically be installed by CMake after they're built.

## Samples

[Samples README](https://github.com/Veides/veides-sdk-c/blob/master/samples)

## Features

### Veides Stream Hub Client

- **SSL/TLS**: By default, this library uses encrypted connection
- **Auto Reconnection**: Client support automatic reconnect to Veides Stream Hub in case of a network issue

### Veides API Client

- **Methods operations**: Use your application to invoke methods on agent
