# OpenICC

The OpenICC configuration data base allows to story, share and manipulate
colour management informations.

Part of that is a file format based on JSON and a implementation of a 
according library for easier access.

## Features
Currently are serialised and parsed device informations.

## Internet
git:  git://openicc.git.sourceforge.net/gitroot/openicc/openicc (read-only)

## Dependencies
Yajl - a JSON parser library http://lloyd.github.com/yajl/

## Building
    $ make
    $ make install

    A optional make check is provided.

## Install
Copy the resulting library into your local library path if not to use
the sources inside your code.

## Usage
The library provides an easy to use API for DB access. Include the 
openicc_config.h header file into your sources and link to the object file
generated from openicc_config.c.
### Example
See the openicc_config_example.c file.

## Known Bugs
The source code provides currently no mechanism for a write lock.

