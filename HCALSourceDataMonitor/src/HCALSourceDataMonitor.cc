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
// $Id: HCALSourceDataMonitor.cc,v 1.6 2013/07/15 17:18:31 scooper Exp $
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
//
// class declaration
//
struct RawHistoData
{
  RawHistoData() { }
  RawHistoData(std::string setTubeName, HcalDetId setDetId)
  {
    tubeName = setTubeName;
    detId = setDetId;
  }

  HcalDetId detId;
  std::string tubeName;
  std::vector<double> eventNumbers;
  std::vector<double> histoAverages;
  std::vector<double> histoRMSs;
};

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

      void startHtml();
      void appendHtml(std::string tubeName, std::vector<std::string>& imageNames);
      void finishHtml();
      std::string getBlockEventDirName(int event);
      // ----------member data ---------------------------
      std::string rootFileName_;
      std::string htmlFileName_;
      int newRowEvery_;
      int thumbnailSize_;
      bool outputRawHistograms_;
      bool selectDigiBasedOnTubeName_;
      int naiveEvtNum_;
      TFile* rootFile_;
      std::vector<RawHistoData> rawHistoDataVec_;
      std::vector<double> evtNumbers_;
      std::vector<double> orbitNumbers_;
      std::vector<double> orbitNumberSecs_;
      std::vector<double> indexVals_;
      std::vector<double> messageCounterVals_;
      std::vector<double> motorCurrentVals_;
      std::vector<double> motorVoltageVals_;
      std::vector<double> reelVals_;
      std::vector<double> timeStamp1Vals_;
      std::vector<double> triggerTimeStampVals_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
