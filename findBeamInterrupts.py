import ROOT as r
import copy, collections, numpy, array
import sys

r.gROOT.SetBatch()
print 'starting...'

runfile = sys.argv[-1]

run = int(runfile.rstrip('.root')[-3:])
print 'analysing run', run
s_file = r.TFile(runfile, 'READ')
s_tree = s_file.Get('tree')

nentries = s_tree.GetEntries()

last_entry = s_tree.GetEntry(nentries-1)
max_time = s_tree.time



foobar   = s_tree.Draw('time:event_number')

graph = copy.deepcopy(r.c1.FindObject('Graph'))
histo = copy.deepcopy(r.c1.FindObject('htemp'))

histo.SetTitle('run %3d' %(run))
histo.SetName('run %3d' %(run))

dts = []
evs = []

i = 20000
step = 100
while i+step < nentries:
    s_tree.GetEntry(i)
    t1 = s_tree.time
    evs.append(s_tree.event_number)
    s_tree.GetEntry(i+step)
    t2 = s_tree.time
    dt = (t2 - t1)
    dts.append(dt)
    i+= step

jumps = []

deq = collections.deque(dts[:100],100)
first = True
for i in dts[101:]:
    avg = numpy.mean(deq)
    if abs(i / avg - 1.) > 0.5: 
        if first:
            print 'found a jump here', i, 'at event number', evs[dts.index(i)]
            jumps.append(evs[dts.index(i)])
            first = False
    else:
        if not first:
            print 'back to normal at event', evs[dts.index(i)]
        deq.appendleft(i)
        first = True

print '\n'
print 'found %d jumps' %(len(jumps))
print 'they are at event numbers', jumps

lat = r.TLatex()
lat.SetNDC()
lat.SetTextColor(r.kRed)
lat.DrawLatex(0.2,0.85, 'run %d' %(run) )
    

if len(jumps):
    print 'the length of jumps is', len(jumps)
    jumps_array = array.array('d', jumps)
    jumps_err_array = array.array('d', len(jumps)*[histo.GetYaxis().GetXmin()])
    
    jumps_graph = r.TGraph(len(jumps), jumps_array, jumps_err_array)
    jumps_graph.SetMarkerSize(3)
    jumps_graph.SetMarkerColor(r.kRed)
    jumps_graph.SetMarkerStyle(33)
    jumps_graph.SetLineColor(r.kRed)
    jumps_graph.Draw('p')

    outfile = open('plots/jumps.txt','r+a')
    # check if the run is already in the file
    runInFile = False
    lines = outfile.readlines()
    for i in lines:
        if len(i.split()) > 0 and i.split()[0] == str(run): 
            runInFile = True
    if not runInFile:
        outfile.write(str(run)+'\t\t')

    lat.SetTextColor(r.kBlack)
    for i in jumps:
        ind = jumps.index(i)
        lat.DrawLatex(0.2, 0.80-ind*0.05, '#%d at %d' %(ind, i) )
        if not runInFile:
            outfile.write(str(i)+'\t')
    if not runInFile:
        outfile.write('\n')
    outfile.close()
        

r.c1.SaveAs('plots/jumpSearch_run%d.png' %(run))

s_file.Close()
