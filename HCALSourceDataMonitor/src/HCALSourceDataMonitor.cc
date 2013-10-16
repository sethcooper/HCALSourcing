// -*- C++ -*-
//
// Package:    HCALSourceDataMonitor
// Class:      HCALSourceDataMonitor
// 
/**\class HCALSourceDataMonitor HCALSourceDataMonitor.cc HCALSourcing/HCALSourceDataMonitor/src/HCALSourceDataMonitor.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Seth Cooper,32 4-B03,+41227675652,
//         Created:  Tue Jul  2 10:47:48 CEST 2013
// $Id: HCALSourceDataMonitor.cc,v 1.7 2013/07/23 08:41:57 scooper Exp $
//
//


// system include files
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/HcalRecHit/interface/HcalSourcePositionData.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"
#include "CondFormats/HcalObjects/interface/HcalElectronicsMap.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "DataFormats/HcalDetId/interface/HcalElectronicsId.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TCanvas.h"
#include "TTree.h"

#define MAXCHPEREVENT 500
//
// class declaration
//
class HCALSourceDataMonitor : public edm::EDAnalyzer {
   public:
      explicit HCALSourceDataMonitor(const edm::ParameterSet&);
      ~HCALSourceDataMonitor();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      bool isDigiAssociatedToSourceTube(const HcalDetId& detId, std::string tubeName);
      // ----------member data ---------------------------
      std::string rootFileName_;
      bool printRawHistograms_;
      bool selectDigiBasedOnTubeName_;
      int naiveEvtNum_;
      TFile* rootFile_;
      TTree* eventTree_;
      // tree content
      int treeEventNum_;
      int treeOrbitNum_;
      int treeIndex_;
      int treeMsgCounter_;
      float treeMotorCurrent_;
      float treeMotorVoltage_;
      int treeReelPos_;
      float treeTimestamp1_;
      float treeTriggerTimestamp_;
      char treeTubeName_[100];
      int treeNChInEvent_;
      uint32_t treeChDenseIndex_[MAXCHPEREVENT];
      float treeChHistMean_[MAXCHPEREVENT];
      float treeChHistRMS_[MAXCHPEREVENT];
      uint16_t treeChHistBinContentCap0_[MAXCHPEREVENT][32];
      uint16_t treeChHistBinContentCap1_[MAXCHPEREVENT][32];
      uint16_t treeChHistBinContentCap2_[MAXCHPEREVENT][32];
      uint16_t treeChHistBinContentCap3_[MAXCHPEREVENT][32];
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//


//
// constructors and destructor
//
HCALSourceDataMonitor::HCALSourceDataMonitor(const edm::ParameterSet& iConfig) :
  rootFileName_ (iConfig.getUntrackedParameter<std::string>("RootFileName","hcalSourceDataMon.root")),
  printRawHistograms_ (iConfig.getUntrackedParameter<bool>("PrintRawHistograms",false)),
  selectDigiBasedOnTubeName_ (iConfig.getUntrackedParameter<bool>("SelectDigiBasedOnTubeName",true))
{
  //now do what ever initialization is needed
  naiveEvtNum_ = 0;
  rootFile_ = new TFile(rootFileName_.c_str(),"recreate");
  rootFile_->cd();
  eventTree_ = new TTree("eventTree","Event data");
  eventTree_->Branch("eventNum",&treeEventNum_);
  eventTree_->Branch("orbitNum",&treeOrbitNum_);
  eventTree_->Branch("index",&treeIndex_);
  eventTree_->Branch("msgCounter",&treeMsgCounter_);
  eventTree_->Branch("motorCurrent",&treeMotorCurrent_);
  eventTree_->Branch("motorVoltage",&treeMotorVoltage_);
  eventTree_->Branch("reelPos",&treeReelPos_);
  eventTree_->Branch("timestamp1",&treeTimestamp1_);
  eventTree_->Branch("triggerTimestamp",&treeTriggerTimestamp_);
  eventTree_->Branch("tubeName",treeTubeName_,"tubeName/C");
  eventTree_->Branch("nChInEvent",&treeNChInEvent_);
  eventTree_->Branch("chDenseIndex",treeChDenseIndex_,"chDenseIndex[nChInEvent]/i");
  eventTree_->Branch("chHistMean",treeChHistMean_,"chHistMean[nChInEvent]/F");
  eventTree_->Branch("chHistRMS",treeChHistRMS_,"chHistRMS[nChInEvent]/F");
  eventTree_->Branch("chHistBinContentCap0",treeChHistBinContentCap0_,"chHistBinContentCap0[nChInEvent][32]/s");
  eventTree_->Branch("chHistBinContentCap1",treeChHistBinContentCap1_,"chHistBinContentCap1[nChInEvent][32]/s");
  eventTree_->Branch("chHistBinContentCap2",treeChHistBinContentCap2_,"chHistBinContentCap2[nChInEvent][32]/s");
  eventTree_->Branch("chHistBinContentCap3",treeChHistBinContentCap3_,"chHistBinContentCap3[nChInEvent][32]/s");

}


HCALSourceDataMonitor::~HCALSourceDataMonitor()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
//
bool HCALSourceDataMonitor::isDigiAssociatedToSourceTube(const HcalDetId& detId, std::string tubeName)
{
  using namespace std;
  int ieta = detId.ieta();
  int iphi = detId.iphi();
  //"H2_HB_PHI11_LAYER0_SRCTUBE" // example tube for H2
  //"HFM01_ETA29_PHI55_T1A_SRCTUBE" // example tube for HF/P5
  //"H2_FAKETEST_1_PHI57" // fake H2 tube
  int tubePhi = atof(tubeName.substr(tubeName.find("PHI")+3,tubeName.find("_LAYER")-1).c_str());
  if(tubeName.find("HB") != string::npos)
  {
    // for HB, tubes go along eta (constant phi)-->keep all eta/depth for specific iphi
    if(tubePhi==iphi)
      return true;
    // TESTING
    //if(iphi==12)
    //  return true;
    // TESTING
  }
  else if(tubeName.find("HFM") != string::npos || tubeName.find("HFP") != string::npos)
  {
    // for HF, tubes go into one tower (require same eta,phi)
    int tubeEta = atof(tubeName.substr(tubeName.find("ETA")+3,tubeName.find("_PHI")-1).c_str());
    if(tubeEta==ieta && tubePhi==iphi)
      return true;
  }
  else if(tubeName.find("FAKE") != string::npos)
  {
    // just keep all the digis for fake
    return true;
  }
  return false;
}

// ------------ method called for each event  ------------
void
HCALSourceDataMonitor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace std;

  naiveEvtNum_++;
  // FIXME: the first five empty events
  if(naiveEvtNum_ < 6) return;

  // get the mapping
  edm::ESHandle<HcalDbService> pSetup;
  iSetup.get<HcalDbRecord>().get( pSetup );

  // get source position data
  Handle<HcalSourcePositionData> hcalSourcePositionDataHandle;
  iEvent.getByType(hcalSourcePositionDataHandle);
  const HcalSourcePositionData* spd = hcalSourcePositionDataHandle.product();

  // get trigger data
  Handle<HcalTBTriggerData> hcalTBTriggerDataHandle;
  iEvent.getByType(hcalTBTriggerDataHandle);
  const HcalTBTriggerData* triggerData = hcalTBTriggerDataHandle.product();
  //// output trigger timestamp
  int32_t  trigtimebase = (int32_t)triggerData->triggerTimeBase();
  uint32_t trigtimeusec = triggerData->triggerTimeUsec();
  // trim seconds off of usec and add to base
  trigtimeusec %= 1000000;
  trigtimebase += trigtimeusec/1000000;
  //triggerTimeStampVals_.push_back(trigtimebase);
  //char str[50];
  //sprintf(str, "  Trigger time: %s", ctime((time_t *)&trigtimebase));
  //cout << str;
  //sprintf(str, "                %d us\n", trigtimeusec);
  //cout << str;
  //cout << endl;
  // consider what comes out as "timestamp2" in payload as usec for driver ts?
  int timebase =0; int timeusec=0;
  spd->getDriverTimestamp(timebase,timeusec);
  // trim seconds off of usec and add to base
  timeusec %= 1000000;
  timebase += timeusec/1000000;
  //timeStamp1Vals_.push_back(timebase);
  //char str[50];
  //sprintf(str, "  Driver Timestamp : %s", ctime((time_t *)&timebase));
  //s << str;
  //FIXME TODO: get tube name out of source position data
  //string tubeName = "H2_HB_PHI11_LAYER0_SRCTUBE"; // example tube for H2
  //string tubeName = "HFM01_ETA29_PHI55_T1A_SRCTUBE"; // example tube for HF/P5
  string tubeName = spd->tubeNameFromCoord();

  int eventNum = iEvent.id().event();
  treeEventNum_ = eventNum;
  treeOrbitNum_ = triggerData->orbitNumber();
  treeIndex_ = spd->indexCounter();
  treeMsgCounter_ = spd->messageCounter();
  treeMotorCurrent_ = spd->motorCurrent();
  treeMotorVoltage_ = spd->motorVoltage();
  treeReelPos_ = spd->reelCounter();
  treeTriggerTimestamp_ = trigtimebase;
  treeTimestamp1_ = timebase;
  strcpy(treeTubeName_,tubeName.c_str());

  vector<Handle<HcalHistogramDigiCollection> > hcalHistDigiCollHandleVec;
  iEvent.getManyByType(hcalHistDigiCollHandleVec);

  int nChInEvent = 0;
  vector<Handle<HcalHistogramDigiCollection> >::iterator itr;
  for(itr = hcalHistDigiCollHandleVec.begin(); itr != hcalHistDigiCollHandleVec.end(); itr++)
  {
    if(!itr->isValid())
    {
      cout << "Invalid digi collection found; continue" << endl;
      continue;
    }

    const HcalHistogramDigiCollection& hcalHistDigiColl=*(*itr);
    HcalHistogramDigiCollection::const_iterator idigi;
    for(idigi = hcalHistDigiColl.begin(); idigi != hcalHistDigiColl.end(); idigi++)
    {
      const HcalDetId detId = idigi->id();
      // check if digi is associated to this source tube (based on tube name only!)
      // otherwise, we keep all histograms...can get very large 
      if(selectDigiBasedOnTubeName_)
      {
        if(!isDigiAssociatedToSourceTube(detId,tubeName))
          continue;
      }

      //string histName = getRawHistName(eventNum,ieta,iphi,depth);
      treeChDenseIndex_[nChInEvent] = detId.denseIndex();
      int binValSum = 0;
      int binValSqrSum = 0;
      int nEntries = 0;
      // loop over histogram bins
      for(int ib = 0; ib < 32; ib++)
      {
        treeChHistBinContentCap0_[nChInEvent][ib] = idigi->get(0,ib); //getting RAW histogram itself for each CAPID
        treeChHistBinContentCap1_[nChInEvent][ib] = idigi->get(1,ib); 
        treeChHistBinContentCap2_[nChInEvent][ib] = idigi->get(2,ib); 
        treeChHistBinContentCap3_[nChInEvent][ib] = idigi->get(3,ib); 
        if(ib > 30) continue; // don't compute avg/rms using overflow bin
        binValSum+=ib*treeChHistBinContentCap0_[nChInEvent][ib];
        binValSum+=ib*treeChHistBinContentCap1_[nChInEvent][ib];
        binValSum+=ib*treeChHistBinContentCap2_[nChInEvent][ib];
        binValSum+=ib*treeChHistBinContentCap3_[nChInEvent][ib];
        binValSqrSum+=ib*ib*treeChHistBinContentCap0_[nChInEvent][ib];
        binValSqrSum+=ib*ib*treeChHistBinContentCap1_[nChInEvent][ib];
        binValSqrSum+=ib*ib*treeChHistBinContentCap2_[nChInEvent][ib];
        binValSqrSum+=ib*ib*treeChHistBinContentCap3_[nChInEvent][ib];
        nEntries+=treeChHistBinContentCap0_[nChInEvent][ib];
        nEntries+=treeChHistBinContentCap1_[nChInEvent][ib];
        nEntries+=treeChHistBinContentCap2_[nChInEvent][ib];
        nEntries+=treeChHistBinContentCap3_[nChInEvent][ib];
      }
      treeChHistMean_[nChInEvent] = nEntries > 0 ? binValSum/(float)nEntries : 0;
      treeChHistRMS_[nChInEvent] = nEntries > 0 ? sqrt(binValSqrSum/(float)nEntries - treeChHistMean_[nChInEvent]*treeChHistMean_[nChInEvent]) : 0;

      // used for looking at and saving raw hists
      if(printRawHistograms_)
      {
        const HcalElectronicsMap* readoutMap = pSetup->getHcalMapping();
        HcalElectronicsId eid = readoutMap->lookup(detId);
        int ieta = detId.ieta();
        int iphi = detId.iphi();
        int depth = detId.depth();
        cout << "event: " << eventNum << endl;
        cout << "electronicsID: " << eid << endl;
        cout << "iEta: "<< ieta << " iPhi: " << iphi << " Depth: " << depth << endl; 
        cout << *idigi << endl;
      }

      nChInEvent++;
    } // end loop over hist. digis

  }
  if(nChInEvent > MAXCHPEREVENT)
  {
    cout << "ERROR: nChInEvent " << nChInEvent << " > MAXCHPEREVENT=" << MAXCHPEREVENT << "; don't fill tree." << endl;
    return;
  }
  treeNChInEvent_ = nChInEvent;
  eventTree_->Fill();
}


// ------------ method called once each job just before starting event loop  ------------
void 
HCALSourceDataMonitor::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HCALSourceDataMonitor::endJob() 
{
  rootFile_->cd();
  eventTree_->Write();
  rootFile_->Close();
}

// ------------ method called when starting to processes a run  ------------
void 
HCALSourceDataMonitor::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
HCALSourceDataMonitor::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
HCALSourceDataMonitor::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
HCALSourceDataMonitor::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HCALSourceDataMonitor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HCALSourceDataMonitor);
