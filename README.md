#UFS
ufs (union file system) is a userspace filesystem that can be used
via a CLI. 

Instead of being a union on filesystem, ufs opts to unionize ufs "areas".

An area is a first class citizen in ufs, users may create/remove them, and
they may add files from the via COW.


##Summary
This repository contains the spec and sqlite implementation of
ufs (union file system). 

The first POC of ufs should behave as follows:

```
ufs init # Mount ufs on the current directory
ufs add area0 # Add an area called "area0" to ufs.
ufs set area0 BASE # Set the current ufs view to area0 first,
then the BASE file system second.
# Do some file system operations, reads follow union logic, while writes go to area0.

ufs collapse # Collapse all the modifications onto teh BASE filesystem.
```
##Roadmap

- [ x ] Package dependencies and create the build environment.
- [ x ] Write the ufs spec.
- [ x ] Write the ufs test suite.
- [   ] Write the in-memory sqlite implementation.
- [   ] Write the ufs protocol spec.
- [   ] Write the FUSE daemon.             
- [   ] Implement the CLI.       

By A.N.
