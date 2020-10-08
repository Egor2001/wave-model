#!/usr/bin/python
import os

def gen_zorder_mask(hfile, nbytes):
    mask = 0x5555555555555555;
    hfile.write('static constexpr uint64_t ZOrderMask = ', mask, ';\n')

def gen_zoffset_mtx(hfile, nbytes, nvals):
    mtx = [offset_lst(1 << log, nvals) for log in range(8 * nbytes)]

    hfile.write('static constexpr uint64_t ZOffsetMtx[nvals][] = {\n')
    for lst in mtx:
        hfile.write('{ \n')
        for val in lst:
            hfile.write('val, ')
        hfile.write('},\n')
    hfile.write('};\n')

def offset_lst(nskip, nvals):


if __name__ == '__main__':
    filename = 'zcurve_constants.h'
    with open(filename) as hfile:
        gen_zorder_mask(hfile)
        gen_zoffset_mtx(hfile)
