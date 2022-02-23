# ec - Enhanced C

ec is an advanced C library which provides relative high level functions
for users to create program in C programming language.

## Features

### Containers

- vector
- list
- stack
- set
- map

### Algorithms

- iterate
- find (WIP)
- sort (WIP)

### Misc

- bytestring
- string
- encoding
- formatter

### Error Handling

- Exception

## Usage

1. Create a directory to contain 3rd-party libraries (such as ec)

```
mkdir deps
cd deps
```

2. Add submodule

```
git submodule add ssh://git@<repo-address>/y2c2/ec.git ec
```

3. Update sub modules

```
git submodule foreach git pull
```
