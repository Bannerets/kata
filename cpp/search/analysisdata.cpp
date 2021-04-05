#include "../search/analysisdata.h"

AnalysisData::AnalysisData()
  :move(Board::NULL_LOC),
   numVisits(0),
   playSelectionValue(0.0),
   lcb(0.0),
   radius(0.0),
   utility(0.0),
   resultUtility(0.0),
   scoreUtility(0.0),
   winLossValue(0.0), 
   noResultValue(0.0),
   policyPrior(0.0),
   scoreMean(0.0),
   scoreStdev(0.0),
   lead(0.0),
   ess(0.0),
   weightFactor(0.0),
   order(0), 
  expectFinishGameMovenum(0),
   pv(),
   pvVisits(),
   node(NULL)
{}

AnalysisData::AnalysisData(const AnalysisData& other)
  :move(other.move),
   numVisits(other.numVisits),
   playSelectionValue(other.playSelectionValue),
   lcb(other.lcb),
   radius(other.radius),
   utility(other.utility),
   resultUtility(other.resultUtility),
   scoreUtility(other.scoreUtility),
   winLossValue(other.winLossValue),
   noResultValue(other.noResultValue),
   policyPrior(other.policyPrior),
   scoreMean(other.scoreMean),
   scoreStdev(other.scoreStdev),
   lead(other.lead),
   ess(other.ess),
   weightFactor(other.weightFactor),
   order(other.order), 
  expectFinishGameMovenum(other.expectFinishGameMovenum),
   pv(other.pv),
   pvVisits(other.pvVisits),
   node(other.node)
{}

AnalysisData::AnalysisData(AnalysisData&& other) noexcept
  :move(other.move),
   numVisits(other.numVisits),
   playSelectionValue(other.playSelectionValue),
   lcb(other.lcb),
   radius(other.radius),
   utility(other.utility),
   resultUtility(other.resultUtility),
   scoreUtility(other.scoreUtility),
   winLossValue(other.winLossValue),
   noResultValue(other.noResultValue),
   policyPrior(other.policyPrior),
   scoreMean(other.scoreMean),
   scoreStdev(other.scoreStdev),
   lead(other.lead),
   ess(other.ess),
   weightFactor(other.weightFactor),
   order(other.order),
  expectFinishGameMovenum(other.expectFinishGameMovenum),
   pv(std::move(other.pv)),
   pvVisits(std::move(other.pvVisits)),
   node(other.node)
{}

AnalysisData::~AnalysisData()
{}

AnalysisData& AnalysisData::operator=(const AnalysisData& other) {
  if(this == &other)
    return *this;
  move = other.move;
  numVisits = other.numVisits;
  playSelectionValue = other.playSelectionValue;
  lcb = other.lcb;
  radius = other.radius;
  utility = other.utility;
  resultUtility = other.resultUtility;
  scoreUtility = other.scoreUtility;
  winLossValue = other.winLossValue;
  noResultValue = other.noResultValue;
  policyPrior = other.policyPrior;
  scoreMean = other.scoreMean;
  scoreStdev = other.scoreStdev;
  lead = other.lead;
  ess = other.ess;
  weightFactor = other.weightFactor;
  order = other.order;
  expectFinishGameMovenum=other.expectFinishGameMovenum,
  pv = other.pv;
  pvVisits = other.pvVisits;
  node = other.node;
  return *this;
}

AnalysisData& AnalysisData::operator=(AnalysisData&& other) noexcept {
  if(this == &other)
    return *this;
  move = other.move;
  numVisits = other.numVisits;
  playSelectionValue = other.playSelectionValue;
  lcb = other.lcb;
  radius = other.radius;
  utility = other.utility;
  resultUtility = other.resultUtility;
  scoreUtility = other.scoreUtility;
  winLossValue = other.winLossValue;
  noResultValue = other.noResultValue;
  policyPrior = other.policyPrior;
  scoreMean = other.scoreMean;
  scoreStdev = other.scoreStdev;
  lead = other.lead;
  ess = other.ess;
  weightFactor = other.weightFactor;
  order = other.order;
  expectFinishGameMovenum = other.expectFinishGameMovenum,
  pv = std::move(other.pv);
  pvVisits = std::move(other.pvVisits);
  node = other.node;
  return *this;
}

bool operator<(const AnalysisData& a0, const AnalysisData& a1) {
  if(a0.playSelectionValue > a1.playSelectionValue)
    return true;
  else if(a0.playSelectionValue < a1.playSelectionValue)
    return false;
  if(a0.numVisits > a1.numVisits)
    return true;
  else if(a0.numVisits < a1.numVisits)
    return false;
  // else if(a0.utility > a1.utility)
  //   return true;
  // else if(a0.utility < a1.utility)
  //   return false;
  else
    return a0.policyPrior > a1.policyPrior;
}

bool AnalysisData::pvContainsPass() const {
  for(int i = 0; i<pv.size(); i++)
    if(pv[i] == Board::PASS_LOC)
      return true;
  return false;
}

void AnalysisData::writePV(std::ostream& out, const Board& board) const {
  for(int j = 0; j<pv.size(); j++) {
    if(j > 0)
      out << " ";
    out << Location::toString(pv[j],board);
  }
}

void AnalysisData::writePVVisits(std::ostream& out) const {
  for(int j = 0; j<pvVisits.size(); j++) {
    if(j > 0)
      out << " ";
    out << pvVisits[j];
  }
}
