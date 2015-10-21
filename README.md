# Infinband Auxiliary Utilities

The OFED (https://www.openfabrics.org) stack provides several high quality output files to analyse the Infiniband fabric. These outputs have been observed to have incompatiblities across OFED versions and vendors. This project exists to write library to parse these output files and provide useful structures to process the data. Most of the data is read via regular expressions that have been tested on multiple levels of OFED outputs and it is expected that they will continue to need to be tweaked as output formats change.


