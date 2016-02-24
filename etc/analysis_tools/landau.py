import ROOT

import math
import json
from pprint import pprint
import os

#with open('../run_log.json') as json_data:
#    d = json.loads(json_data,strict=False)
colors =["#023FA5","#7D87B9","#BEC1D4","#D6BCC0","#BB7784","#4A6FE3","#8595E1","#B5BBE3","#E6AFB9","#E07B91","#D33F6A","#11C638","#8DD593","#C6DEC7","#EAD3C6","#F0B98D","#EF9708","#0FCFC0","#9CDED6","#D5EAE7","#F3E1EB","#F6C4E1","#F79CD4"]
ROOT.gROOT.SetBatch()
missing_runs = []
landau_range = (510,-10,500)
with open('../../run_log.json') as data_file:    
    print data_file
    data = json.load(data_file)

def ensure_dir(f):
    d = os.path.dirname(f)
    if not os.path.exists(d):
        os.makedirs(d)

def save_canvas(c1,fname):
    for ext in ['pdf','png','svg','root']:
        fname2 = "./output/landau/%s.%s"%(fname,ext)
        ensure_dir(fname2)
        c1.SaveAs(fname2)

def get_cut(ch):
    cut = '1./fft_max[%d] > 1e-4 '%ch
    cut +=' && fft_mean[%d] < 500'%(ch)
    cut +=' && fft_mean[%d]>50'%ch
    if cut !='': cut += ' && '
    cut += "!pulser && !is_saturated[%d]"%ch
    return cut

def get_sig_int_time(tree,run,cut='',var='sig_integral1',nstart=50e3):
    n_mean = "h_%s_time_%d_ch%d"%(var,run,ch)
    title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
    title += ' %+4dV'%bias
    tree.GetEntry(0)
    ts0 =tree.time
    tree.GetEntry(tree.GetEntries()-1)
    ts1 = tree.time
    #print tree.GetEntries(),ts0,ts1,ts1-ts0
    bins = max(int(tree.GetEntries()/20000+.5),1)
    #print bins
    h_mean = ROOT.TH2F(n_mean,title,bins,ts0,ts1,landau_range[0],landau_range[1],landau_range[2])
    #tree->Draw("sig_integral1[0]-ped_median[0]:time","!pulser&&1/fft_max[0]>1e-4","colz")
    #print var
    if 'spread' in var:
        varexp = "%s[%d]:time>>%s"%(var,ch,n_mean)
    else:
        varexp = "%s[%d]-ped_median[%d]:time>>%s"%(var,ch,ch,n_mean)
    #print varexp
    cut = get_cut(ch)
    entries = tree.Draw(varexp,cut,"goff",int(1e9),int(nstart))
    if entries == 0:
        print 'empty plot',run
        missing_runs.append(run)
        return None
    h_mean.Scale(1./entries)
    h_mean.SetDirectory(0)
    return h_mean

def get_sig_int(tree,run,cut='',var='sig_integral1',nstart=50e3):
    n_mean = "h_%s_%d_ch%d"%(var,run,ch)
    title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
    title += ' %+4dV'%bias
    h_mean = ROOT.TH1F(n_mean,title,landau_range[0],landau_range[1],landau_range[2])
    #tree->Draw("sig_integral1[0]-ped_median[0]:time","!pulser&&1/fft_max[0]>1e-4","colz")
    if 'spread' in var:
        varexp = "%s[%d]>>%s"%(var,ch,n_mean)
    else:
        varexp = "%s[%d]-ped_median[%d]>>%s"%(var,ch,ch,n_mean)
    #print n_mean,varexp
    cut = get_cut(ch)
    entries = tree.Draw(varexp,cut,"goff",int(1e9),int(nstart))
    if entries == 0:
        print 'empty plot',run
        missing_runs.append(run)
        return None
    h_mean.Scale(1./entries)
    h_mean.SetDirectory(0)
    return h_mean
        
