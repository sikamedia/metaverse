#!/bin/bash
set -ex
wget -c http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz
tar -xzf boost_1_56_0.tar.gz
cd boost_1_56_0 && ./bootstrap.sh && ./b2 -j 4 && sudo ./b2 install && sudo ldconfig
