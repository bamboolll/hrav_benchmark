#!/bin/bash
########## CONFIG REGISTER ##########################
wraxi 0x00000000 0x0ff2  # reset counter
#wraxi 0x00000000 0x0ff1
#wraxi 0x00000000 0x01000f01 # read from nf1, not infinitive receive
wraxi 0x00000000 0x0400FF01 # configure for benmark scanner
wraxi 0x00000000 0x0400FF41 # configure for benmark DMA


########## READ register ###########################
#rdaxi 0x00000000	# read config register
#rdaxi 0x00000001	# read number packet
#rdaxi 0x00000002	# read number user packet
#rdaxi 0x00000004	# read total received data size.
#rdaxi 0x00000005	# read number of packets each 10^6 CLK
#rdaxi 0x00000006	# read number of bytes each 10^6 CLK

