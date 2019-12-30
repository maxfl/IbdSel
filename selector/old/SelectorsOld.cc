#pragma once

#include <TH1F.h>

#include "Constants.cc"
#include "Readers.cc"
#include "MuonAlg.cc"
#include "MultCut.cc"
#include "IbdTree.cc"

#include "SelectorFramework/core/TreeWriter.cc"

template <class ReaderT>
class SelectorBase : public SimpleAlg<ReaderT, Det> {
public:
  SelectorBase(Det detector, MuonAlg::Purpose purpose) :
    SimpleAlg<ReaderT, Det>(detector),
    detector(detector),
    purpose(purpose) {}

  void connect(Pipeline& p) override;

  const Det detector;

protected:
  const MuonAlg* muonAlg;
  const MultCutTool* multCutTool;

private:
  const MuonAlg::Purpose purpose;
};

template <class ReaderT>
void SelectorBase<ReaderT>::connect(Pipeline& p)
{
  SimpleAlg<ReaderT, Det>::connect(p);

  muonAlg = p.getAlg<MuonAlg>(purpose);
  multCutTool = p.getTool<MultCutTool>();
}

class SingleSelector : public SelectorBase<SingleReader> {
  static constexpr float EMIN = 0.7;
  static constexpr float EMAX = 12;

public:
  SingleSelector(Det detector);

  Algorithm::Status consume(const ClusterTree& e) override;
  void finalize(Pipeline& p) override;

  TH1F* hist;
};

SingleSelector::SingleSelector(Det detector) :
  SelectorBase(detector, MuonAlg::Purpose::ForSingles)
{
  auto hname = Form("h_single_d%d", int(detector));
  hist = new TH1F(hname, hname, 113, 0.7, 12);
}

void SingleSelector::finalize(Pipeline& _p)
{
  hist->Write();
}

Algorithm::Status SingleSelector::consume(const ClusterTree& cluster)
{
  for (size_t i = 0; i < cluster.size; ++i) {
    const float e = cluster.energy[i];

    if (EMIN < e && e < EMAX &&
        !muonAlg->isVetoed(cluster.time(i), detector) &&
        // In current implementation, mult cut is unnecessary since
        // "singles" are isolated by 1000 us by definition.
        // If we want a smaller isolation cut, we need to look at
        // clusters_ADx tree and then apply singleDmcOk
        multCutTool->singleDmcOk(cluster, detector, 0)) {
      hist->Fill(e);
    }
  }

  return Status::Continue;
}

class IbdSelector : public SelectorBase<ClusterReader> {
  static constexpr float PROMPT_MIN = 0.7;
  static constexpr float PROMPT_MAX = 12;
  static constexpr float DELAYED_MIN = 6;
  static constexpr float DELAYED_MAX = 12;

public:
  IbdSelector(Det detector);

  Algorithm::Status consume(const ClusterTree& e) override;
  void connect(Pipeline& p) override;
  void finalize(Pipeline& p) override;

  TH1F* hist;

private:
  void save(const ClusterTree& cluster, size_t iP, size_t iD);

  TreeWriter<IbdTree> ibdTree;
};

IbdSelector::IbdSelector(Det detector) :
  SelectorBase(detector, MuonAlg::Purpose::ForIBDs),
  ibdTree(Form("ibd_AD%d", int(detector)))
{
  auto hname = Form("h_ibd_d%hhu", detector);
  hist = new TH1F(hname, hname, 113, 0.7, 12);
}

void IbdSelector::connect(Pipeline& p)
{
  ibdTree.connect(p);
  SelectorBase::connect(p);
}

void IbdSelector::finalize(Pipeline& p)
{
  hist->Write();
}

void IbdSelector::save(const ClusterTree& cluster, size_t iP, size_t iD)
{
  ibdTree.data.runNo = cluster.runNo;
  ibdTree.data.fileNo = cluster.fileNo;
  ibdTree.data.trigP = cluster.trigNo[iP];
  ibdTree.data.trigD = cluster.trigNo[iD];

  ibdTree.fill();
}

Algorithm::Status IbdSelector::consume(const ClusterTree& cluster)
{
  for (size_t iP = 0; iP < unsigned(cluster.size - 1); ++iP) {
    for (size_t iD = iP+1; iD < cluster.size; ++iD) {
      const float eP = cluster.energy[iP];
      const float eD = cluster.energy[iD];
      const float dt_us = cluster.time(iD).diff_us(cluster.time(iP));

      if (PROMPT_MIN  < eP && eP < PROMPT_MAX &&
          DELAYED_MIN < eD && eD < DELAYED_MAX &&
          1 < dt_us && dt_us < 200 &&
          !muonAlg->isVetoed(cluster.time(iD), detector) &&
          multCutTool->pairDmcOk(cluster, detector, iP, iD)) {

        // std::cout << Form("IBD AD%d %d %d\n", int(detector),
        //                   cluster.trigNo[iP], cluster.trigNo[iD]);

        save(cluster, iP, iD);

        hist->Fill(eP);
      }
    }
  }

  return Status::Continue;
}