#include "SelectorFramework/core/BaseIO.cc"

class ClusterTree : public TreeBase {
  static constexpr int NMAX = 2048;
  template <class T> using Buf = std::array<T, NMAX>;

public:
  UChar_t size;
  Buf<UInt_t> trigSec;
  Buf<UInt_t> trigNanoSec;
  Buf<Float_t> energy;

  void initBranches() override;
};

void ClusterTree::initBranches()
{
  BR(size);
  BR_VARLEN(trigSec, size);
  BR_VARLEN(trigNanoSec, size);
  BR_VARLEN(energy, size);
}
