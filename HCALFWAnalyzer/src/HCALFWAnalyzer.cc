// -*- C++ -*-
//
// Package:    HCALFWAnalyzer
// Class:      HCALFWAnalyzer
// 
/**\class HCALFWAnalyzer HCALFWAnalyzer.cc Analysis/HCALFWAnalyzer/src/HCALFWAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Seth Cooper,32 4-B03,+41227675652,
//         Created:  Thu Dec 13 13:20:52 CET 2012
// $Id: HCALFWAnalyzer.cc,v 1.1 2013/03/13 11:08:54 scooper Exp $
//
//


// system include files
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <set>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/HcalRecHit/interface/HcalSourcePositionData.h"
#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"

//
// class declaration
//

class HCALFWAnalyzer : public edm::EDAnalyzer {
   public:
      explicit HCALFWAnalyzer(const edm::ParameterSet&);
      ~HCALFWAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      // ----------member data ---------------------------
      uint16_t capHist[4][32];
      uint16_t dataBuf[32];    

      bool checkDuplicates_;

      TH1F* avgHistValueAllDataHist;
      TH1F* histIntegralValueAllCapIDs;
      TH1F* histIntegralValueEachCapID[4];
      TH2F* messageCounterVsHistAverageTest;
      int naiveEvtNum;
      std::vector<double> evtNumbers;
      std::vector<double> orbitNumbers;
      std::vector<double> orbitNumberSecs;
      std::vector<double> indexVals;
      std::vector<double> messageCounterVals;
      std::vector<double> motorCurrentVals;
      std::vector<double> motorVoltageVals;
      std::vector<double> reelVals;
      std::vector<double> timeStamp1Vals;
      std::vector<double> triggerTimeStampVals;
      std::map< HcalDetId, std::vector<TH1F*> > individualHistsMap;
      std::set< HcalDetId> detIdsWithFullHistsSet;
      TH1F* numEmptyHistsVsEventHist;
      TH1F* numPartiallyEmptyHistsVsEventHist;
      TH1F* numFullHistsVsEventHist;
      TH1F* numHistsReadVsEventHist;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//
edm::Service<TFileService> fs;


// my intToString function
std::string intToString(int num)
{
    using namespace std;
    ostringstream myStream;
    myStream << num << flush;
    return (myStream.str ());
}

//
std::string getHistName(int ievent, int ieta, int iphi, int depth, int capId)
{
  std::string histName = "histEvent";
  histName+=intToString(ievent);
  histName+="_Ieta";
  histName+=intToString(ieta);
  histName+="_Iphi";
  histName+=intToString(iphi);
  histName+="_Depth";
  histName+=intToString(depth);
  histName+="_capID";
  histName+=intToString(capId);
  
  return histName;
}
//
std::string getDirName(int ieta, int iphi, int depth)
{
  std::string dirName = "digi_ieta";
  dirName+=intToString(ieta);
  dirName+="_Iphi";
  dirName+=intToString(iphi);
  dirName+="_Depth";
  dirName+=intToString(depth);
  
  return dirName;
}
//
int getEventFromHistName(std::string histName)
{
  int undIetaPos = histName.find("_Ieta");
  return atoi(histName.substr(9,undIetaPos-9).c_str());
}
//
int getIetaFromHistName(std::string histName)
{
  int undIetaPos = histName.find("_Ieta");
  int undIphiPos = histName.find("_Iphi");
  return atoi(histName.substr(undIetaPos+5,undIphiPos-undIetaPos-5).c_str());
}
//
int getIphiFromHistName(std::string histName)
{
  int undIphiPos = histName.find("_Iphi");
  int undDepthPos = histName.find("_Depth");
  return atoi(histName.substr(undIphiPos+5,undDepthPos-undIphiPos-5).c_str());
}
//
int getDepthFromHistName(std::string histName)
{
  int undDepthPos = histName.find("_Depth");
  int undCapIdPos = histName.find("_capID");
  return atoi(histName.substr(undDepthPos+6,undCapIdPos-undDepthPos-6).c_str());
}
//
int getCapIdFromHistName(std::string histName)
{
  int undCapIdPos = histName.find("_capID");
  return atoi(histName.substr(undCapIdPos+6,histName.length()-undCapIdPos-6).c_str());
}
//
void printHists(const TH1F* hist, TH1F* currentHist)
{
  std::cout << "binIndex\tPreviousHistContent\tCurrentHistContent" << std::endl;
  for(int i=1;i<hist->GetNbinsX()+1; ++i)
    std::cout << " " << std::setw(4) << i-1 << std::setw(10) << hist->GetBinContent(i) << std::setw(10) << currentHist->GetBinContent(i) << std::endl;
  std::cout << std::endl;
}
bool compareHists(const TH1F* hist, TH1F* currentHist)
{
  for(int i=1;i<hist->GetNbinsX()+1; ++i)
  {
    if(hist->GetBinContent(i) != currentHist->GetBinContent(i))
      return false;
  }

  return true;
}


//
// constructors and destructor
//
HCALFWAnalyzer::HCALFWAnalyzer(const edm::ParameterSet& iConfig):
  checkDuplicates_ (iConfig.getParameter<bool>("CheckForDuplicates"))
{

  avgHistValueAllDataHist = fs->make<TH1F>("avgHistValueAllData","Avg. Hist. Value All Data",1000,0,10);
  histIntegralValueAllCapIDs = fs->make<TH1F>("histIntegralAllCaps","Hist. Integral for All Cap IDs",70000,0,70000);
  messageCounterVsHistAverageTest = fs->make<TH2F>("messageCounterVsHistAverageTest",";Hist. Avg.;Message Number",600,2,8,400,150,550);
  numEmptyHistsVsEventHist = fs->make<TH1F>("numEmptyHistsVsEventHist",";event;hists",10000,1,10001);
  numPartiallyEmptyHistsVsEventHist = fs->make<TH1F>("numPartiallyEmptyHistsVsEventHist",";event;hists",10000,1,10001);
  numFullHistsVsEventHist = fs->make<TH1F>("numFullHistsVsEventHist",";event;hists",10000,1,10001);
  numHistsReadVsEventHist = fs->make<TH1F>("numHistsReadVsEventHist",";event;hists",10000,1,10001);
  for(int i=0; i < 4; ++i)
  {
    std::string histName = "histIntegralsCapID";
    histName+=intToString(i);
    std::string histTitle = "Hist. Integrals for cap ID ";
    histTitle+=intToString(i);
    histIntegralValueEachCapID[i] = fs->make<TH1F>(histName.c_str(),histTitle.c_str(),70000,0,70000);
  }

  //now do what ever initialization is needed
  for(int iBin = 0; iBin < 32; iBin++) dataBuf[iBin] = 0;

  naiveEvtNum = 0;
}


HCALFWAnalyzer::~HCALFWAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
HCALFWAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace std;

  // get source position data
  Handle<HcalSourcePositionData> hcalSourcePositionDataHandle;
  iEvent.getByType(hcalSourcePositionDataHandle);
  const HcalSourcePositionData* spd = hcalSourcePositionDataHandle.product();
  //cout << "message = " << spd->messageCounter() << endl;

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
  triggerTimeStampVals.push_back(trigtimebase);
  //char str[50];
  //sprintf(str, "  Trigger time: %s", ctime((time_t *)&trigtimebase));
  //cout << str;
  //sprintf(str, "                %d us\n", trigtimeusec);
  //cout << str;
  //cout << endl;
  //// orbit
  //cout << "  Orbit# =" << triggerData->orbitNumber() << endl;

  vector<Handle<HcalHistogramDigiCollection> > hcalHistDigiCollHandleVec;
  iEvent.getManyByType(hcalHistDigiCollHandleVec);

  int eventNum = iEvent.id().event();
  naiveEvtNum++;
  // fill evt, orbit
  evtNumbers.push_back(naiveEvtNum);
  orbitNumbers.push_back(triggerData->orbitNumber());
  orbitNumberSecs.push_back(88.9e-6*triggerData->orbitNumber());
  // fill index, msg, others
  indexVals.push_back(spd->indexCounter());
  messageCounterVals.push_back(spd->messageCounter());
  motorCurrentVals.push_back(spd->motorCurrent());
  motorVoltageVals.push_back(spd->motorVoltage());
  reelVals.push_back(spd->reelCounter());
  // consider what comes out as "timestamp2" in payload as usec for driver ts?
  int timebase =0; int timeusec=0;
  spd->getDriverTimestamp(timebase,timeusec);
  // trim seconds off of usec and add to base
  timeusec %= 1000000;
  timebase += timeusec/1000000;
  timeStamp1Vals.push_back(timebase);
  //char str[50];
  //sprintf(str, "  Driver Timestamp : %s", ctime((time_t *)&timebase));
  //s << str;

  // make subdirs for individual hists
  string directoryName = "Event";
  directoryName+=intToString(eventNum);
  TFileDirectory subDir = fs->mkdir(directoryName.c_str());

  int numEmptyHists = 0;
  int numPartiallyEmptyHists = 0;
  int numFullHists = 0;
  int numHistsRead = 0;
  TH1F* thisHist[4]; // to hold individual histograms
  unsigned int sum=0;
  double average=0;
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
      // Finding ieta, iphi, depth for each individual digi and dumping
      int ieta = idigi->id().ieta();
      int iphi = idigi->id().iphi();
      int depth = idigi->id().depth();
      // SIC -- use for looking at raw hists
      cout << "event: " << naiveEvtNum << endl;
      cout << "iEta: "<< ieta << " iPhi: " << iphi << " Depth: " << depth << endl; 
      cout << *idigi << endl;
      const HcalDetId detId = idigi->id();

      int maxVals[4];
      for(int i=0;i<4;++i)
        maxVals[i] = -1;
      // making all these dirs leads to slowdownm even without hists saved
      //TFileDirectory digiDir = subDir.mkdir(getDirName(ieta,iphi,depth));
      // capid loop
      for(int icap = 0; icap < 4; icap++)
      { 
        // save this histo
        string histName = getHistName(naiveEvtNum,ieta,iphi,depth,icap);
        // to keep track of all previous hists seen
        // takes up a lot of memory
        //individualHistsMap[detId].push_back(digiDir.make<TH1F>(histName.c_str(),histName.c_str(),32,0,32));
        if(checkDuplicates_)
        {
          individualHistsMap[detId].push_back(new TH1F(histName.c_str(),histName.c_str(),32,0,32));
          thisHist[icap] = individualHistsMap[detId].back();
        }
        else
          thisHist[icap] = new TH1F(histName.c_str(),histName.c_str(),32,0,32);
        // even saving all hists takes up lots of memory it seems
        //thisHist[icap] = digiDir.make<TH1F>(histName.c_str(),histName.c_str(),32,0,32);
        // loop over histogram bins
        for(int ib = 0; ib < 32; ib++)
        {
          capHist[icap][ib]=idigi->get(icap,ib); //getting RAW histogram itself for each CAPID
          //cout << "Cap bin: " << icap << " " << ib << " Value " << idigi->get(icap,ib) << endl;
          thisHist[icap]->Fill(ib,idigi->get(icap,ib));
        }
        maxVals[icap] = thisHist[icap]->GetMaximum();
        numHistsRead++;
        if(maxVals[icap]==0)
          numEmptyHists++;
        else if(thisHist[icap]->GetMaximumBin()==32 && thisHist[icap]->Integral()==thisHist[icap]->GetBinContent(32))
          numPartiallyEmptyHists++;
        else
        {
          detIdsWithFullHistsSet.insert(detId);
          numFullHists++;
        }
      }  

      // check integral
      for(int icap=0; icap<4; icap++)
      {
        int integral = thisHist[icap]->Integral();
        if(integral != 65532)
          cout << "Hist for detId: " << detId << " icap=" << icap << " has integral " << integral << " != 65532." << endl;
      }

      // takes up a lot of memory
      // now let's see if we've seen this event's maximum value before
      for(vector<TH1F*>::const_iterator itr = individualHistsMap[detId].begin();
          itr != individualHistsMap[detId].end(); ++itr)
      {
        if(naiveEvtNum==getEventFromHistName((*itr)->GetName()))
          continue; // don't look at the same event

        // ignore previous hists that are empty or partially empty
        int prevHistMaxVal = (*itr)->GetMaximum();
        if(prevHistMaxVal==0)
          continue;
        else if((*itr)->GetMaximumBin()==32 && (*itr)->Integral()==(*itr)->GetBinContent(32))
          continue;

        //if(thisMax==0)
        //{
        //  // we should have 3 capID's with zero, and some number of entries in the last bin of the 4th
        //  // for the current hist
        //  // check to make sure that 
        //  int numZeroIntegralCaps = 0;
        //  int indexNonZeroIntegral = -1;
        //  for(int icap=0; icap<4; icap++)
        //  {
        //    if(thisMax==0 && thisHist[icap]->Integral()==0)
        //      numZeroIntegralCaps++;
        //    else if(thisMax!=0)
        //    {
        //      indexNonZeroIntegral = icap;
        //    }
        //  }
        //  if(numZeroIntegralCaps != 3)
        //  {
        //    cout << "STRANGE: Found " << numZeroIntegralCaps << " hists with integral 0 and max 0 instead of 3 expected" << endl;
        //    cout << "Looking at DetId: " << detId <<
        //      " (cap" << indexNonZeroIntegral << "); previous histogram (cap" << getCapIdFromHistName(string((*itr)->GetName())) << ") from event " << getEventFromHistName(string((*itr)->GetName())) << endl;
        //    printHists(*itr,thisHist[indexNonZeroIntegral]);
        //  }
        //  if(!(thisHist[indexNonZeroIntegral]->GetMaximumBin()==32 && thisHist[indexNonZeroIntegral]->Integral() == thisHist[indexNonZeroIntegral]->GetBinContent(32)))
        //  {
        //    cout << "STRANGE: Previous hist max val was zero; this hist looks like it has some non-trivial content" << endl;
        //    cout << "Looking at DetId: " << detId << ", max=0" <<
        //      " (cap" << indexNonZeroIntegral << "); previous histogram (cap" << getCapIdFromHistName(string((*itr)->GetName())) << ") from event " << getEventFromHistName(string((*itr)->GetName())) << endl;
        //    printHists(*itr,thisHist[indexNonZeroIntegral]);
        //  }
        //  continue;
        //}

        // check previous hist against the 4 capIDs of current digi's hist
        for(int icap=0; icap<4; icap++)
        {
          // check maxima
          if(maxVals[icap]==prevHistMaxVal)
          {
            // can indicate if maxes are shared here, but this is not so uncommon
            // mark/print if hists are identical
            if(compareHists(*itr,thisHist[icap]))
            {
              cout << "This one is a duplicate!" << "Looking at DetId: " << detId << ", found current max of " << maxVals[icap] <<
                " (cap" << icap << ") in previous histogram (cap" << getCapIdFromHistName(string((*itr)->GetName())) << ") from event " << getEventFromHistName(string((*itr)->GetName())) << endl;
              //printHists(*itr,thisHist[icap]);
            }
          }

        }
      }

      //XXX REMOVE?
      for(int i=0;i<4;++i)
        delete thisHist[i];

      
      //Summing up histograms, please check if correct
      for(int ib = 0; ib < 32; ib++)
        dataBuf[ib] = capHist[0][ib]+capHist[1][ib]+capHist[2][ib]+capHist[3][ib];

      //Finding average of RAW (HTR) histogram
      for(int iBin = 0; iBin < 32; iBin++)
      {
        //cout<<"Bin: "<<iBin<<" Value "<<  dataBuf[iBin]<<endl;
        sum += dataBuf[iBin];
        average += (dataBuf[iBin]) * (iBin + 0.5);
      }

      if(sum > 0)
        average /= sum;
      // Filling root histogram, All iphi, ieta, depth together
      avgHistValueAllDataHist->Fill(average); 
      // RAW (HTR) Histogram average, THIS IS WHAT WE NEED
      //cout << "Average = " << average << endl;
      if(ieta==3 && iphi==16 && depth==1)
      {
        cout << "Average this tile = " << average << " eventNumber = " << eventNum << endl;
        messageCounterVsHistAverageTest->Fill(average,spd->messageCounter());
      }

      // calculate integrals and fill histograms
      for(int capId=0; capId < 4; ++capId)
      {
        double integralThisCapID = 0;
        for(int iBin = 0; iBin < 32; iBin++)
        {
          integralThisCapID+=capHist[capId][iBin];
        }
        histIntegralValueEachCapID[capId]->Fill(integralThisCapID);
        histIntegralValueAllCapIDs->Fill(integralThisCapID);
      }
    } // end loop over hist. digis
  }
  // after reading digis, output number of hists read, empty, etc.
  cout << "event: " << naiveEvtNum << "; numHistsRead = " << numHistsRead << "; numEmptyHists = " << numEmptyHists << " numPartiallyEmptyHists" <<
    numPartiallyEmptyHists << " numFullHists = " << numFullHists << " sum = " << numEmptyHists+numPartiallyEmptyHists+numFullHists << endl;

  // fill hists
  numEmptyHistsVsEventHist->Fill(naiveEvtNum,numEmptyHists);
  numPartiallyEmptyHistsVsEventHist->Fill(naiveEvtNum,numPartiallyEmptyHists);
  numFullHistsVsEventHist->Fill(naiveEvtNum,numFullHists);
  numHistsReadVsEventHist->Fill(naiveEvtNum,numHistsRead);

}


// ------------ method called once each job just before starting event loop  ------------
void 
HCALFWAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HCALFWAnalyzer::endJob() 
{
  using namespace std;

  TGraph* eventNumVsOrbitNumGraph = fs->make<TGraph>(evtNumbers.size(),&(*orbitNumbers.begin()),&(*evtNumbers.begin()));
  eventNumVsOrbitNumGraph->Draw();
  eventNumVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  eventNumVsOrbitNumGraph->GetYaxis()->SetTitle("event");
  eventNumVsOrbitNumGraph->SetName("naiveEventNumVsOrbitNumGraph");

  TGraph* eventNumVsOrbitNumSecsGraph = fs->make<TGraph>(evtNumbers.size(),&(*orbitNumberSecs.begin()),&(*evtNumbers.begin()));
  eventNumVsOrbitNumSecsGraph->Draw();
  eventNumVsOrbitNumSecsGraph->GetXaxis()->SetTitle("orbit [s]");
  eventNumVsOrbitNumSecsGraph->GetYaxis()->SetTitle("event");
  eventNumVsOrbitNumSecsGraph->SetName("naiveEventNumVsOrbitNumSecsGraph");

  TGraph* messageCounterVsOrbitNumGraph = fs->make<TGraph>(messageCounterVals.size(),&(*orbitNumberSecs.begin()),&(*messageCounterVals.begin()));
  messageCounterVsOrbitNumGraph->SetName("messageCounterVsOrbitNumGraph");
  messageCounterVsOrbitNumGraph->Draw();
  messageCounterVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  messageCounterVsOrbitNumGraph->GetYaxis()->SetTitle("message");
  messageCounterVsOrbitNumGraph->SetTitle("");

  TGraph* indexVsOrbitNumGraph = fs->make<TGraph>(indexVals.size(),&(*orbitNumberSecs.begin()),&(*indexVals.begin()));
  indexVsOrbitNumGraph->SetName("indexVsOrbitNumGraph");
  indexVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  indexVsOrbitNumGraph->GetYaxis()->SetTitle("index");
  indexVsOrbitNumGraph->SetTitle("");

  TGraph* motorCurrentVsOrbitNumGraph = fs->make<TGraph>(motorCurrentVals.size(),&(*orbitNumberSecs.begin()),&(*motorCurrentVals.begin()));
  motorCurrentVsOrbitNumGraph->SetName("motorCurrentVsOrbitNumGraph");
  motorCurrentVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  motorCurrentVsOrbitNumGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsOrbitNumGraph->SetTitle("");

  TGraph* motorVoltageVsOrbitNumGraph = fs->make<TGraph>(motorVoltageVals.size(),&(*orbitNumberSecs.begin()),&(*motorVoltageVals.begin()));
  motorVoltageVsOrbitNumGraph->SetName("motorVoltageVsOrbitNumGraph");
  motorVoltageVsOrbitNumGraph->Draw();
  motorVoltageVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  motorVoltageVsOrbitNumGraph->GetYaxis()->SetTitle("motor voltage [V]");
  motorVoltageVsOrbitNumGraph->SetTitle("");

  TGraph* motorCurrentVsReelPosGraph = fs->make<TGraph>(motorCurrentVals.size(),&(*reelVals.begin()),&(*motorCurrentVals.begin()));
  motorCurrentVsReelPosGraph->SetName("motorCurrentVsReelPosGraph");
  motorCurrentVsReelPosGraph->Draw();
  motorCurrentVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  motorCurrentVsReelPosGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsReelPosGraph->SetTitle("");

  TGraph* motorVoltageVsReelPosGraph = fs->make<TGraph>(motorVoltageVals.size(),&(*reelVals.begin()),&(*motorVoltageVals.begin()));
  motorVoltageVsReelPosGraph->SetName("motorVoltageVsReelPosGraph");
  motorVoltageVsReelPosGraph->Draw();
  motorVoltageVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  motorVoltageVsReelPosGraph->GetYaxis()->SetTitle("motor voltage [V]");
  motorVoltageVsReelPosGraph->SetTitle("");

  TGraph* reelVsOrbitNumGraph = fs->make<TGraph>(reelVals.size(),&(*orbitNumberSecs.begin()),&(*reelVals.begin()));
  reelVsOrbitNumGraph->SetName("reelVsOrbitNumGraph");
  reelVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  reelVsOrbitNumGraph->GetYaxis()->SetTitle("reel [mm]");
  reelVsOrbitNumGraph->SetTitle("");

  TGraph* triggerTimestampVsOrbitNumGraph = fs->make<TGraph>(triggerTimeStampVals.size(),&(*orbitNumberSecs.begin()),&(*triggerTimeStampVals.begin()));
  triggerTimestampVsOrbitNumGraph->SetName("triggerTimestampVsOrbitNumGraph");
  triggerTimestampVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  triggerTimestampVsOrbitNumGraph->GetYaxis()->SetTitle("trigger timestamp [s]");
  triggerTimestampVsOrbitNumGraph->SetTitle("");

  TGraph* timeStamp1VsOrbitNumGraph = fs->make<TGraph>(timeStamp1Vals.size(),&(*orbitNumberSecs.begin()),&(*timeStamp1Vals.begin()));
  timeStamp1VsOrbitNumGraph->SetName("timeStamp1VsOrbitNumGraph");
  timeStamp1VsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  timeStamp1VsOrbitNumGraph->GetYaxis()->SetTitle("timestamp1 [s]");
  timeStamp1VsOrbitNumGraph->SetTitle("");

  TGraph* triggerTimeStampVsTimeStamp1Graph = fs->make<TGraph>(timeStamp1Vals.size(),&(*timeStamp1Vals.begin()),&(*triggerTimeStampVals.begin()));
  triggerTimeStampVsTimeStamp1Graph->SetName("triggerTimeStampVsTimeStamp1Graph");
  triggerTimeStampVsTimeStamp1Graph->GetXaxis()->SetTitle("timestamp1 [s]");
  triggerTimeStampVsTimeStamp1Graph->GetYaxis()->SetTitle("trigger timestamp [s]");
  triggerTimeStampVsTimeStamp1Graph->SetTitle("");

  cout << "Number of digis/channels read: " << individualHistsMap.size() << endl;
  cout << "Number of digis with at least one full histogram: " << detIdsWithFullHistsSet.size() << endl;
}

// ------------ method called when starting to processes a run  ------------
void 
HCALFWAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
HCALFWAnalyzer::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
HCALFWAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
HCALFWAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HCALFWAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HCALFWAnalyzer);
