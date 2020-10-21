#! /usr/bin/python

import sys
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print('usage: ', sys.argv[0], ' INPUT_FILE OUTPUT_PNG\n')
        sys.exit()

    mtx = np.loadtxt(str(sys.argv[1]))
    row_cnt, col_cnt = mtx.shape

    x = np.linspace(0.0, 1.0, col_cnt)
    y = np.linspace(0.0, 1.0, row_cnt)

    fig, axes = plt.subplots()
    xv, yv = np.meshgrid(x, y)
    axes = fig.add_subplot(111, projection='3d')
    axes.plot_surface(xv, yv, mtx)

    plt.savefig(str(sys.argv[2]))

