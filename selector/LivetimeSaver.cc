#pragma once

#include <TH1F.h>

#include "SelectorFramework/core/Kernel.cc"

#include "EventReader.cc"

using Status = Algorithm::Status;

class LivetimeSaver : public SimpleAlg<EventReader> {
public:
  Status consume(const EventReader::Data& data) override;
  void finalize(Pipeline& pipeline) override;

private:
  float last_integralRunTime_ms;
};

Status LivetimeSaver::consume(const EventReader::Data& e)
{
  last_integralRunTime_ms = e.integralRunTime_ms;

  return Status::Continue;
}

void LivetimeSaver::finalize(Pipeline& _pipeline)
{
  auto h = new TH1F("h_livetime", "Livetime (s)", 1, 0, 1);
  h->SetBinContent(1, last_integralRunTime_ms / 1000);
  h->Write();
}
