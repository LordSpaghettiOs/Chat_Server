# Chat Server

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Travis (.org)](https://travis-ci.org/caminek/Chat_Server.svg?branch=master)

Tested on:
- Currently re-testing under Linux

## About

Chat Server as a multi-threaded, command-line application that depending on the switches passed, will run as either a chat server or a chat client.  Due to the fact that all communications are passed in **PLAIN TEXT** its not recommended for use outside of a trusted network.

Tested on:
- Ubuntu 16.04 LTS (Xenial Xerus) with gcc 5.4.0 and cmake 3.12.4
- Ubuntu 18.04 LTS (Bionic Beaver) with gcc 7.4.0 and cmake 3.12.4
- OS X with Apple LLVM version 9.1.0 (clang-902.0.39.2) and cmake 3.11.4

### Usage
```
Usage: ./Chat_Server [OPTION]
  -S, --server              Start as server (default is client)
  -I, --ip                  IP address to use as server, or connect to as client
  -H, --hostname            Hostname to use as server, or connect to as client
  -P, --port                Port numberto use as server, or connect to as client
  -h, --help                Print this help text and exit
```


### Chat Commands
```
Server Commands:
  /w       <name> <message>   Send whisper
  /r                          Reply to whisper
  /who                        Show active clients
  /rename  <name>             Change nickname
  /help                       Shows this message
  /quit                       Quit chatroom
```

### Client Screenshots
<img src="c1_output.png" alt="Client 1 Output" height="342" width="496" />
<img src="c2_output.png" alt="Client 2" height="342" width="496" />

### Server Screenshot
<img src="server_output.png" alt="Server Output" height="325" width="506" />
