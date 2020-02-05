#include "AdBuffer.cc"
#include "Calculator.cc"
#include "MultCut.cc"
#include "MuonAlg.cc"
#include "Readers.cc"
#include "Selectors.cc"

#include "../common/Misc.cc"

#include "../SelectorFramework/core/ConfigTool.cc"

void stage2_main(const char* confFile, const char* inFile, const char* outFile,
                 Site site, Phase phase, UInt_t seq)
{
  Pipeline p;

  p.makeOutFile(outFile);

  p.makeTool<Config>(confFile);

  p.makeAlg<MuonReader>();
  p.makeAlg<MuonAlg>(MuonAlg::Purpose::ForIBDs);
  p.makeAlg<MuonAlg>(MuonAlg::Purpose::ForSingles);
  p.makeAlg<PrefetchLooper<MuonReader>>(); // build up a lookahead buffer

  std::vector<Det> allADs = util::ADsFor(site, phase);
  // std::reverse(allADs.begin(), allADs.end()); // XXX

  for (Det detector : allADs) {
    bool lastAD = detector == allADs.back();

    p.makeAlg<AdReader>(detector, lastAD); // ClockWriter if lastAD
    p.makeAlg<AdBuffer>(detector);

    p.makeAlg<SingleSel>(detector);
    p.makeAlg<IbdSel>(detector);

    if (not lastAD)
      p.makeAlg<PrefetchLooper<AdReader, Det>>(detector);
  }

  // Clock is needed for TimeSyncTool (i.e. all the readers)
  p.makeTool<Clock>();

  p.makeTool<MultCutTool>();

  p.process({inFile});

  Calculator calc(p, phase, seq, site);
  calc.writeValues();
}
