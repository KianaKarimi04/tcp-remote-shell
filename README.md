# TCP Remote Shell (C)

A multi-client TCP remote shell implemented in C using IPv4 sockets.  
The server executes client commands using fork() and exec(), supports basic I/O redirection, and returns stdout/stderr over a network connection.

## Overview

This project simulates a remote command execution environment where clients connect to a server over TCP. The server handles multiple client connections and executes shell commands securely in separate processes.

The goal of this project was to gain a deeper understanding of:
- TCP/IP networking
- Process creation and management
- System calls in Unix/Linux
- Client-server architecture

## Features

- Multi-client TCP server
- IPv4 socket communication
- fork() and exec() for command execution
- Basic input/output redirection
- Returns stdout and stderr to client
- Concurrent client handling

## Technologies

- C
- POSIX Sockets
- Linux System Calls
- TCP/IP Networking

## Key Concepts Demonstrated

- Socket programming
- Process management
- Inter-process communication
- Command parsing
- Error handling in network systems

## How to Run

1. Compile the server and client:
gcc server.c -o server
gcc client.c -o client

2. Run the server:
./server

3. Connect using client:
./client <server_ip> <port>


## What I Learned

This project strengthened my understanding of how operating systems handle processes and how TCP-based communication works at a low level. It also improved my debugging skills in concurrent and networked environments.
