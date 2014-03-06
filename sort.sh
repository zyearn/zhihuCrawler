#!/bin/bash

sort datafile/rawData.raw | uniq | sort -k3n,3
