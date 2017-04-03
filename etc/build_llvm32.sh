#! /bin/bash
. $INCLUDEOS_SRC/etc/set_traps.sh

# Download, configure, compile and install llvm

newlib_inc=$TEMP_INSTALL_DIR/x86_64-elf/include	# path for newlib headers
IncludeOS_posix=$INCLUDEOS_SRC/api/posix
libcxx_inc=$BUILD_DIR/llvm/projects/libcxx/include
libcxxabi_inc=$BUILD_DIR/llvm/projects/libcxxabi/include

# Install dependencies
sudo apt-get install -y cmake ninja-build subversion zlib1g-dev libtinfo-dev

cd $BUILD_DIR

download_llvm=${download_llvm:-"1"}	# This should be more dynamic

if [ ! -z $download_llvm ]; then
    # Clone LLVM
    git clone -b release_39 git@github.com:llvm-mirror/llvm.git
    #svn co http://llvm.org/svn/llvm-project/llvm/tags/$LLVM_TAG llvm

    # Clone libc++, libc++abi, and some extra stuff (recommended / required for clang)
    pushd llvm/projects
    git checkout release_39

    # Compiler-rt
    git clone -b release_39 git@github.com:llvm-mirror/compiler-rt.git
    #svn co http://llvm.org/svn/llvm-project/compiler-rt/tags/$LLVM_TAG compiler-rt

    # libc++abi
    git clone -b release_39 git@github.com:llvm-mirror/libcxxabi.git
    #svn co http://llvm.org/svn/llvm-project/libcxxabi/tags/$LLVM_TAG libcxxabi

    # libc++
    git clone -b release_39 git@github.com:llvm-mirror/libcxx.git
    #svn co http://llvm.org/svn/llvm-project/libcxx/tags/$LLVM_TAG libcxx

    # libunwind
    git clone -b release_39 git@github.com:llvm-mirror/libunwind.git
    #svn co http://llvm.org/svn/llvm-project/libunwind/tags/$LLVM_TAG libunwind

    # Back to start
    popd
fi

# Make a build-directory
mkdir -p build_llvm
pushd build_llvm

if [ ! -z $clear_llvm_build_cache ]; then
    rm CMakeCache.txt
fi

TRIPLE=x86_64-pc-linux-elf
CXX_FLAGS="-std=c++14 -msse3 -mfpmath=sse"

# CMAKE configure step
#
# Include-path ordering:
# 1. IncludeOS_posix has to come first, as it provides lots of C11 prototypes that libc++ relies on, but which newlib does not provide (see our math.h)
# 2. libcxx_inc must come before newlib, due to math.h function wrappers around C99 macros (signbit, nan etc)
# 3. newlib_inc provodes standard C headers
cmake -GNinja $OPTS  \
      -DCMAKE_CXX_FLAGS="$CXX_FLAGS -I$IncludeOS_posix -I$libcxxabi_inc -I$libcxx_inc -I$newlib_inc " $BUILD_DIR/llvm \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_C_COMPILER=clang-$clang_version \
      -DCMAKE_CXX_COMPILER=clang++-$clang_version \
      -DTARGET_TRIPLE=$TRIPLE \
      -DLLVM_BUILD_32_BITS=OFF \
      -DLLVM_INCLUDE_TESTS=OFF \
      -DLLVM_ENABLE_THREADS=OFF \
      -DLLVM_DEFAULT_TARGET_TRIPLE=$TRIPLE \
      -DLIBCXX_ENABLE_SHARED=OFF \
      -DLIBCXX_ENABLE_THREADS=OFF \
      -DLIBCXX_TARGET_TRIPLE=$TRIPLE \
      -DLIBCXX_BUILD_32_BITS=OFF \
      -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
      -DLIBCXX_CXX_ABI=libcxxabi \
      -DLIBCXXABI_TARGET_TRIPLE=$TRIPLE \
      -DLIBCXXABI_ENABLE_THREADS=OFF \
      -DLIBCXXABI_HAS_PTHREAD_API=OFF


# MAKE
ninja libc++abi.a
ninja libc++.a

popd

trap - EXIT
