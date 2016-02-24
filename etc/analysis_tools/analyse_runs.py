import ROOT
from subprocess import call
from multiprocessing.dummy import Pool
from subprocess import call
import subprocess
import multiprocessing
import datetime

from functools import partial

import argparse
import time

import json
from pprint import pprint



def is_valid_run(runnumber,timestamp,out_dir="."):
    fname = out_dir+'/run%d.root'%runnumber
    fname = os.path.abspath(os.path.expanduser(fname))
    if not os.path.exists(fname):
        return False
    ts = time.ctime(os.path.getmtime(fname))
def load_json(filename):
    with open('../run_log.json') as data_file:    
        print data_file
        data = json.load(data_file)
    print 'JSON has %d keys'%len(data)
    return data

parser = argparse.ArgumentParser()

parser.add_argument('-a','--all', action='store_true',
                   default = False,
                   help='Analyse all')
parser.add_argument('-j','--jobs', default = 4, 
                   help='Set jobs')

parser.add_argument('-r', '--runs', nargs='+', type=int,default=None)
parser.add_argument('-t','--testbeam',default=1505,type=int)
parser.add_argument('-d','--diamond',default=None)
parser.add_argument('-p','--pwd',default= '~/testing/eudaq-felix/')
args = parser.parse_args()
data  = load_json('../run_log.json')
pwd =args.pwd
if args.runs:
        runs = args.runs
        runs = [int('%d%05d'%(args.testbeam,x)) for x in runs]
if args.diamond:
    diamond = args.diamond
    print 'Found Diamond', args.diamond
    runs = []
    for run in data:
        if data[run]['type'] != 'signal':
            continue
        if data[run]["diamond 1"] == diamond:
            runs.append(int(run))
        if data[run]["diamond 2"] == diamond:
            runs.append(int(run))

print 'RUNS: ', runs
raw_input()
nProcesses = int(args.jobs)
commands = []



for run in runs:
    cmd = pwd+'./bin/Converter.exe  -t drs4tree '
    inp = ' /data/psi_2015_05/raw/run%d.raw'%run
    bias = ''
    run_log = data[str(run)]
    for d in [1,2]:
        if int(run_log['hv dia%d'%d]) < 0:
            bias+='M'
        else:
            bias+='P'

    conf = ' -c %s/conf/converter%s.conf'%(pwd,bias)
    out = ' -o  /data/psi_2015_05/root/run%d.root'%run
    cmd += inp + conf + out
    print cmd

    commands.append((run,cmd))

exit
pool = Pool(nProcesses)
it = pool.imap_unordered(partial(call, shell=True), [c[1] for c in commands])
failures = []
complete = []

for i, returncode in enumerate(it):
    # print multiprocessing.active_children()
    if returncode != 0:
        print("Command '%s'  failed: %d" % (commands[i], returncode))
        failures.append(commands[i][0])
    else:
        complete.append(commands[i][0])
        print("Command '%s'  completed: %d" % (commands[i], returncode))
print 'completed:',complete

print 'Failures:',failures

print 'finished'

