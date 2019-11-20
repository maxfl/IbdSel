import ROOT
ROOT.gSystem.AddIncludePath("-Iselector/")
ROOT.gSystem.AddIncludePath("-Iselector/SelectorFramework/core")
ROOT.PyConfig.IgnoreCommandLineOptions = True

for line in (".L selector/stage1_main.cc+", ".L selector/stage2_main.cc+"):
    ROOT.gROOT.ProcessLine(line)

ROOT.Stage, ROOT.Site                 # preload

