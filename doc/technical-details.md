Peso [PSO] Technical Details
===================================

This document outlines the technical implementation details for Peso. It should be of use to advanced users and developers.

Specifications
--------------

* 21 million coins in total
* 2 minute average block time
* 12.5 coins per block
* Block reward halves 
* Retargets difficulty using DarkGravityWave
* x11 Proof-of-Work algorithm

Port numbers
------------

The following port numbers are used by Peso.

* P2P uses port 16118
* RPC uses port 16117 

Message start string
--------------------

The message start string used by Peso is:

```
0xfe, 0xc2, 0xb3, 0xee
```
