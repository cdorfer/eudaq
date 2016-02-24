import ROOT

import math
import json
from pprint import pprint
import os

#with open('../run_log.json') as json_data:
#    d = json.loads(json_data,strict=False)
colors =["#023FA5","#7D87B9","#BEC1D4","#D6BCC0","#BB7784","#4A6FE3","#8595E1","#B5BBE3","#E6AFB9","#E07B91","#D33F6A","#11C638","#8DD593","#C6DEC7","#EAD3C6","#F0B98D","#EF9708","#0FCFC0","#9CDED6","#D5EAE7","#F3E1EB","#F6C4E1","#F79CD4"]
diamonds = ['S129','II-6-94','II-6-95','II-6-96']
#diamonds = ['S129']

def get_max_freq(tree,run,ch,rate):
        hname = "h_maxfreq_%d_ch%d"%(run,ch)
        title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
        title += ' %+4dV'%bias
        h_maxfreq =  ROOT.TH1F(hname,title,1024,0,1e6)
        varexp = "fft_max_freq[%d]>>%s"%(ch,hname)
        entries = tree.Draw(varexp,"!pulser","goff")
        h_maxfreq.SetDirectory(0)
        h_maxfreq.Scale(1./entries)
        return h_maxfreq

def get_inv_max_time(tree,run,ch,rate):
        hname = "h_inv_max_time_%d_ch%d"%(run,ch)
        bin_entries = 10000
        tree.GetEntry(0)
        ts0 = tree.time
        tree.GetEntry(tree.GetEntries()-1)
        ts1 = tree.time
        bins = round(tree.GetEntries()+bin_entries/2-1,-1*int(math.log10(bin_entries)))
        bins =  100
        bins = int(bins)
        title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
        title += ' %+4dV'%bias
        title+=';duration;fft_max_inv'
        h_inv_max_time = ROOT.TH2F(hname,title,bins,ts0,ts1,26000,1e-6,.1)
        try:
            varexp = "1./fft_max[%d]:time>>%s"%(ch,hname)
            entries = tree.Draw(varexp,"!pulser","goff")
            h_inv_max_time.SetDirectory(0)
        except Exception as e:
            print '\nERROR!'
            print 'Cannot create plot',varexp,e
            pass
        return h_inv_max_time

def get_inv_max(tree,run,ch,rate):
        n_inv_mean = "h_inv_max_%d_ch%d"%(run,ch)
        title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
        title += ' %+4dV'%bias
        h_inv_max =  ROOT.TH1F(n_inv_mean,title,26000,1e-6,.1)
        varexp = "1./fft_max[%d]>>%s"%(ch,n_inv_mean)
        entries = tree.Draw(varexp,"!pulser","goff")
        h_inv_max.SetDirectory(0)
        h_inv_max.Scale(1./entries)
        return h_inv_max

def get_mean_inv_max(tree,run,ch,rate):
    hname = 'h_mean_inv_max_%d_ch%d_nonpulser'%(run,ch)
    title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
    title += ' %+4dV'%bias
    histo = ROOT.TH2F(hname,title,200,0,2000,26000,1e-7,.1))
    varexp = "1./fft_max[%d]:fft_mean[%d]>>%s"%(ch,hname)
    entries = tree.Draw(varexp,"!pulser","goff")
    histo.SetDirectory(0)
    histo.Scale(1./entries)

    histo2 = ROOT.TH2F(hname,title,200,0,2000,26000,1e-7,.1))
    hname = 'h_mean_inv_max_%d_ch%d_nonpulser'%(run,ch)
    varexp = "1./fft_max[%d]:fft_mean[%d]>>%s"%(ch,hname)
    entries = tree.Draw(varexp,"pulser","goff")
    histo2.SetDirectory(0)
    histo2.Scale(1./entries)

    return histo,histo2

def get_mean(tree,run,ch,rate):
    n_mean = "h_mean_%d_ch%d"%(run,ch)
    title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
    title += ' %+4dV'%bias
    h_mean = ROOT.TH1F(n_mean,title,2600,0,2600)
    varexp = "fft_mean[%d]>>%s"%(ch,n_mean)
    entries = tree.Draw(varexp,"!pulser","goff")
    if entries == 0:
        print 'empty plot'
        missing_runs.append(run)
        h_mean= None
    else:
        h_mean.Scale(1./entries)
        h_mean.SetDirectory(0)
    return h_mean

def get_mean_time(tree,run,ch,rate):
    hname = "h_mean_time_%d_ch%d"%(run,ch)
    title = 'Run %d, %s kHz/cm^{2},'%(int(r[-3:]),rate)
    title += ' %+4dV'%bias
    title+=';duration;ff_mean'
    bin_entries = 10000
    tree.GetEntry(0)
    ts0 = tree.time
    tree.GetEntry(tree.GetEntries()-1)
    ts1 = tree.time
    bins = round(tree.GetEntries()+bin_entries/2-1,-1*int(math.log10(bin_entries)))
    bins =  100
    bins = int(bins)
    h_mean_time = ROOT.TH2F(hname,title,bins,ts0,ts1,2600,0,2600)
    try:
        varexp = "fft_mean[%d]:time>>%s"%(ch,hname)
        entries = tree.Draw(varexp,"!pulser","goff")
        h_mean_time.SetDirectory(0)
    except Exception as e:
        print '\nERROR!'
        print 'Cannot create plot',varexp,e
        pass
    return h_mean_time


