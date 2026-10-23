# The CUDA toolkit, when installed (sudo jk-cuda install): nvcc on PATH, its
# libraries (cuBLAS, ...) found by the programs built with it.
if [ -x /usr/local/cuda/bin/nvcc ]; then
    export PATH="$PATH:/usr/local/cuda/bin"
    export LD_LIBRARY_PATH="/usr/local/cuda/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    # jk_os's GCC (16) is newer than the ones CUDA lists as supported.
    export NVCC_APPEND_FLAGS="${NVCC_APPEND_FLAGS:--allow-unsupported-compiler}"
fi
