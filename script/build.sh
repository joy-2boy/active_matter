#!/bin/bash
# Check if a file name is provided as the first argument
if [ -z "$1" ]; then
  echo "Usage: $0 <source file> [-g] [-p]"
  exit 1
fi

# Define the source and include directories
SRC_DIR=/data2/xsli/active_matter/active_md_lib/src
INC_DIR=/data2/xsli/active_matter/active_md_lib/include

GSL_INC_DIR=/data2/xsli/gsl/include/ # GSL headers are here
GSL_LIB_DIR=/data2/xsli/gsl/lib/     # GSL libraries are here

# Extract the directory and file name from the first argument
INPUT_FILE=$1
BUILD_DIR=$(dirname "$INPUT_FILE")

# Initialize flags
DEBUG_FLAG=""
PROFILE_FLAG=""

# Check for -g option as the second argument for debugging
if [ "$2" == "-g" ] || [ "$3" == "-g" ]; then
  DEBUG_FLAG="-g"
fi

# Check for -p option as the second or third argument for profiling with gprof
if [ "$2" == "-p" ] || [ "$3" == "-p" ]; then
  PROFILE_FLAG="-pg"
fi

# Define the output executable name
OUTPUT_EXECUTABLE=${BUILD_DIR}/job

# Compile the input file
echo "Compiling $INPUT_FILE..."
gcc -c -o "${BUILD_DIR}/main.o" "$INPUT_FILE" -I"${INC_DIR}" -I"${GSL_INC_DIR}" $DEBUG_FLAG $PROFILE_FLAG

# Compile each source file in the src directory
for file in ${SRC_DIR}/*.c; do
    # Extract the basename of the file
    base=$(basename "${file}" .c)
    gcc -c -o "${BUILD_DIR}/${base}.o" "${file}" -I"${INC_DIR}" -I"${GSL_INC_DIR}" $DEBUG_FLAG $PROFILE_FLAG
done

# Linking
echo "Linking..."
#  gcc -o "${OUTPUT_EXECUTABLE}" "${BUILD_DIR}"/*.o    将多个目标文件（.o文件）和库一起合并生成最终的可执行文件
# 链接静态库 libgsl.a 和 libgslcblas.a  (动态链接一般是 .so ).
# 最终编译好的文件只有 -lm 是动态链接的，可移植性比较强
gcc -o "${OUTPUT_EXECUTABLE}" "${BUILD_DIR}"/*.o "${GSL_LIB_DIR}/libgsl.a" "${GSL_LIB_DIR}/libgslcblas.a" -lm $DEBUG_FLAG $PROFILE_FLAG

# Clean up .o files
echo "Cleaning up..."
rm "${BUILD_DIR}"/*.o

echo "Build completed: ${OUTPUT_EXECUTABLE}"
