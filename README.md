# Kswish engine - Pluggable Network Payload Dedupe Engine

The kswish engine is a pluggable network payload dedupe engine
which takes 4K blocks in the order of 256 blocks and transfers
only blocks that are only unique

##Source Code

~~~{.sh}
git clone https://github.com/krchug/kswish.git
cd kswish
~~~

## Prerequisites

~~~{.sh}
sudo ./scripts/pkgdep.sh
~~~

## Buiild the workspace

~~~[.sh]
scons -c
scons
~~~

## Use poc to determine the dedupe cacpacity of your file

If you have a huge file and want to check if it has dedupe blocks
and the engine can be used in your usecase, update the file in
poc.c and the size and run it


~~~[.sh]
sudo ./poc
Total chunks 207

Total unique blocks 50436 which amounts to 197MB of data

Total dedupe blocks 2555 which amounts to 9MB of data
~~~
