PATH2PASS=cmake-build-debug/src/CUDAPrefetch.so
GPU_ARCH=sm_61
FLAGS=-g -std=c++11 --cuda-gpu-arch=${GPU_ARCH}
SOURCE=benchmarks/gaussian.cu
TARGET=result

# Derived Label
SOURCE_NAME=$(basename $(notdir ${SOURCE}))
DEVICE_IR=${SOURCE_NAME}-cuda-nvptx64-nvidia-cuda-${GPU_ARCH}.ll

all: compile

compile:
	clang++ ${SOURCE} -flegacy-pass-manager -Xclang -load -Xclang ${PATH2PASS} -o ${SOURCE_NAME}.out ${FLAGS} -L/usr/local/cuda/lib64 -lcudart_static -ldl -lrt -pthread

apply:
	opt -enable-new-pm=0 -S -o ${SOURCE_NAME}-opt.ll -load ${PATH2LIB} ${PASS} < ${DEVICE_IR} > /dev/null

ll.ptx:
	clang++ -stdlib=libstdc++ -S --cuda-gpu-arch=${GPU_ARCH} ${SOURCE}

ll:
	clang++ ${FLAGS} -emit-llvm -S ${SOURCE}

bc:
	clang++ ${FLAGS} -emit-llvm -c ${SOURCE}

asm:
	clang++ -cc1 -triple nvptx64-nvidia-cuda -aux-triple x86_64-unknown-linux-gnu -S -disable-free -clear-ast-before-backend -main-file-name axpy.cu \
	-mrelocation-model static -mframe-pointer=all -fno-rounding-math -fno-verbose-asm -no-integrated-as -aux-target-cpu x86-64 -fcuda-is-device -mllvm \
	-enable-memcpyopt-without-libcalls -mlink-builtin-bitcode /usr/local/cuda-11.8/nvvm/libdevice/libdevice.10.bc -target-feature +ptx75 \
	-target-sdk-version=11.5 -target-cpu sm_61 -mllvm -treat-scalable-fixed-error-as-warning -debug-info-kind=constructor -dwarf-version=2 \
	-debugger-tuning=gdb -fno-dwarf-directory-asm -v -resource-dir /usr/local/lib/clang/14.0.6 -internal-isystem /usr/local/lib/clang/14.0.6/include/cuda_wrappers \
	-include __clang_cuda_runtime_wrapper.h \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12 \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/x86_64-linux-gnu/c++/12 \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12/backward \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12 \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/x86_64-linux-gnu/c++/12 \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../include/c++/12/backward -internal-isystem /usr/local/lib/clang/14.0.6/include \
	-internal-isystem /usr/local/include -internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../x86_64-linux-gnu/include \
	-internal-externc-isystem /usr/include/x86_64-linux-gnu -internal-externc-isystem /include -internal-externc-isystem /usr/include \
	-internal-isystem /usr/local/cuda-11.8/include -internal-isystem /usr/local/lib/clang/14.0.6/include -internal-isystem /usr/local/include \
	-internal-isystem /usr/lib/gcc/x86_64-linux-gnu/12/../../../../x86_64-linux-gnu/include -internal-externc-isystem /usr/include/x86_64-linux-gnu \
	-internal-externc-isystem /include -internal-externc-isystem /usr/include -fdeprecated-macro -fno-autolink \
	-fdebug-compilation-dir=/home/wshenyi/eecs583/final -ferror-limit 19 -pthread -fgnuc-version=4.2.1 -fcxx-exceptions -fexceptions -fcolor-diagnostics \
	-cuid=8ddc1cd25f6e9af3 -D__GCC_HAVE_DWARF2_CFI_ASM=1 -std=c++11 -o ${SOURCE_NAME}.asm -x cuda ${SOURCE}

clean:
	rm -f *.ll *.bc *.ptx *.out *.s *.cubin