def draw_bias_plots(d,typ):
    print 'draw_plots - bias' ,typ, len(d),d
    for bias in d:
        if bias < 0:
            b='m%04d'%abs(bias)
        else:
            b='p%04d'%abs(bias)
        hs = ROOT.THStack('hs_%s_bias%s'%(typ,b),'%s: %s bias %dV;fft_%s;no of entries'%(dia,typ,bias,typ))
        c1 = ROOT.TCanvas()
        color = 0
        for h in sorted(d[bias].items()):
            if h[1]:
                h[1].SetLineColor(ROOT.TColor.GetColor(colors[color]))
                hs.Add(h[1])
                color +=1
        c1.SetLogy()
        if 'max' in typ:
            c1.SetLogx()
        hs.Draw("nostack")
        c1.BuildLegend(0.7,0.4,.99,.95)
        c1.Update()
        save_canvas(c1,"%s/%s/%s_bias_%sV"%(dia,typ,typ,b))

def draw_time_plots(d,typ):
    for run in d:
        histo = d[run]
        c1 = ROOT.TCanvas()
        histo.Draw('colz')
        if 'max' in typ:
            c1.SetLogy()
        save_canvas(c1,"%s/time/fft_%s/%s_time_%s_%d"%(dia,typ,dia,typ,run))
def draw_freq_plots(d,typ):
    for run in d:
        histo = d[run]
        c1 = ROOT.TCanvas()
        c1.SetLogy()
        histo.Draw()
        save_canvas(c1,"%s/%s/%s_%d"%(dia,typ,typ,run))

def draw_rate_plots(d,typ):
    for rate in d:
        r = '%s kHz/cm^{2}'%rate
        print dia, r
        hs = ROOT.THStack('hs_%s_flux%s'%(typ,rate),'%s: %s flux %d kHz/cm^{2};fft_%s;no of entries'%(dia,typ,rate,typ))
        c1 = ROOT.TCanvas()
        color = 0
        for h in d[rate].items():
            if h[1]:
                h[1].SetLineColor(ROOT.TColor.GetColor(colors[color]))
                hs.Add(h[1])
                color+=1
        c1.SetLogy()
        if 'max'  in typ:
            c1.SetLogx()
        hs.Draw("nostack")
        c1.BuildLegend(0.7,0.4,.99,.95)
        c1.Update()
        save_canvas(c1,"%s/%s/%s_flux_%skHz"%(dia,typ,typ,rate))

def ensure_dir(f):
    d = os.path.dirname(f)
    if not os.path.exists(d):
        os.makedirs(d)

def save_canvas(c1,fname):
    for ext in ['pdf','png','svg','root']:
        fname2 = "./output/fft/%s.%s"%(fname,ext)
        ensure_dir(fname2)
        c1.SaveAs(fname2)

ROOT.gROOT.SetBatch()


with open('../../run_log.json') as data_file:    
    print data_file
    data = json.load(data_file)
missing_runs = []
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
    means = {}
    means_rate ={}
    inv_max = {}
    inv_max_rate = {}
    means_time ={}
    inv_max_time = {}
    max_freqs = {}
    mean_inv = {}
    
    
    for r in sorted(runs.keys()):
        run = int(r)
        fname = "run%s.root"%r
        #print fname
        f = ROOT.TFile.Open(fname)
        if not f:
            print 'missing', run
            missing_runs.append(run)
            continue
        tree = f.Get('tree')
        rate = data[r]['measured flux']
        aimed_flux = data[r]['aimed flux']
        if runs[r] == 0:
            bias = data[r]['hv dia1']
        else:
            bias = data[r]['hv dia2']
        bias = int(bias)
        bias = round(bias,-1)
        ch = runs[r]
        if not means.has_key(bias):
            means[bias] = {}
            inv_max[bias] = {}
        if not means_rate.has_key(aimed_flux):
            means_rate[aimed_flux] = {}
            inv_max_rate[aimed_flux] = {}

        h_mean = get_mean(tree,run,ch,rate)
        if not h_mean:
            continue
        h_mean_time = get_mean_time(tree,run,ch,rate)

        h_inv_max = get_inv_max(tree,run,ch,rate)
        h1, h2 = get_mean_inv_max(tree,run,ch,rate)
        h_maxfreq = get_max_freq(tree,run,ch,rate)
        h_inv_max_time = get_inv_max_time(tree,run,ch,rate)
        
        means[bias][run] = h_mean
        means_time[run] = h_mean_time
        inv_max[bias][run] = h_inv_max
        inv_max_rate[aimed_flux][run] = h_inv_max
        max_freqs[run] = h_maxfreq
        inv_max_time[run] = h_inv_max_time
        means_rate[aimed_flux][run] = h_mean

    
    print dia,means.keys(), means_rate.keys(),means_time.keys()
    draw_bias_plots(means,'mean')
    draw_bias_plots(inv_max,'inv_max')

    draw_rate_plots(means_rate,'mean')
    draw_rate_plots(inv_max_rate,'inv_max')

    draw_freq_plots(max_freqs,'max_freq')

    draw_time_plots(means_time,'mean')
    draw_time_plots(inv_max_time,'inv_max')
        
print 'missing runs',set(missing_runs)
