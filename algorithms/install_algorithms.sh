#!/bin/bash

# Get TigerGraph version
tg_ver=$(gadmin version | awk '/^TigerGraph version:/ { print $3 }')

# Install thirdparty libraries
cp -r word2vec /home/tigergraph/tigergraph/app/${tg_ver}/dev/gdk/gsdk/include/thirdparty/
cp -r eigen /home/tigergraph/tigergraph/app/${tg_ver}/dev/gdk/gsdk/include/thirdparty/

# Install GSQL algorithms
gsql install_algorithms.gsql
