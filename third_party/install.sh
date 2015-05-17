#!/bin/bash

# prefix=/usr/local

prefix=`dirname "$0"`
prefix=`cd "$prefix"; pwd`

cd build

tar -zxf sparsehash-2.0.2.tar.gz
tar -zxf gflags-2.1.1.tar.gz
tar -xzf gperftools-2.1.tar.gz
tar -xzf boost_1_56_0.tar.gz

#
cd sparsehash* && ./configure -prefix=$prefix && make -j  && make install && cd ..
cd boost* && ./bootstrap.sh && ./b2 install -j20 --prefix=$prefix && cd ..
cd gflags* && mkdir _build; cd _build && cmake -D CMAKE_INSTALL_PREFIX=../../../ ../ ../. && make && make install && cd ../..
cd gperftools* && ./configure --enable-frame-pointers -prefix=$prefix && make -j  && make install && cd ..
cd ..
