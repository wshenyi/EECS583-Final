PATH2LIB=cmake-build-debug/src/CUDAPrefetch.so
PASS=-cuda-prefetch
GPU_ARCH=sm_61
FLAGS=-g -std=c++11 --cuda-gpu-arch=${GPU_ARCH}
SOURCE=benchmarks/gaussian.cu
TARGET=result

# Derived Label
SOURCE_NAME=$(basename $(notdir ${SOURCE}))
DEVICE_IR=${SOURCE_NAME}-cuda-nvptx64-nvidia-cuda-${GPU_ARCH}.ll

apply:
	opt -enable-new-pm=0 -S -o ${SOURCE_NAME}-opt.ll -load ${PATH2LIB} ${PASS} < ${DEVICE_IR} > /dev/null

ll:
	clang++ ${FLAGS} -emit-llvm -S ${SOURCE}

claen:
	rm *.ll