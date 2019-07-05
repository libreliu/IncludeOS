OLD_PS1="$PS1"
export OLD_PS1
PS1="(conanenv) $PS1"
export PS1
CC="aarch64-linux-gnu-gcc"
export CC
CXX="aarch64-linux-gnu-g++"
export CXX
CFLAGS="-mcpu=cortex-a53 -O2 -g"
export CFLAGS
CXXFLAGS="-mcpu=cortex-a53 -O2 -g"
export CXXFLAGS
PATH="/home/includeos/.conan/data/binutils/2.31/includeos/toolchain/package/1907037da8323f14f8235c9a3fabcb665a84c867/aarch64-elf/bin":"/home/includeos/.conan/data/binutils/2.31/includeos/toolchain/package/1907037da8323f14f8235c9a3fabcb665a84c867/bin"${PATH+:$PATH}
export PATH
