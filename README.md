# HTTP Server

This is an implementation of an http server from scratch in C++.
This is a learning experience that may result in a full fledged http server
for C++.

## Core Principles

This library should be an abstraction layer with reasonable assumptions.
It should be easy to modify.
It should be easy to understand and not assume the user is incompetent, but
promote knowledge of the internals instead of shielding them from it.

## Current system assumptions
- You are on linux
- You have ***make*** installed
- You have g++ as a compiler capable of running c++23


## Getting started
- ***make run***

## Todo
- add windows functionality
- add web socket implementation for hot reloading
- add route handling
- add better error handling
- add examples

## Documentation
- [TCP Docs](./documentation/tcp_docs.md)
- [HTTP Docs](./documentation/http_docs.md)
