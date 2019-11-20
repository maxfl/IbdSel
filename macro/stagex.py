#!/usr/bin/env python

"""Simple python macro to run stage1_main.cc"""

from load import ROOT as R

def checkoutput(filename):
    if not filename.endswith('.root'):
        raise Exception('Invalid filename')
    return filename

def main(args):
    period, site = getattr(R, 'k'+args.period), getattr(R, args.site)

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

if __name__ == "__main__":
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('input', help='input files')
    parser.add_argument('--site',  choices=('EH1', 'EH2', 'EH3'), required=True, help='site to process')
    parser.add_argument('--period', choices=('6AD', '8AD', '7AD'), required=True, help='DAQ stage')
    parser.add_argument('--stage', type=int, choices=[1, 2], required=True, help='Analysis stage')
    parser.add_argument('-o', '--output', type=checkoutput, required=True, help='output file name for stage ')
    parser.add_argument('--cfg', required=True, help='configuration file')

    main(parser.parse_args())
