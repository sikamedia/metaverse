#!/bin/bash
set -ex
wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
tar -xzf zeromq-4.2.1.tar.gz
cd zeromq-4.2.1 && ./autogen.sh && ./configure && make && sudo make install && sudo ldconfig
