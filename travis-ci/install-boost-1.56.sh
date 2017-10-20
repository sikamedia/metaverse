#!/bin/bash
set -ex
wget -c http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz
tar -xzvf boost_1_56_0.tar.gz
cd boost_1_56_0 && ./bootstrap.sh && sudo ./b2 install
