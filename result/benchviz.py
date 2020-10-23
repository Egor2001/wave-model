#! /usr/bin/python

import sys
import matplotlib.pyplot as plt
import pandas as pd

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print('usage: ', sys.argv[0], ' DATA_1_CSV DATA_2_CSV OUTPUT_PNG\n')
        sys.exit()

    name_1, name_2 = str(sys.argv[1]), str(sys.argv[2])
    bench_1, bench_2 = pd.read_csv(name_1), pd.read_csv(name_2)

    fig, ax = plt.subplots()
    ax.set_title('time benchmark [~1 billion stencil computations]')
    ax.set_xlabel('grid size, bytes')
    ax.set_ylabel('time, seconds')
    ax.plot(bench_1.Grid, bench_1.Time, 
            color='red', label=name_1.split(".")[0])
    ax.plot(bench_2.Grid, bench_2.Time, 
            color='black', label=name_2.split(".")[0])
    ax.legend()

    plt.savefig(str(sys.argv[3]))

