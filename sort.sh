#!/bin/bash

sort datafile/rawData.raw | uniq | sort -nrk 3,3