def save_landaus(landaus,var):
    for bias in landaus:
        if bias < 0:
            b='m%04d'%abs(bias)
        else:
            b='p%04d'%abs(bias)
        s_landaus = ROOT.THStack('hs_landaus_bias%s'%b,'%s: landaus bias %dV;fft_mean;no of entries'%(dia,bias))
        c1 = ROOT.TCanvas()
        color = 0
        for h in sorted(landaus[bias].items()):
            if h[1]:
                h[1].SetLineColor(ROOT.TColor.GetColor(colors[color]))
                s_landaus.Add(h[1])
                color +=1
        s_landaus.Draw("nostack")
        c1.BuildLegend(0.7,0.4,.99,.95)
        c1.Update()
        save_canvas(c1,"%s/%s/landaus_bias_%sV"%(dia,var,b))
def save_landaus_time(landaus,var):
    for bias in landaus:
        profs = {}
        if bias < 0:
            b='m%04d'%abs(bias)
        else:
            b='p%04d'%abs(bias)
        ymax = -1000
        ymin = 1000
        for run in landaus[bias]:
            c1 =  ROOT.TCanvas()
            landaus[bias][run].Draw('colz')

            prof = landaus[bias][run].ProfileX()
            profs[run] = prof.Clone()
            profs[run].SetDirectory(0)
            prof.SetLineColor(ROOT.kBlack)
            prof.SetMarkerColor(ROOT.kBlack)
            prof.SetMarkerStyle(2)

            prof.Draw('same hist PL')
            if ymax < prof.GetMaximum():
                ymax = prof.GetMaximum()
            if prof.GetMinimum() < ymin:
                ymin = prof.GetMinimum()
            save_canvas(c1,'%s/%s/time/landau_time_%s_%s'%(dia,var,b,run))
        c1 =  ROOT.TCanvas()
        color = 0
        ydelta = ymax-ymin
        ymin = max(ymin -.7 * ydelta,0)
        ymax = ymax + .1 * ydelta
        c1.DrawFrame(0,ymin,2500e3,ymax,'%s;time/ms;%s/mV'%(var,var))
        for r in sorted(profs.keys()):
            p = profs[r]
            if p.GetNbinsX()< 2:
                continue
            c = ROOT.TColor.GetColor(colors[color])
            color+=1
            p.SetLineColor(c)
            p.SetMarkerColor(c)
            if color == 0:
                p.Draw('hist')
            else:
                p.Draw('hist pl same')
        leg = c1.BuildLegend(.55,.15,.85,.40)
        leg.Draw()
        save_canvas(c1,'%s/%s/time/landau_prof_%s'%(dia,var,b))


var_exps=  ['sig_spread','sig_int','sig_integral1','sig_integral2','sig_integral3']
#var_exps=  ['sig_integral1']

diamonds = ['S129','II-6-94','II-6-95','II-6-96']
#diamonds = ['S129']
#diamonds = ['II-6-96']
for diamond in diamonds:
    runs = {}
    dia = diamond
    for run in data:
        if data[run]['type'] != 'signal':
            continue
        if data[run]["diamond 1"] == diamond:
            runs[run] = 0
        if data[run]["diamond 2"] == diamond:
            runs[run] = 3
    print 'Diamond: %s with %d Runs'%(dia,len(run))
    
    landaus = {}
    landaus_time = {}
    
    for r in sorted(runs.keys()):
        run = int(r)
        print run
        fname = "run%s.root"%r
        #print fname
        f = ROOT.TFile.Open(fname)
        if not f:
            print 'missing', run
            missing_runs.append(run)
            continue
        tree = f.Get('tree')
        if tree.GetEntries()<50e3:
            continue
        rate = data[r]['measured flux']
        aimed_flux = data[r]['aimed flux']
        rate = aimed_flux
        if runs[r] == 0:
            bias = data[r]['hv dia1']
        else:
            bias = data[r]['hv dia2']
        bias = int(bias)
        bias = round(bias,-1)
        ch = runs[r]
        for var in var_exps:
            if not landaus.has_key(var):
                landaus[var] = {}
                landaus_time[var] = {}
            if not landaus[var].has_key(bias):
                landaus[var][bias] = {}
                landaus_time[var][bias] = {}
            h_mean = get_sig_int(tree,run,var=var)
            h_mean_time = get_sig_int_time(tree,run,var=var)
            if not h_mean: continue
            landaus[var][bias][run] = h_mean
            landaus_time[var][bias][run] = h_mean_time
    for var in var_exps:
        save_landaus(landaus[var],var)
        save_landaus_time(landaus_time[var],var)
print 'missing runs',set(missing_runs)
