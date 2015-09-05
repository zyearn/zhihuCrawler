#!/bin/bash

#sort datafile/rawData.raw | uniq | sort -nrk 3,3
sort -u -t' ' -k6,6  datafile/rawData.raw | sort -nrk 3,3
