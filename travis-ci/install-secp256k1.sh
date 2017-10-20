#!/bin/bash
set -ex
git clone https://github.com/mvs-live/secp256k1
cd secp256k1 && ./autogen.sh && ./configure --enable-module-recovery && make -j8 && sudo make install && sudo ldconfig