# Architecture Overview

The backend has been reorganized around a modular architecture with clear responsibilities:

- core: orchestrates server lifecycle and startup/shutdown
- net: socket and connection handling
- http: request parsing and response building
- router: route registration and dispatch
- middleware: request interception and pipeline execution
- threadpool: concurrency for client handling
- tests: regression tests for core protocol behaviors

This layout improves cohesion and makes it easier to evolve the framework toward production-style capabilities such as keep-alive, static file serving, and dynamic routing.