std::string intToString(int num)
{
    using namespace std;
    ostringstream myStream;
    myStream << num << flush;
    return (myStream.str ());
}
//
std::string getRawHistName(int ievent, int ieta, int iphi, int depth, int capId)
{
  std::string histName = "rawHistEvent";
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
std::string getRawHistName(int ievent, int ieta, int iphi, int depth)
{
  std::string histName = "rawHistEvent";
  histName+=intToString(ievent);
  histName+="_Ieta";
  histName+=intToString(ieta);
  histName+="_Iphi";
  histName+=intToString(iphi);
  histName+="_Depth";
  histName+=intToString(depth);
  
  return histName;
}
//
std::string getGraphName(const HcalDetId& detId, std::string tubeName)
{
  std::string histName=tubeName;
  histName+="_Ieta";
  histName+=intToString(detId.ieta());
  histName+="_Iphi";
  histName+=intToString(detId.iphi());
  histName+="_Depth";
  histName+=intToString(detId.depth());
  
  return histName;
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
  return atoi(histName.substr(undDepthPos+6,histName.length()-undDepthPos-6).c_str());
}
//
bool isDigiAssociatedToSourceTube(const HcalDetId& detId, std::string tubeName)
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

//
// constructors and destructor
//
HCALSourceDataMonitor::HCALSourceDataMonitor(const edm::ParameterSet& iConfig) :
  rootFileName_ (iConfig.getUntrackedParameter<std::string>("RootFileName","hcalSourceDataMon.root")),
  htmlFileName_ (iConfig.getUntrackedParameter<std::string>("HtmlFileName","test.html")),
  newRowEvery_ (iConfig.getUntrackedParameter<int>("NewRowEvery",3)),
  thumbnailSize_ (iConfig.getUntrackedParameter<int>("ThumbnailSize",350)),
  outputRawHistograms_ (iConfig.getUntrackedParameter<bool>("OutputRawHistograms",false)),
  selectDigiBasedOnTubeName_ (iConfig.getUntrackedParameter<bool>("SelectDigiBasedOnTubeName",true))
{
  //now do what ever initialization is neededCheckForDuplicates
  naiveEvtNum_ = 0;
  rootFile_ = new TFile(rootFileName_.c_str(),"recreate");
  rootFile_->cd();
  

}


HCALSourceDataMonitor::~HCALSourceDataMonitor()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//
void HCALSourceDataMonitor::startHtml()
{
  using namespace std;
  ofstream htmlFile(htmlFileName_);
  if(htmlFile.is_open())
  {
    htmlFile << "<!DOCTYPE html>\n";
    htmlFile << "<html>\n";
    htmlFile << "<head>\n";
    htmlFile << "<title>Histogram Data" << "</title>\n";
    htmlFile << "</head>\n";
    htmlFile << "<body>\n";
    htmlFile << "<h2>Histogram Data</h2>\n";
    htmlFile << "<hr>\n";
    htmlFile.close();
  }
}
//
void HCALSourceDataMonitor::appendHtml(std::string tubeName, std::vector<std::string>& imageNames)
{
  using namespace std;
  ofstream htmlFile(htmlFileName_, ios::out | ios::app);
  if(htmlFile.is_open())
  {
    htmlFile << "<h3>Tube name: " << tubeName << "</h3>\n";
    htmlFile << "<table>\n";
    htmlFile << "<tr>\n";
    int counter = 0;
    for(std::vector<std::string>::const_iterator imageName = imageNames.begin(); imageName != imageNames.end();
        ++imageName)
    {
      if(counter % newRowEvery_ == 0)
      {
        htmlFile << "</tr>\n";
        htmlFile << "<tr>\n";
      }
      htmlFile << "<td><a href=\"" << *imageName << "\"><img width=" << thumbnailSize_ << " src=\"" << *imageName << "\"></a></td>\n";
      ++counter;
    }
    htmlFile << "</tr>\n";
    htmlFile << "</table>\n";
    htmlFile << "<hr>\n";
    htmlFile.close();
  }
}
//
void HCALSourceDataMonitor::finishHtml()
{
  using namespace std;
  ofstream htmlFile(htmlFileName_, ios::out | ios::app);
  if(htmlFile.is_open())
  {
    htmlFile << "<a href=\"" << rootFileName_ << "\">Download Root file</a>\n";
    htmlFile << "</body>\n";
    htmlFile << "</html>\n";
    htmlFile.close();
  }
}
//
std::string HCALSourceDataMonitor::getBlockEventDirName(int event)
{
  int numEventsPerDir = 1000;
  int blockDirNum = (event-1) / numEventsPerDir;
  int firstEventNum = blockDirNum*numEventsPerDir + 1;
  int lastEventNum = (blockDirNum+1)*numEventsPerDir;
  std::string superDirName = "events";
  superDirName+=intToString(firstEventNum);
  superDirName+="to";
  superDirName+=intToString(lastEventNum);
  return superDirName;
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
  const HcalElectronicsMap* readoutMap = pSetup->getHcalMapping();

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
  triggerTimeStampVals_.push_back(trigtimebase);
  //char str[50];
  //sprintf(str, "  Trigger time: %s", ctime((time_t *)&trigtimebase));
  //cout << str;
  //sprintf(str, "                %d us\n", trigtimeusec);
  //cout << str;
  //cout << endl;

  // fill evt, orbit
  evtNumbers_.push_back(naiveEvtNum_);
  orbitNumbers_.push_back(triggerData->orbitNumber());
  orbitNumberSecs_.push_back(88.9e-6*triggerData->orbitNumber());
  // fill index, msg, others
  indexVals_.push_back(spd->indexCounter());
  messageCounterVals_.push_back(spd->messageCounter());
  motorCurrentVals_.push_back(spd->motorCurrent());
  motorVoltageVals_.push_back(spd->motorVoltage());
  reelVals_.push_back(spd->reelCounter());
  // consider what comes out as "timestamp2" in payload as usec for driver ts?
  int timebase =0; int timeusec=0;
  spd->getDriverTimestamp(timebase,timeusec);
  // trim seconds off of usec and add to base
  timeusec %= 1000000;
  timebase += timeusec/1000000;
  timeStamp1Vals_.push_back(timebase);
  //char str[50];
  //sprintf(str, "  Driver Timestamp : %s", ctime((time_t *)&timebase));
  //s << str;
  //FIXME TODO: get tube name out of source position data
  //string tubeName = "H2_HB_PHI11_LAYER0_SRCTUBE"; // example tube for H2
  //string tubeName = "HFM01_ETA29_PHI55_T1A_SRCTUBE"; // example tube for HF/P5
  string tubeName = spd->tubeNameFromCoord();

  TDirectory* tubeDir = (TDirectory*) rootFile_->GetDirectory(tubeName.c_str());
  if(!tubeDir)
    tubeDir = rootFile_->mkdir(tubeName.c_str());

  vector<Handle<HcalHistogramDigiCollection> > hcalHistDigiCollHandleVec;
  iEvent.getManyByType(hcalHistDigiCollHandleVec);

  int eventNum = iEvent.id().event();
  string blockDirName = getBlockEventDirName(eventNum);
  string blockDirPath = tubeName;
  blockDirPath+="/";
  blockDirPath+=blockDirName;
  TDirectory* blockEventDir = (TDirectory*) rootFile_->GetDirectory(blockDirPath.c_str());
  if(!blockEventDir)
    blockEventDir = tubeDir->mkdir(blockDirName.c_str());
    
  string directoryName = "event";
  directoryName+=intToString(eventNum);
  string dirPath = blockDirPath;
  dirPath+="/";
  dirPath+=directoryName;
  TDirectory* subDir = (TDirectory*) rootFile_->GetDirectory(dirPath.c_str());
  if(!subDir)
    subDir = blockEventDir->mkdir(directoryName.c_str());
  subDir->cd();

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
      TH1F* thisHist; // to hold individual histograms
      const HcalDetId detId = idigi->id();
      int ieta = detId.ieta();
      int iphi = detId.iphi();
      int depth = detId.depth();
      // check if digi is associated to this source tube (based on tube name only!)
      // otherwise, we keep all histograms...can get very large 
      if(selectDigiBasedOnTubeName_)
      {
        if(!isDigiAssociatedToSourceTube(detId,tubeName))
          continue;
      }

      // used for looking at raw hists
      if(outputRawHistograms_)
      {
        HcalElectronicsId eid = readoutMap->lookup(detId);
        cout << "event: " << eventNum << endl;
        cout << "electronicsID: " << eid << endl;
        cout << "iEta: "<< ieta << " iPhi: " << iphi << " Depth: " << depth << endl; 
        cout << *idigi << endl;
      }

      string histName = getRawHistName(eventNum,ieta,iphi,depth);
      thisHist = new TH1F(histName.c_str(),histName.c_str(),32,0,32);
      thisHist->Sumw2();
      // loop over histogram bins
      int totalEnts = 0;
      //SIC FIXME: ignore bin 31 (overflow/error) for now
      //for(int ib = 0; ib < 32; ib++)
      for(int ib = 0; ib < 31; ib++)
      {
        int binValSum = 0;
        // capid loop
        for(int icap = 0; icap < 4; icap++)
        { 
          binValSum+=idigi->get(icap,ib); //getting RAW histogram itself for each CAPID
          totalEnts+=idigi->get(icap,ib);
        }
        for(int content = 0; content < binValSum; ++content)
          thisHist->Fill(ib);
        //thisHist->SetBinContent(thisHist->FindBin(ib),binValSum);
        //thisHist->SetBinError(thisHist->FindBin(ib),sqrt(binValSum));
      }  
      //thisHist->SetEntries(totalEnts);
      thisHist->Write();
      //cout << "GetMean: (before): " << thisHist->GetMean() << endl;
      //thisHist->GetXaxis()->SetRange(1,31); // compute mean/RMS excluding last bin
      //cout << "GetMean: (after): " << thisHist->GetMean() << endl;
      vector<RawHistoData>::iterator histoItr = rawHistoDataVec_.begin();
      while(rawHistoDataVec_.size() > 0 && histoItr != rawHistoDataVec_.end() && ((tubeName != histoItr->tubeName) || (detId != histoItr->detId)))
        histoItr++;
      RawHistoData* thisHistoData;
      if(histoItr!=rawHistoDataVec_.end())
        thisHistoData = &(*histoItr);
      else
        thisHistoData = new RawHistoData(tubeName,detId);
        
      thisHistoData->eventNumbers.push_back(eventNum);
      thisHistoData->histoAverages.push_back(thisHist->GetMean());
      thisHistoData->histoRMSs.push_back(thisHist->GetRMS());
      if(histoItr==rawHistoDataVec_.end())
        rawHistoDataVec_.push_back(*thisHistoData);

      delete thisHist;


    } // end loop over hist. digis
  }

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
  using namespace std;
  // now make plots of avgVal vs. event number
  rootFile_->cd();
  vector<string> imageNamesThisTube; 
  set<string> tubeNameSet;
  for(vector<RawHistoData>::const_iterator itr = rawHistoDataVec_.begin();
      itr != rawHistoDataVec_.end(); ++itr)
  {
    tubeNameSet.insert(itr->tubeName);
    //XXX DEBUG
    cout << "SIC DEBUG: insert tubeName = " << itr->tubeName << endl;
  }

  startHtml();
  for(set<string>::const_iterator tubeItr = tubeNameSet.begin(); tubeItr != tubeNameSet.end(); ++tubeItr)
  {
    string thisTube = *tubeItr;
    imageNamesThisTube.clear();
    //XXX DEBUG
    cout << "SIC DEBUG: thisTube = " << thisTube << endl;
    for(vector<RawHistoData>::const_iterator itr = rawHistoDataVec_.begin();
        itr != rawHistoDataVec_.end(); ++itr)
    {
      RawHistoData data = *(itr);
      if(data.tubeName != thisTube) // only consider current tube
        continue;

      // compute avg y value for plot scaling
      double yavg = 0;
      int count = 0;
      for(std::vector<double>::const_iterator i = data.histoAverages.begin(); i != data.histoAverages.end(); ++i)
      {
        yavg+=*i;
        count++;
      }
      yavg/=count;
      // make eventNum errs
      vector<double> eventNumErrs;
      for(std::vector<double>::const_iterator i = data.eventNumbers.begin(); i != data.eventNumbers.end(); ++i)
        eventNumErrs.push_back(0);
      // make plot
      //TGraphErrors* thisGraph = new TGraphErrors(data.eventNumbers.size(),&(*data.eventNumbers.begin()),
      //    &(*data.histoAverages.begin()),&(*eventNumErrs.begin()),&(*data.histoRMSs.begin()));
      TGraph* thisGraph = new TGraph(data.eventNumbers.size(),&(*data.eventNumbers.begin()),&(*data.histoAverages.begin()));
      string graphName = getGraphName(itr->detId,itr->tubeName);
      thisGraph->SetTitle(graphName.c_str());
      thisGraph->SetName(graphName.c_str());
      thisGraph->Draw();
      thisGraph->GetXaxis()->SetTitle("Event");
      //thisGraph->GetYaxis()->SetTitle("hist. mean+/-RMS [ADC]");
      thisGraph->GetYaxis()->SetTitle("hist. mean [ADC]");
      thisGraph->GetYaxis()->SetRangeUser(yavg-0.5,yavg+0.5);
      thisGraph->SetMarkerStyle(33);
      thisGraph->SetMarkerSize(0.8);
      TCanvas* canvas = new TCanvas("canvas","canvas",900,600);
      canvas->cd();
      thisGraph->Draw("ap");
      thisGraph->Write();
      canvas->Print((graphName+".png").c_str());
      imageNamesThisTube.push_back(graphName+".png");
      delete canvas;
      delete thisGraph;
    }
    appendHtml(thisTube,imageNamesThisTube);
  }
  finishHtml();

  TDirectory* dInfoPlotsDir = rootFile_->mkdir("driverInfoPlots");
  dInfoPlotsDir->cd();
  // make driver info graphs
  TGraph* eventNumVsOrbitNumGraph = new TGraph(evtNumbers_.size(),&(*orbitNumbers_.begin()),&(*evtNumbers_.begin()));
  eventNumVsOrbitNumGraph->Draw();
  eventNumVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  eventNumVsOrbitNumGraph->GetYaxis()->SetTitle("event");
  eventNumVsOrbitNumGraph->SetName("naiveEventNumVsOrbitNumGraph");
  eventNumVsOrbitNumGraph->Write();

  TGraph* eventNumVsOrbitNumSecsGraph = new TGraph(evtNumbers_.size(),&(*orbitNumberSecs_.begin()),&(*evtNumbers_.begin()));
  eventNumVsOrbitNumSecsGraph->Draw();
  eventNumVsOrbitNumSecsGraph->GetXaxis()->SetTitle("orbit [s]");
  eventNumVsOrbitNumSecsGraph->GetYaxis()->SetTitle("event");
  eventNumVsOrbitNumSecsGraph->SetName("naiveEventNumVsOrbitNumSecsGraph");
  eventNumVsOrbitNumSecsGraph->Write();

  TGraph* messageCounterVsOrbitNumGraph = new TGraph(messageCounterVals_.size(),&(*orbitNumberSecs_.begin()),&(*messageCounterVals_.begin()));
  messageCounterVsOrbitNumGraph->SetName("messageCounterVsOrbitNumGraph");
  messageCounterVsOrbitNumGraph->Draw();
  messageCounterVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  messageCounterVsOrbitNumGraph->GetYaxis()->SetTitle("message");
  messageCounterVsOrbitNumGraph->SetTitle("");
  messageCounterVsOrbitNumGraph->Write();

  TGraph* indexVsOrbitNumGraph = new TGraph(indexVals_.size(),&(*orbitNumberSecs_.begin()),&(*indexVals_.begin()));
  indexVsOrbitNumGraph->SetName("indexVsOrbitNumGraph");
  indexVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  indexVsOrbitNumGraph->GetYaxis()->SetTitle("index");
  indexVsOrbitNumGraph->SetTitle("");
  indexVsOrbitNumGraph->Write();

  TGraph* motorCurrentVsOrbitNumGraph = new TGraph(motorCurrentVals_.size(),&(*orbitNumberSecs_.begin()),&(*motorCurrentVals_.begin()));
  motorCurrentVsOrbitNumGraph->SetName("motorCurrentVsOrbitNumGraph");
  motorCurrentVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  motorCurrentVsOrbitNumGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsOrbitNumGraph->SetTitle("");
  motorCurrentVsOrbitNumGraph->Write();

  TGraph* motorVoltageVsOrbitNumGraph = new TGraph(motorVoltageVals_.size(),&(*orbitNumberSecs_.begin()),&(*motorVoltageVals_.begin()));
  motorVoltageVsOrbitNumGraph->SetName("motorVoltageVsOrbitNumGraph");
  motorVoltageVsOrbitNumGraph->Draw();
  motorVoltageVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  motorVoltageVsOrbitNumGraph->GetYaxis()->SetTitle("motor voltage [V]");
  motorVoltageVsOrbitNumGraph->SetTitle("");
  motorVoltageVsOrbitNumGraph->Write();

  TGraph* motorCurrentVsReelPosGraph = new TGraph(motorCurrentVals_.size(),&(*reelVals_.begin()),&(*motorCurrentVals_.begin()));
  motorCurrentVsReelPosGraph->SetName("motorCurrentVsReelPosGraph");
  motorCurrentVsReelPosGraph->Draw();
  motorCurrentVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  motorCurrentVsReelPosGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsReelPosGraph->SetTitle("");
  motorCurrentVsReelPosGraph->Write();

  TGraph* motorVoltageVsReelPosGraph = new TGraph(motorVoltageVals_.size(),&(*reelVals_.begin()),&(*motorVoltageVals_.begin()));
  motorVoltageVsReelPosGraph->SetName("motorVoltageVsReelPosGraph");
  motorVoltageVsReelPosGraph->Draw();
  motorVoltageVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  motorVoltageVsReelPosGraph->GetYaxis()->SetTitle("motor voltage [V]");
  motorVoltageVsReelPosGraph->SetTitle("");
  motorVoltageVsReelPosGraph->Write();

  TGraph* reelVsOrbitNumGraph = new TGraph(reelVals_.size(),&(*orbitNumberSecs_.begin()),&(*reelVals_.begin()));
  reelVsOrbitNumGraph->SetName("reelVsOrbitNumGraph");
  reelVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  reelVsOrbitNumGraph->GetYaxis()->SetTitle("reel [mm]");
  reelVsOrbitNumGraph->SetTitle("");
  reelVsOrbitNumGraph->Write();

  TGraph* triggerTimestampVsOrbitNumGraph = new TGraph(triggerTimeStampVals_.size(),&(*orbitNumberSecs_.begin()),&(*triggerTimeStampVals_.begin()));
  triggerTimestampVsOrbitNumGraph->SetName("triggerTimestampVsOrbitNumGraph");
  triggerTimestampVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  triggerTimestampVsOrbitNumGraph->GetYaxis()->SetTitle("trigger timestamp [s]");
  triggerTimestampVsOrbitNumGraph->SetTitle("");
  triggerTimestampVsOrbitNumGraph->Write();

  TGraph* timeStamp1VsOrbitNumGraph = new TGraph(timeStamp1Vals_.size(),&(*orbitNumberSecs_.begin()),&(*timeStamp1Vals_.begin()));
  timeStamp1VsOrbitNumGraph->SetName("timeStamp1VsOrbitNumGraph");
  timeStamp1VsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  timeStamp1VsOrbitNumGraph->GetYaxis()->SetTitle("timestamp1 [s]");
  timeStamp1VsOrbitNumGraph->SetTitle("");
  timeStamp1VsOrbitNumGraph->Write();

  TGraph* triggerTimeStampVsTimeStamp1Graph = new TGraph(timeStamp1Vals_.size(),&(*timeStamp1Vals_.begin()),&(*triggerTimeStampVals_.begin()));
  triggerTimeStampVsTimeStamp1Graph->SetName("triggerTimeStampVsTimeStamp1Graph");
  triggerTimeStampVsTimeStamp1Graph->GetXaxis()->SetTitle("timestamp1 [s]");
  triggerTimeStampVsTimeStamp1Graph->GetYaxis()->SetTitle("trigger timestamp [s]");
  triggerTimeStampVsTimeStamp1Graph->SetTitle("");
  triggerTimeStampVsTimeStamp1Graph->Write();

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
