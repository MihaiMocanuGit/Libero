#!/bin/bash
# unofficial bash strict mode
set -euo pipefail
IFS=$'\n\t'

project="Libero"

build_type="release"
job_count=$(nproc)

Help()
{
   echo "Builds and runs the project."
   echo
   echo "Syntax: build.sh [-h|b|B|j] [ARGS]"
   echo "Options:"
   echo "    -h     Prints this help page."
   echo "    -b     Sets the build type (debug/release/relwithdebinfo/minsizerel)."
   echo "           DEFAULT: ${build_type}"
   echo "    -j     Sets the thread job count."
   echo "           DEFAULT: ${job_count}"
   echo "ARGS: run program with given arguments"
}

while getopts ":hb:Bj:" option; do
   case $option in
      h) # help
         Help
         exit;;
      b) # build type
         build_type=$OPTARG;;
      j) # job count
         job_count=$OPTARG;;
     \?) # Invalid option
         echo "Error: Invalid option"
         exit;;
   esac
done
shift $(expr $OPTIND - 1 ) # jump over all parsed arguments

if ! [[ "$build_type" =~ ^(debug|relwithdebinfo|minsizerel)$ ]]
then
    build_type="release"
fi

echo "build_type = $build_type"

build_dir="$(dirname $0)/build_$build_type"
echo "build_dir = $build_dir"

echo "Setting build options"
time (cmake  -DCMAKE_BUILD_TYPE="$build_type" -S . -B "$build_dir" \
    && echo "Building $build_type" && cmake --build "$build_dir" -j "$job_count")

