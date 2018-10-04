# temperature_monitoring
Average temperature monitoring system using Sky TelosB motes

This a code for ContikiOS to be run on as many TelosB motes as you'd like, the code currently supports
a mesh network of motes but that can be scaled easily by using larger data structures.

The motes running this code would constantly keep calculating the average temperature of the field they're
deployed in. The temperature can be found in an interesting way by pressing the user button on the motes. 

The same code needs to be run on however motes you wish to deploy and the code is designed to be self aware
to detect if new motes are deployed or if some were taken off the field.

Please refer to the PDF in the repository for a better understanding.
