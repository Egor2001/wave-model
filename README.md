# wave-model
Iterative wave modelling

## build
```console
$ mkdir -p build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
Requires modern C++17 GNU/Clang compiler to be used

## docs
```console
$ mkdir -p build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make docs
```
See docs in docs/html/index.html with your browser
Requires Doxygen to be installed

## run
```console
$ ./run.sh
```
See visualiation result in out/fig.png

## metrics
CPU: Intel(R) Core(TM) i3-3110M 
Single-Threaded program

### Time (profiled with Intel VTune)

#### Linear vs Z-order data layout
![layout-bench](result/result_sync_noavx/time_bench.png?raw=true)

#### No-AVX vs AVX computations for linear layout 
![avx-bench](/result/result_sync_avx/time_bench.png?raw=true)

### FLOPS (estimated)
peak = ~20 Gflops
(DS - domain size)

data layout | DS = 2^8 | DS = 2^10 | DS = 2^12 
------------|----------|-----------|----------
linear | 0.79 Gflops | 0.80 Gflops | 0.60 Gflops
Z-order | 0.77 Gflops | 0.75 Gflops  | 0.68 Gflops

### L1 cache miss rate (cachegrind)
domain size = 2^10  

data layout | rd rate | wr rate
------------|---------|--------
linear | 7.5 % | 14.6 %
Z-order | 0.8 % | 6.1 %

## contacts
- telegram: [@geome\_try](https://t.me/geome_try)
- mail:
  - elchinov.es@phystech.edu
  - elchinov.es@gmail.com
