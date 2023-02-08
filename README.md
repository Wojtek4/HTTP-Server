# HTTP-Server
Fast HTTP Linux static file server.

## Configuration

In file `src/config.hpp` you can set port and working directory.

## Run
```
make
./server
```

## TODO:
- [ ] Add proper file handling:
  - [ ] Do not keep files in memory, leave sending files to kernel,
  - [ ] Check if file is in working directory,
  - [ ] Add handling of special characters in paths (e.g., spaces),
- [ ] Implement `src/http_checks.cpp`,
- [ ] Implement handling of partial data,
- [ ] Fix unnecessary closing of connections.
