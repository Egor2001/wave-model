# wave-model
Iterative wave modelling

## metrics
CPU: Intel(R) Core(TM) i3-3110M 
Single-Threaded program

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

One possible explanation of these incorrelation is 
that vectorization is better for linear layout, 
so that cache misses may be not crucial in this case.
(this guess needs to be verified).

## build
```console
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release
```

## contacts
- telegram: [@geome\_try](https://t.me/geome_try)
- mail:
  - elchinov.es@phystech.edu
  - elchinov.es@gmail.com
