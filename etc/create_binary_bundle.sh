#! /bin/bash
. $INCLUDEOS_SRC/etc/set_traps.sh

# Paths
export INCLUDEOS_SRC=${INCLUDEOS_SRC:-~/IncludeOS}
export BUILD_DIR=${BUILD_DIR:-~/IncludeOS_build}	# Where the libs are built
export TEMP_INSTALL_DIR=${TEMP_INSTALL_DIR:-$BUILD_DIR/IncludeOS_TEMP_install}	# Libs are installed
export PATH="$TEMP_INSTALL_DIR/bin:$PATH"

# Build options
export ARCH=${ARCH:-i686} # CPU architecture. Alternatively x86_64
export BUNDLE_ARCHES=${BUNDLE_ARCHES:-i686 x86_64}
export TARGET=$ARCH-elf	# Configure target based on arch. Always ELF.
export num_jobs=${num_jobs:--j}	# Specify number of build jobs

# Version numbers
export binutils_version=${binutils_version:-2.28} #ftp://ftp.gnu.org/gnu/binutils
export musl_version=${musl_version:-v1.1.18}
export clang_version=${clang_version:-4.0}				# http://releases.llvm.org/
export llvm_branch=${llvm_branch:-release_40}

# Options to skip steps
[ ! -v do_binutils ] && do_binutils=1
[ ! -v do_musl ] && do_musl=1
[ ! -v do_libunwind ] && do_libunwind=1;
[ ! -v do_includeos ] &&  do_includeos=1
[ ! -v do_llvm ] &&  do_llvm=1
[ ! -v do_bridge ] &&  do_bridge=1
[ ! -v do_packages ] && do_packages=1

############################################################
# COMMAND LINE PROPERTIES:
############################################################

# Initialize variables:
install_yes=0

while getopts "h?y" opt; do
    case "$opt" in
    h|\?)
        printf "%s\n" "Options:"\
                "-y Yes: answer yes to install"\
        exit 0
        ;;
    y)  install_yes=1
        ;;
    esac
done

# Install build dependencies
DEPS_BUILD="build-essential make nasm texinfo clang-$clang_version clang++-$clang_version"

echo -e "\n\n >>> Trying to install prerequisites for *building* IncludeOS"
echo -e  "        Packages: $DEPS_BUILD \n"

if [ ! -z $do_packages ]; then
  sudo apt-get update
  sudo apt-get install -y $DEPS_BUILD
fi


# Print currently set install options
printf "\n\n>>> Bundle will be created with the following options:\n\n"
printf "    %-25s %-25s %s\n"\
	   "Env variable" "Description" "Value"\
	   "------------" "-----------" "-----"\
	   "INCLUDEOS_SRC" "Source dir of IncludeOS" "$INCLUDEOS_SRC"\
     "binutils_version" "binutils version" "$binutils_version"\
	   "musl_version" "musl version" "$musl_version"\
	   "clang_version" "clang version" "$clang_version"\
	   "llvm_branch" "LLVM version" "$llvm_branch"\
     "BUNDLE_ARCHES" "Build for CPU arches" "$BUNDLE_ARCHES"\

# Give user option to evaluate install options
if tty -s && [ $install_yes -eq 0 ]; then
	read -p "Is this correct [Y/n]?" answer
	answer=${answer:-"Y"}	# Default value
	case $answer in
		[yY] | [yY][Ee][Ss] )
			true;;
		[nN] | [n|N][O|o] )
			exit 1;;
		*) echo "Invalid input"
		   exit 1;;
	esac
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR


pushd $INCLUDEOS_SRC
tag=`git describe --abbrev=0`
filename_tag=`echo $tag | tr . -`
popd

# Where to place the installation bundle
DIR_NAME="IncludeOS_dependencies"
OUTFILE="${DIR_NAME}_$filename_tag.tar.gz"
BUNDLE_PATH=${BUNDLE_PATH:-$BUILD_DIR}

function do_build {
  echo -e "\n\n >>> Building bundle for ${ARCH} \n"
  # Build all sources
  if [ ! -z $do_binutils ]; then
    echo -e "\n\n >>> GETTING / BUILDING binutils (Required for cross compilation) \n"
    $INCLUDEOS_SRC/etc/build_binutils.sh
  fi

  if [ ! -z $do_musl ]; then
    echo -e "\n\n >>> GETTING / BUILDING MUSL \n"
    $INCLUDEOS_SRC/etc/build_musl.sh
  fi

  if [ ! -z $do_llvm ]; then
    echo -e "\n\n >>> GETTING / BUILDING llvm / libc++ \n"
    $INCLUDEOS_SRC/etc/build_llvm.sh
  fi


  #
  # Create the actual bundle
  #
  BUNDLE_DIR=$BUNDLE_PATH/$DIR_NAME
  BUNDLE_LOC=${BUNDLE_DIR}/$ARCH

  echo ">>> Creating Installation Bundle as $BUNDLE_LOC"

  musl=$TEMP_INSTALL_DIR/$TARGET/lib
  llvm=$BUILD_DIR/build_llvm
  libcpp=$llvm/lib/libc++.a
  libunwind=$llvm/lib/libunwind.a
  libcppabi=$llvm/lib/libc++abi.a

  # Includes
  include_musl=$TEMP_INSTALL_DIR/$TARGET/include
  include_libcxx=$llvm/include/c++/v1
  include_libunwind=$BUILD_DIR/llvm/projects/libunwind/include/

  # Make directory-tree
  mkdir -p $BUNDLE_LOC
  mkdir -p $BUNDLE_LOC/musl
  mkdir -p $BUNDLE_LOC/libcxx
  mkdir -p $BUNDLE_LOC/libunwind
  mkdir -p $BUNDLE_LOC/libunwind/include

  # Copy binaries
  cp $libcpp $BUNDLE_LOC/libcxx/
  cp $libcppabi $BUNDLE_LOC/libcxx/
  cp $libunwind $BUNDLE_LOC/libunwind/
  cp -r $musl $BUNDLE_LOC/musl/

  # Copy includes
  cp -r $include_musl $BUNDLE_LOC/musl/
  cp -r $include_libcxx $BUNDLE_LOC/libcxx/include
  cp $include_libunwind/libunwind.h $BUNDLE_LOC/libunwind/include
  cp $include_libunwind/__libunwind_config.h $BUNDLE_LOC/libunwind/include

  if [ $ARCH == "i686" ]
  then
    crtbegin=`$CC -m32 --print-file-name crtbegin.o`
    crtend=`$CC -m32 --print-file-name crtend.o`
  else
    crtbegin=`$CC --print-file-name crtbegin.o`
    crtend=`$CC --print-file-name crtend.o`
  fi

  mkdir -p $BUNDLE_LOC/crt
  cp $crtbegin $BUNDLE_LOC/crt/
  cp $crtend $BUNDLE_LOC/crt/

}

for B_ARCH in $BUNDLE_ARCHES
do
  export ARCH=$B_ARCH
  export TARGET=$ARCH-elf	# Configure target based on arch. Always ELF.
  do_build
done

# Zip it
tar -czvf $OUTFILE --directory=$BUNDLE_PATH $DIR_NAME

echo ">>> IncludeOS Installation Bundle created as $BUNDLE_PATH/$OUTFILE"

trap - EXIT
