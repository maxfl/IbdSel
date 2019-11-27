#!/usr/bin/env python

"""Simple python macro to run stage1_main.cc"""

from load import ROOT as R
from dayabay_filename import *

def checkoutput(filename):
    if not filename.endswith('.root'):
        raise Exception('Invalid filename')
    return filename

def process_file(file, args):
    filedata = parse_dayabay_filename(file)

    period, site = getattr(R, 'k'+filedata.daq_period), getattr(R, filedata.site)

    print(filedata)
    return

    stage1_file = args.output[:-5]+'_stage1.root'
    stage2_file = args.output[:-5]+'_stage2.root'
    if args.stage==1:
        ifile = args.input
        ofile = stage1_file
        R.stage1_main(ifile, ofile, period, site)
    else:
        ifile = stage1_file
        ofile = stage2_file
        R.stage2_main(args.cfg, ifile, ofile, period, 0, site)

    print('Done processing file', ifile)
    print('Write output file', ofile)

def main(args):
    args.common_root = find_common_root(args.input)

    for fname in args.input:
        process_file(fname, args)

if __name__ == "__main__":
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('input', nargs='+', help='input files')
    parser.add_argument('--stage', type=int, choices=[1, 2], required=True, help='Analysis stage')
    parser.add_argument('-o', '--output', type=checkoutput, required=True, help='output file name for stage ')
    parser.add_argument('--cfg', required=True, help='configuration file')

    main(parser.parse_args())
