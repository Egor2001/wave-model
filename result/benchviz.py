#! /usr/bin/python

import sys
import matplotlib.pyplot as plt
import pandas as pd

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print('usage: ', sys.argv[0], ' DATA_1_CSV DATA_2_CSV DATA_3_CSV OUTPUT_PNG\n')
        sys.exit()

    name_1, name_2, name_3 = str(sys.argv[1]), str(sys.argv[2]), str(sys.argv[3])
    bench_1, bench_2, bench_3 = pd.read_csv(name_1), pd.read_csv(name_2), pd.read_csv(name_3)

    fig, ax = plt.subplots()
    ax.set_title('Время вычислений для 64 миллиардов узлов')
    ax.set_xlabel('Размер сетки, байт')
    ax.set_ylabel('Время, секунды')
    ax.plot(bench_1.Grid, bench_1.Time, 
            color='red', label='векторизация') # name_1.split(".")[0]
    ax.plot(bench_2.Grid, bench_2.Time, 
            color='blue', label='запуск в 4 потока') # name_2.split(".")[0]
    ax.plot(bench_3.Grid, bench_3.Time, 
            color='black', label='без оптимизации') # name_3.split(".")[0]
    ax.legend(loc='lower right')

    plt.savefig(str(sys.argv[4]))

