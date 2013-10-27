// system include files
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

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

#include "PhysicsTools/FWLite/interface/CommandLineParser.h"
#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/PythonProcessDesc.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TPad.h"
#include "TROOT.h"

#define MAXCHPEREVENT 500
std::string rootInputFileName_;
std::string rootOutputFileName_;
std::string plotsDirName_;
std::string htmlDirName_;
std::string htmlFileName_;
int newRowEvery_;
int thumbnailSize_;
bool outputRawHistograms_;
bool selectDigiBasedOnTubeName_;
int maxEvents_;
TFile* rootInputFile_;
TFile* rootOutputFile_;
TH2F* firstEventHistMeanMapsHFM[2];
TH2F* firstEventHistMeanMapsHFP[2];
TH2F* firstEventHistRMSMapsHFM[2];
TH2F* firstEventHistRMSMapsHFP[2];
TTree* eventTree_;
std::vector<float> evtNumbers_;
std::vector<float> orbitNumberSecs_;
std::vector<float> indexVals_;
std::vector<float> motorCurrentVals_;
std::vector<float> reelVals_;
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
uint32_t treeDriverStatus_;
uint32_t treeChDenseIndex_[MAXCHPEREVENT];
float treeChHistMean_[MAXCHPEREVENT];
float treeChHistRMS_[MAXCHPEREVENT];
uint16_t treeChHistBinContentCap0_[MAXCHPEREVENT][32];
uint16_t treeChHistBinContentCap1_[MAXCHPEREVENT][32];
uint16_t treeChHistBinContentCap2_[MAXCHPEREVENT][32];
uint16_t treeChHistBinContentCap3_[MAXCHPEREVENT][32];
//
// class declaration
//
struct RawHistoData
{
  RawHistoData() { }
  RawHistoData(std::string setTubeName, HcalDetId setDetId, int maxEvents)
  {
    tubeName = setTubeName;
    detId = setDetId;
    eventNumbers.reserve(maxEvents);
    reelPositions.reserve(maxEvents);
    histoAverages.reserve(maxEvents);
    histoRMSs.reserve(maxEvents);
  }

  HcalDetId detId;
  std::string tubeName;
  std::vector<float> eventNumbers;
  std::vector<float> reelPositions;
  std::vector<float> histoAverages;
  std::vector<float> histoRMSs;
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
  else if(tubeName.find("HFM") != string::npos)
  {
    // for HF, tubes go into one tower (require same eta,phi)
    int tubeEta = atof(tubeName.substr(tubeName.find("ETA")+3,tubeName.find("_PHI")-1).c_str());
    int tubePhi = atof(tubeName.substr(tubeName.find("PHI")+3,tubeName.find("_T")-1).c_str());
    //cout << "tubeEta=" << tubeEta << "tubePhi=" << tubePhi << " chEta: " << ieta << " chPhi: " << iphi << endl;
    ieta = fabs(ieta);
    if(tubeEta==ieta && tubePhi==iphi)
      return true;
  }
  else if(tubeName.find("HFP") != string::npos)
  {
    // for HF, tubes go into one tower (require same eta,phi)
    int tubeEta = atof(tubeName.substr(tubeName.find("ETA")+3,tubeName.find("_PHI")-1).c_str());
    int tubePhi = atof(tubeName.substr(tubeName.find("PHI")+3,tubeName.find("_T")-1).c_str());
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
// member functions
//
void startHtml()
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
    if(selectDigiBasedOnTubeName_)
    {
      htmlFile << "<h2>Histogram Data for all channels matching tubes</h2>\n";
    }
    htmlFile.close();
  }
}
//
void appendHtml(std::string tubeName, std::vector<std::string>& imageNames, std::string htmlFileName)
{
  using namespace std;
  string outputPath = htmlDirName_;
  outputPath+="/";
  outputPath+=htmlFileName;
  ofstream htmlFile(outputPath);
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
      htmlFile << "<td><a href=\"../" << *imageName << "\"><img width=" << thumbnailSize_ << " src=\"../" << *imageName << "\"></a></td>\n";
      ++counter;
    }
    htmlFile << "</tr>\n";
    htmlFile << "</table>\n";
    htmlFile << "<hr>\n";
    htmlFile << "<a href=\"" << rootOutputFileName_ << "\">Download Root file</a>\n";
    htmlFile << "</body>\n";
    htmlFile << "</html>\n";
    htmlFile.close();
  }
}

void appendHtmlMainPage(std::string tubeName, std::vector<std::string>& imageNamesEvt, std::vector<std::string>& imageNamesReel)
{
  using namespace std;
  ofstream htmlFile(htmlFileName_, ios::out | ios::app);
  if(htmlFile.is_open())
  {
    htmlFile << "<h3>Tube name: " << tubeName << "</h3>\n";
    htmlFile << "<table>\n";
    htmlFile << "<tr>\n";
    int counter = 0;
    for(std::vector<std::string>::const_iterator imageName = imageNamesEvt.begin(); imageName != imageNamesEvt.end();
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
    for(std::vector<std::string>::const_iterator imageName = imageNamesReel.begin(); imageName != imageNamesReel.end();
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
  }
}
//
void finishHtml(std::set<std::string> tubeNameSet)
{
  using namespace std;
  ofstream htmlFile(htmlFileName_, ios::out | ios::app);
  if(htmlFile.is_open())
  {
    htmlFile << "<hr>\n";
    htmlFile << "<h2>Histogram Avgs. vs. Event</h2>\n";
    for(set<string>::const_iterator tubeItr = tubeNameSet.begin(); tubeItr != tubeNameSet.end(); ++tubeItr)
    {
      htmlFile << "<a href=\"" << htmlDirName_ << "/" << *tubeItr << "_histAvgsVsEvent.html" << "\">Tube " << *tubeItr << "</a>\n";
      htmlFile << "<br>\n";
    }
    htmlFile << "<hr>\n";
    htmlFile << "<h2>Histogram Avgs. vs. Reel Position</h2>\n";
    for(set<string>::const_iterator tubeItr = tubeNameSet.begin(); tubeItr != tubeNameSet.end(); ++tubeItr)
    {
      htmlFile << "<a href=\"" << htmlDirName_ <<  "/" << *tubeItr << "_histAvgsVsReel.html" << "\">Tube " << *tubeItr << "</a>\n";
      htmlFile << "<br>\n";
    }
    htmlFile << "<hr>\n";
    htmlFile << "<h2>Mean/RMS Maps</h2>\n";
    int mapThumbSize = 500;
    if(firstEventHistMeanMapsHFM[0]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFM[0]->GetName() << ".png"
        << "\"><img width=" << thumbnailSize_ << " src=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFM[0]->GetName()
        << ".png" << "\"></a>\n";
    if(firstEventHistMeanMapsHFM[1]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFM[1]->GetName() << ".png"
        << "\"><img width=" << thumbnailSize_ << " src=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFM[1]->GetName()
        << ".png" << "\"></a>\n";
    htmlFile << "<br>\n";
    if(firstEventHistMeanMapsHFP[0]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFP[0]->GetName() << ".png"
        << "\"><img width=" << thumbnailSize_ << " src=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFP[0]->GetName()
        << ".png" << "\"></a>\n";
    if(firstEventHistMeanMapsHFP[1]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFP[1]->GetName() << ".png"
        << "\"><img width=" << thumbnailSize_ << " src=\"" << plotsDirName_ <<  "/" << firstEventHistMeanMapsHFP[1]->GetName()
        << ".png" << "\"></a>\n";
    htmlFile << "<br>\n";
    if(firstEventHistRMSMapsHFM[0]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFM[0]->GetName() << ".png"
        << "\"><img width=" << mapThumbSize << " src=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFM[0]->GetName()
        << ".png" << "\"></a>\n";
    if(firstEventHistRMSMapsHFM[1]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFM[1]->GetName() << ".png"
        << "\"><img width=" << mapThumbSize << " src=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFM[1]->GetName()
        << ".png" << "\"></a>\n";
    htmlFile << "<br>\n";
    if(firstEventHistRMSMapsHFP[0]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFP[0]->GetName() << ".png"
        << "\"><img width=" << mapThumbSize << " src=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFP[0]->GetName()
        << ".png" << "\"></a>\n";
    if(firstEventHistRMSMapsHFP[1]->GetEntries() > 0)
      htmlFile << "<a href=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFP[1]->GetName() << ".png"
        << "\"><img width=" << mapThumbSize << " src=\"" << plotsDirName_ <<  "/" << firstEventHistRMSMapsHFP[1]->GetName()
        << ".png" << "\"></a>\n";
    htmlFile << "<br>\n";
    htmlFile << "<hr>\n";
    htmlFile << "<a href=\"" << rootOutputFileName_ << "\">Download Root file</a>\n";
    htmlFile << "</body>\n";
    htmlFile << "</html>\n";
    htmlFile.close();
  }
}
//
std::string getBlockEventDirName(int event)
{
  int numEventsPerDir = 1000;
  int blockDirNum = (event-1) / numEventsPerDir;
  //int firstEventNum = blockDirNum*numEventsPerDir + 1;
  int firstEventNum = blockDirNum*numEventsPerDir + 1;
  int lastEventNum = (blockDirNum+1)*numEventsPerDir;
  std::string superDirName = "events";
  superDirName+=intToString(firstEventNum);
  superDirName+="to";
  superDirName+=intToString(lastEventNum);
  return superDirName;
}


// ------------ method called once each job just after ending the event loop  ------------
int main(int argc, char ** argv)
{
  // parse arguments
  if(argc < 2)
  {
    std::cout << "Usage : " << argv[0] << " [parameters.py]" << std::endl;
    return 0;
  }

  // get the python configuration
  const edm::ParameterSet& process = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("process");
  // now get each parameterset
  const edm::ParameterSet& ana = process.getParameter<edm::ParameterSet>("HCALSourceDataMonPlots");

  
  rootInputFileName_ = ana.getParameter<std::string>("RootInputFileName");
  rootOutputFileName_ = ana.getParameter<std::string>("RootOutputFileName");
  plotsDirName_ =ana.getParameter<std::string>("PlotsDirName");
  htmlDirName_ = ana.getParameter<std::string>("HtmlDirName");
  htmlFileName_ = ana.getParameter<std::string>("HtmlFileName");
  newRowEvery_ = ana.getParameter<int>("NewRowEvery");
  thumbnailSize_ = ana.getParameter<int>("ThumbnailSize");
  outputRawHistograms_ = ana.getParameter<bool>("OutputRawHistograms");
  selectDigiBasedOnTubeName_ = ana.getParameter<bool>("SelectDigiBasedOnTubeName");
  maxEvents_ = ana.getParameter<int>("MaxEvents");

  using namespace std;

  const int32_t in_detector_mask=0x00100000;

  // do initialization
  rootInputFile_ = new TFile(rootInputFileName_.c_str());
  eventTree_ = (TTree*) rootInputFile_->Get("eventTree");
  cout << "RootInputFile: " << rootInputFileName_ << endl;
  cout << "eventTree: found entries = " << eventTree_->GetEntries() << endl;
  eventTree_->SetBranchAddress("eventNum",&treeEventNum_);
  eventTree_->SetBranchAddress("orbitNum",&treeOrbitNum_);
  eventTree_->SetBranchAddress("index",&treeIndex_);
  eventTree_->SetBranchAddress("msgCounter",&treeMsgCounter_);
  eventTree_->SetBranchAddress("motorCurrent",&treeMotorCurrent_);
  eventTree_->SetBranchAddress("motorVoltage",&treeMotorVoltage_);
  eventTree_->SetBranchAddress("reelPos",&treeReelPos_);
  eventTree_->SetBranchAddress("timestamp1",&treeTimestamp1_);
  eventTree_->SetBranchAddress("triggerTimestamp",&treeTriggerTimestamp_);
  eventTree_->SetBranchAddress("tubeName",treeTubeName_);
  eventTree_->SetBranchAddress("driverStatus",&treeDriverStatus_);
  eventTree_->SetBranchAddress("nChInEvent",&treeNChInEvent_);
  eventTree_->SetBranchAddress("chDenseIndex",treeChDenseIndex_);
  eventTree_->SetBranchAddress("chHistMean",treeChHistMean_);
  eventTree_->SetBranchAddress("chHistRMS",treeChHistRMS_);
  eventTree_->SetBranchAddress("chHistBinContentCap0",treeChHistBinContentCap0_);
  eventTree_->SetBranchAddress("chHistBinContentCap1",treeChHistBinContentCap1_);
  eventTree_->SetBranchAddress("chHistBinContentCap2",treeChHistBinContentCap2_);
  eventTree_->SetBranchAddress("chHistBinContentCap3",treeChHistBinContentCap3_);

  evtNumbers_.reserve(500000);
  orbitNumberSecs_.reserve(500000);
  indexVals_.reserve(500000);
  motorCurrentVals_.reserve(500000);
  reelVals_.reserve(500000);

  map<pair<string,HcalDetId>, RawHistoData*> rawHistoDataMap;
  set<string> garageHistNameSet;
  set<string> absorberHistNameSet;
  set<string> garageHistUnevenNameSet;
  set<string> absorberHistUnevenNameSet;
  set<string> tubeNameSet;
  //vector<RawHistoData> rawHistoDataVec_;
  TFile* outputRootFile = new TFile(rootOutputFileName_.c_str(),"recreate");
  outputRootFile->cd();
  TH1F* tempHist = new TH1F("tempHist","tempHist",32,0,31);
  firstEventHistMeanMapsHFM[0] = new TH2F("firstEventHistMeanMapHFMDepth1","histMean HFM d1;i#eta;i#phi",13,-41,-28,36,1,73);
  firstEventHistRMSMapsHFM[0] = new TH2F("firstEventHistRMSMapHFMDepth1","histRMS HFM d1;i#eta;i#phi",13,-41,-28,36,1,73);
  firstEventHistMeanMapsHFP[0] = new TH2F("firstEventHistMeanMapHFPDepth1","histMean HFP d1;i#eta;i#phi",13,29,42,36,1,73);
  firstEventHistRMSMapsHFP[0] = new TH2F("firstEventHistRMSMapHFPDepth1","histRMS HFP d1;i#eta;i#phi",13,29,42,36,1,73);
  firstEventHistMeanMapsHFM[1] = new TH2F("firstEventHistMeanMapHFMDepth2","histMean HFM d2;i#eta;i#phi",13,-41,-28,36,1,73);
  firstEventHistRMSMapsHFM[1] = new TH2F("firstEventHistRMSMapHFMDepth2","histRMS HFM d2;i#eta;i#phi",13,-41,-28,36,1,73);
  firstEventHistMeanMapsHFP[1] = new TH2F("firstEventHistMeanMapHFPDepth2","histMean HFP d2;i#eta;i#phi",13,29,42,36,1,73);
  firstEventHistRMSMapsHFP[1] = new TH2F("firstEventHistRMSMapHFPDepth2","histRMS HFP d2;i#eta;i#phi",13,29,42,36,1,73);
  set<uint32_t> denseIndexAlreadyInMeanRMSMaps;

  int NHbins = 32;
  float binsArray[33]  =   {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
                      17.0, 19.0, 21.0, 23.0, 25.0, 27.0, 29.0, 32.0, 35.0, 38.0, 41.0, 45.0, 49.0, 53.0,
                      58.0, 63.0, 64.0};

  // make plots dir
  int status = mkdir(plotsDirName_.c_str(), S_IRWXU);
  if(status != 0 && errno != EEXIST)
  {
    cout << "Creating directory: '" << plotsDirName_ << "' failed; can't continue." << endl;
    return -1;
  }
  // make html dir
  status = mkdir(htmlDirName_.c_str(), S_IRWXU);
  if(status != 0 && errno != EEXIST)
  {
    cout << "Creating directory: '" << htmlDirName_ << "'  failed; can't continue." << endl;
    return -1;
  }


  int emptyChannels = 0;
  int emptyChannelsHFMQ1Q4FEDs = 0;
  cout << "Running over " << maxEvents_ << " max events." << endl;
  // loop over tree
  int nevents = eventTree_->GetEntries();
  cout << "Beginning loop over events --> found " << nevents << " events." << endl;
  for(int evt=0; evt < nevents; ++evt)
  {
    if((evt+1) > maxEvents_)
      break;

    if(evt % 1000 == 0)
      cout << "Loop: event " << evt << endl;
    else if((evt+2) > maxEvents_)
      cout << "Loop (last): event " << evt << endl;
    else if(!selectDigiBasedOnTubeName_)
      if(evt % 100 != 0)
        continue; // only look at each 100th (or last) event to speed things up if not selecting digi via tubeName

    eventTree_->GetEntry(evt);
    if(treeNChInEvent_ > MAXCHPEREVENT)
    {
      cout << "ERROR: nChInEvent " << treeNChInEvent_ << " > MAXCHPEREVENT=" << MAXCHPEREVENT << "; can't read tree." << endl;
      return -2;
    }

    if(evt % 100 == 0) // only do this for each 100th event
    {
      evtNumbers_.push_back(treeEventNum_);
      orbitNumberSecs_.push_back(88.9e-6*treeOrbitNum_);
      indexVals_.push_back(treeIndex_);
      motorCurrentVals_.push_back(treeMotorCurrent_);
      reelVals_.push_back(treeReelPos_);
    }

    string tubeName = string(treeTubeName_);
    tubeNameSet.insert(tubeName);

    outputRootFile->cd();
    if(outputRawHistograms_)
    {
      TDirectory* tubeDir = (TDirectory*) outputRootFile->GetDirectory(tubeName.c_str());
      if(!tubeDir)
        tubeDir = outputRootFile->mkdir(tubeName.c_str());
      string blockDirName = getBlockEventDirName(treeEventNum_);
      string blockDirPath = tubeName;
      blockDirPath+="/";
      blockDirPath+=blockDirName;
      TDirectory* blockEventDir = (TDirectory*) outputRootFile->GetDirectory(blockDirPath.c_str());
      if(!blockEventDir)
        blockEventDir = tubeDir->mkdir(blockDirName.c_str());
      string directoryName = "event";
      directoryName+=intToString(treeEventNum_);
      string dirPath = blockDirPath;
      dirPath+="/";
      dirPath+=directoryName;
      TDirectory* subDir = (TDirectory*) outputRootFile->GetDirectory(dirPath.c_str());
      if(!subDir)
        subDir = blockEventDir->mkdir(directoryName.c_str());
      subDir->cd();
    }

    // loop over channels in event
    for(int nCh = 0; nCh < treeNChInEvent_; ++nCh)
    {
      HcalDetId detId = HcalDetId::detIdFromDenseIndex(treeChDenseIndex_[nCh]);
      //if(find(denseIndexAlreadyInMeanRMSMaps.begin(),denseIndexAlreadyInMeanRMSMaps.end(),treeChDenseIndex_[nCh])
      //    == denseIndexAlreadyInMeanRMSMaps.end())
      //{
      //  if(treeChHistMean_[nCh] == 0)
      //  {
      //    if(detId.zside() < 0) // && (detId.iphi() > 55 || detId.iphi() < 19) )
      //    {
      //      cout << "ERROR: HF- detId: " << detId << " hist mean=0" << endl;
      //      ++emptyChannelsHFMQ1Q4FEDs;
      //    }
      //    ++emptyChannels;
      //  }
      //  else if(detId.subdet() == HcalForward && detId.zside() < 0)
      //  {
      //    firstEventHistMeanMapsHFM[detId.depth()-1]->Fill(detId.ieta(),detId.iphi(),treeChHistMean_[nCh]);
      //    firstEventHistRMSMapsHFM[detId.depth()-1]->Fill(detId.ieta(),detId.iphi(),treeChHistRMS_[nCh]);
      //  }
      //  else if(detId.subdet() == HcalForward && detId.zside() > 0)
      //  {
      //    firstEventHistMeanMapsHFP[detId.depth()-1]->Fill(detId.ieta(),detId.iphi(),treeChHistMean_[nCh]);
      //    firstEventHistRMSMapsHFP[detId.depth()-1]->Fill(detId.ieta(),detId.iphi(),treeChHistRMS_[nCh]);
      //  }
      //  denseIndexAlreadyInMeanRMSMaps.insert(treeChDenseIndex_[nCh]);
      //}

      if(selectDigiBasedOnTubeName_)
        if(!isDigiAssociatedToSourceTube(detId,tubeName))
          continue;

      //cout << "Associated this channel: " << detId << " with tube " << tubeName << endl;
      RawHistoData* thisHistoData = rawHistoDataMap.insert(make_pair(make_pair(tubeName,detId), new RawHistoData(tubeName,detId,50000))).first->second;
      TH1F* thisChannelHist = 0;
      TH1F* thisChannelUnevenBinsHist = 0;
      string histName = "hist_Ieta";
      histName+=intToString(detId.ieta());
      histName+="_Iphi";
      histName+=intToString(detId.iphi());
      histName+="_Depth";
      histName+=intToString(detId.depth());
      histName+="_";
      histName+=tubeName;
      string histNameUneven = histName;
      histNameUneven+="_uneven";
      if(fabs(treeReelPos_) < 5)
      {
        histName+="_sourceInGarage";
        //thisChannelHist = (TH1F*)gDirectory->GetList()->FindObject(histName.c_str());
        //if(!thisChannelHist)
        //{
        //  thisChannelHist = new TH1F(histName.c_str(),histName.c_str(),33,0,32);
        //  thisChannelHist->Sumw2();
        //  garageHistNameSet.insert(histName);
        //}
        histNameUneven+="_sourceInGarage";
        thisChannelUnevenBinsHist = (TH1F*)gDirectory->GetList()->FindObject(histNameUneven.c_str());
        if(!thisChannelUnevenBinsHist)
        {
          string histTitleUneven = histNameUneven;
          histTitleUneven+=";linearized QIE counts";
          thisChannelUnevenBinsHist = new TH1F(histNameUneven.c_str(),histTitleUneven.c_str(),NHbins,binsArray);
          thisChannelUnevenBinsHist->Sumw2();
          garageHistUnevenNameSet.insert(histNameUneven);
        }
      }
      else if(treeDriverStatus_ & in_detector_mask)
      {
        histName+="_sourceInAbsorber";
        //thisChannelHist = (TH1F*)gDirectory->GetList()->FindObject(histName.c_str());
        //if(!thisChannelHist)
        //{
        //  thisChannelHist = new TH1F(histName.c_str(),histName.c_str(),33,0,32);
        //  thisChannelHist->Sumw2();
        //  absorberHistNameSet.insert(histName);
        //}
        histNameUneven+="_sourceInAbsorber";
        thisChannelUnevenBinsHist = (TH1F*)gDirectory->GetList()->FindObject(histNameUneven.c_str());
        if(!thisChannelUnevenBinsHist)
        {
          string histTitleUneven = histNameUneven;
          histTitleUneven+=";linearized QIE counts";
          thisChannelUnevenBinsHist = new TH1F(histNameUneven.c_str(),histTitleUneven.c_str(),NHbins,binsArray);
          thisChannelUnevenBinsHist->Sumw2();
          absorberHistUnevenNameSet.insert(histNameUneven);
        }
      }

      if(evt % 100 == 0 || evt % 101 == 0 || evt % 102 == 0 || evt % 104 == 0) // only do this for each 100th-ish event
      {
        thisHistoData->eventNumbers.push_back(treeEventNum_);
        thisHistoData->reelPositions.push_back(treeReelPos_);
        thisHistoData->histoAverages.push_back(treeChHistMean_[nCh]);
        thisHistoData->histoRMSs.push_back(treeChHistRMS_[nCh]);
      }

      // make hist
      histName = getRawHistName(treeEventNum_,detId.ieta(),detId.iphi(),detId.depth());
      tempHist->Reset();
      tempHist->SetNameTitle(histName.c_str(),histName.c_str());
      for(int ibin=0; ibin<32; ibin++)
      {
        float binValSum = treeChHistBinContentCap0_[nCh][ibin];
        binValSum+=treeChHistBinContentCap1_[nCh][ibin];
        binValSum+=treeChHistBinContentCap2_[nCh][ibin];
        binValSum+=treeChHistBinContentCap3_[nCh][ibin];
        //for(int content = 0; content < binValSum; ++content)
        //{
        //  tempHist->Fill(ibin);
        //  //if(thisChannelHist != 0)
        //  //{
        //  //  thisChannelHist->Fill(ibin);
        //  //}
        //}

        if(ibin < 15) thisChannelUnevenBinsHist->SetBinContent(ibin+1,thisChannelUnevenBinsHist->GetBinContent(ibin+1)+binValSum);
        if(14<ibin && ibin<22) thisChannelUnevenBinsHist->SetBinContent(ibin+1,thisChannelUnevenBinsHist->GetBinContent(ibin+1)+binValSum/2.0);
        if(21<ibin && ibin<26) thisChannelUnevenBinsHist->SetBinContent(ibin+1,thisChannelUnevenBinsHist->GetBinContent(ibin+1)+binValSum/3.0);
        if(25<ibin && ibin<29) thisChannelUnevenBinsHist->SetBinContent(ibin+1,thisChannelUnevenBinsHist->GetBinContent(ibin+1)+binValSum/4.0);
        if(28<ibin) thisChannelUnevenBinsHist->SetBinContent(ibin+1,thisChannelUnevenBinsHist->GetBinContent(ibin+1)+binValSum/5.0);

      }
      
      //if(outputRawHistograms_)
      //{
      //  tempHist->Write();
      //}
    }
  }

  rootInputFile_->Close();
  cout << "Ended loop over events." << endl;
  cout << "Saw " << emptyChannels << " empty channels total and " << emptyChannelsHFMQ1Q4FEDs << " empty channels in HF- Q1/Q4 FEDs." << endl;

  // write maps
  outputRootFile->cd();
  firstEventHistMeanMapsHFM[0]->SetMaximum(5);
  firstEventHistRMSMapsHFM[0]->SetMaximum(5);
  firstEventHistMeanMapsHFM[1]->SetMaximum(5);
  firstEventHistRMSMapsHFM[1]->SetMaximum(5);
  firstEventHistMeanMapsHFP[0]->SetMaximum(5);
  firstEventHistRMSMapsHFP[0]->SetMaximum(5);
  firstEventHistMeanMapsHFP[1]->SetMaximum(5);
  firstEventHistRMSMapsHFP[1]->SetMaximum(5);
  
  firstEventHistMeanMapsHFM[0]->Write();
  firstEventHistRMSMapsHFM[0]->Write();
  firstEventHistMeanMapsHFM[1]->Write();
  firstEventHistRMSMapsHFM[1]->Write();
  firstEventHistMeanMapsHFP[0]->Write();
  firstEventHistRMSMapsHFP[0]->Write();
  firstEventHistMeanMapsHFP[1]->Write();
  firstEventHistRMSMapsHFP[1]->Write();

  // make images
  TCanvas* canvasMap = new TCanvas("canvasMap","canvasMap",900,600);
  gStyle->SetOptStat(11);
  firstEventHistMeanMapsHFM[0]->Draw("colz"); gPad->Update();
  TPaveStats *st = (TPaveStats*)firstEventHistMeanMapsHFM[0]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistMeanMapsHFM[0]->Draw("colz");
  string fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistMeanMapsHFM[0]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistMeanMapsHFM[1]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistMeanMapsHFM[1]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistMeanMapsHFM[1]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistMeanMapsHFM[1]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistMeanMapsHFP[0]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistMeanMapsHFP[0]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistMeanMapsHFP[0]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistMeanMapsHFP[0]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistMeanMapsHFP[1]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistMeanMapsHFP[1]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistMeanMapsHFP[1]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistMeanMapsHFP[1]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  //
  firstEventHistRMSMapsHFM[0]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistRMSMapsHFM[0]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistRMSMapsHFM[0]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistRMSMapsHFM[0]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistRMSMapsHFM[1]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistRMSMapsHFM[1]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistRMSMapsHFM[1]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistRMSMapsHFM[1]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistRMSMapsHFP[0]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistRMSMapsHFP[0]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistRMSMapsHFP[0]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistRMSMapsHFP[0]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());
  firstEventHistRMSMapsHFP[1]->Draw("colz"); gPad->Update();
  st = (TPaveStats*)firstEventHistRMSMapsHFP[1]->GetListOfFunctions()->FindObject("stats");
  st->SetX1NDC(0.8);st->SetX2NDC(0.995);st->SetY1NDC(0.93);st->SetY2NDC(0.995);firstEventHistRMSMapsHFP[1]->Draw("colz");
  fullMapPath = plotsDirName_;
  fullMapPath+="/";
  fullMapPath+=firstEventHistRMSMapsHFP[1]->GetName();
  fullMapPath+=".png";
  canvasMap->Print(fullMapPath.c_str());

  gStyle->SetOptStat(2222211);

  // do quality checks
  for(set<string>::const_iterator itr = garageHistUnevenNameSet.begin(); itr != garageHistUnevenNameSet.end(); ++itr)
  {
    TH1F* sourceGarageHist = (TH1F*)gDirectory->GetList()->FindObject(itr->c_str());
    string abHistName = *itr;
    abHistName = abHistName.substr(0,abHistName.find("Garage"));
    cout << "abHistName=" << abHistName << endl;
    abHistName+="Absorber";
    cout << "abHistName2=" << abHistName << endl;
    TH1F* sourceAbsorberHist = (TH1F*)gDirectory->GetList()->FindObject(abHistName.c_str());
    if(!sourceAbsorberHist)
    {
      cout << "ERROR: could not find source absorber hist:" << abHistName << endl;
      continue;
    }
    sourceGarageHist->Scale(sourceAbsorberHist->Integral(0,10)/sourceGarageHist->Integral(0,10));
    // look for excess
    if(sourceAbsorberHist->Integral(11,31)/sourceGarageHist->Integral(11,31) < 5)
      cout << "ERROR: ratio of events in tail (bins 11-31) (sourceAbsorber/sourceGarage) < 5 for this hist: " << sourceAbsorberHist << endl;

    TCanvas* t = new TCanvas("canvas","canvas",900,600);
    t->cd();
    t->SetLogy();
    sourceGarageHist->SetStats(0);
    sourceGarageHist->Draw();
    sourceAbsorberHist->SetStats(0);
    sourceAbsorberHist->SetLineColor(2);
    sourceAbsorberHist->Draw("same");
    string imgName = *itr;
    imgName = imgName.substr(0,imgName.find("Garage"));
    imgName+="AbsorberAndGarageOverlay";
    string fullPath = plotsDirName_;
    fullPath+="/";
    fullPath+=imgName;
    fullPath+=".png";
    cout << "fullPath=" << fullPath << endl;
    t->Print(fullPath.c_str());
    delete t;
  }
  // write per-channel source in/out hists; 
  for(set<string>::const_iterator itr = garageHistNameSet.begin(); itr != garageHistNameSet.end(); ++itr)
    gDirectory->GetList()->FindObject(itr->c_str())->Write();
  for(set<string>::const_iterator itr = absorberHistNameSet.begin(); itr != absorberHistNameSet.end(); ++itr)
    gDirectory->GetList()->FindObject(itr->c_str())->Write();
  // uneven
  for(set<string>::const_iterator itr = garageHistUnevenNameSet.begin(); itr != garageHistUnevenNameSet.end(); ++itr)
    gDirectory->GetList()->FindObject(itr->c_str())->Write();
  for(set<string>::const_iterator itr = absorberHistUnevenNameSet.begin(); itr != absorberHistUnevenNameSet.end(); ++itr)
    gDirectory->GetList()->FindObject(itr->c_str())->Write();


  // now make plots of avgVal vs. event number
  vector<string> imageNamesThisTube; 
  vector<string> reelImageNamesThisTube;

  startHtml();

  cout << "Beginning loop over tubes." << endl;
  outputRootFile->cd();
  for(set<string>::const_iterator tubeItr = tubeNameSet.begin(); tubeItr != tubeNameSet.end(); ++tubeItr)
  {
    string thisTube = *tubeItr;
    cout << "Found tube: " << thisTube << endl;
    imageNamesThisTube.clear();
    reelImageNamesThisTube.clear();
    for(map<pair<string,HcalDetId>,RawHistoData*>::const_iterator itr = rawHistoDataMap.begin();
        itr != rawHistoDataMap.end(); ++itr)
    {
      RawHistoData data = *(itr->second);
      if(data.tubeName != thisTube) // only consider current tube
        continue;

      // compute avg y value for plot scaling
      float yavg = 0;
      int count = 0;
      for(std::vector<float>::const_iterator i = data.histoAverages.begin(); i != data.histoAverages.end(); ++i)
      {
        yavg+=*i;
        count++;
      }
      yavg/=count;
      //// make eventNum errs
      //vector<float> eventNumErrs;
      //for(std::vector<float>::const_iterator i = data.eventNumbers.begin(); i != data.eventNumbers.end(); ++i)
      //  eventNumErrs.push_back(0);
      //TGraphErrors* thisGraph = new TGraphErrors(data.eventNumbers.size(),&(*data.eventNumbers.begin()),
      //    &(*data.histoAverages.begin()),&(*eventNumErrs.begin()),&(*data.histoRMSs.begin()));
      TGraph* thisGraph = new TGraph(data.eventNumbers.size(),&(*data.eventNumbers.begin()),&(*data.histoAverages.begin()));
      string graphName = getGraphName(data.detId,data.tubeName);
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
      std::string fullPath = plotsDirName_;
      fullPath+="/";
      fullPath+=graphName;
      fullPath+=".png";
      canvas->Print(fullPath.c_str());
      imageNamesThisTube.push_back(fullPath);
      delete thisGraph;
      TGraph* reelGraph = new TGraph(data.reelPositions.size(),&(*data.reelPositions.begin()),&(*data.histoAverages.begin()));
      string reelGraphName = getGraphName(data.detId,data.tubeName);
      reelGraphName+="reelPosition";
      reelGraph->SetTitle(reelGraphName.c_str());
      reelGraph->SetName(reelGraphName.c_str());
      canvas->cd();
      reelGraph->Draw();
      reelGraph->GetXaxis()->SetTitle("Reel [mm]");
      reelGraph->GetYaxis()->SetTitle("hist. mean [ADC]");
      reelGraph->GetYaxis()->SetRangeUser(yavg-0.4,yavg+0.4);
      reelGraph->SetMarkerStyle(33);
      reelGraph->SetMarkerSize(0.8);
      reelGraph->Draw("ap");
      reelGraph->Write();
      fullPath = plotsDirName_;
      fullPath+="/";
      fullPath+=reelGraphName;
      fullPath+=".png";
      canvas->Print(fullPath.c_str());
      reelImageNamesThisTube.push_back(fullPath);
      delete reelGraph;
      delete canvas;
    }
    appendHtml(thisTube,imageNamesThisTube,thisTube+"_histAvgsVsEvent.html");
    appendHtml(thisTube,reelImageNamesThisTube,thisTube+"_histAvgsVsReel.html");
    if(selectDigiBasedOnTubeName_)
      appendHtmlMainPage(thisTube,imageNamesThisTube,reelImageNamesThisTube);
  }
  cout << "Ending loop over tubes." << endl;

  finishHtml(tubeNameSet);

  cout << "Making driver info plots." << endl;
  // make driver info graphs
  TDirectory* dInfoPlotsDir = outputRootFile->mkdir("driverInfoPlots");
  dInfoPlotsDir->cd();
  //TGraph* eventNumVsOrbitNumGraph = new TGraph(evtNumbers_.size(),&(*orbitNumbers_.begin()),&(*evtNumbers_.begin()));
  //eventNumVsOrbitNumGraph->Draw();
  //eventNumVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  //eventNumVsOrbitNumGraph->GetYaxis()->SetTitle("event");
  //eventNumVsOrbitNumGraph->SetName("naiveEventNumVsOrbitNumGraph");
  //eventNumVsOrbitNumGraph->Write();

  TGraph* eventNumVsOrbitNumSecsGraph = new TGraph(evtNumbers_.size(),&(*orbitNumberSecs_.begin()),&(*evtNumbers_.begin()));
  eventNumVsOrbitNumSecsGraph->Draw();
  eventNumVsOrbitNumSecsGraph->GetXaxis()->SetTitle("orbit [s]");
  eventNumVsOrbitNumSecsGraph->GetYaxis()->SetTitle("event");
  eventNumVsOrbitNumSecsGraph->SetName("naiveEventNumVsOrbitNumSecsGraph");
  eventNumVsOrbitNumSecsGraph->Write();

  //TGraph* messageCounterVsOrbitNumGraph = new TGraph(messageCounterVals_.size(),&(*orbitNumberSecs_.begin()),&(*messageCounterVals_.begin()));
  //messageCounterVsOrbitNumGraph->SetName("messageCounterVsOrbitNumGraph");
  //messageCounterVsOrbitNumGraph->Draw();
  //messageCounterVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  //messageCounterVsOrbitNumGraph->GetYaxis()->SetTitle("message");
  //messageCounterVsOrbitNumGraph->SetTitle("");
  //messageCounterVsOrbitNumGraph->Write();

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

  //TGraph* motorVoltageVsOrbitNumGraph = new TGraph(motorVoltageVals_.size(),&(*orbitNumberSecs_.begin()),&(*motorVoltageVals_.begin()));
  //motorVoltageVsOrbitNumGraph->SetName("motorVoltageVsOrbitNumGraph");
  //motorVoltageVsOrbitNumGraph->Draw();
  //motorVoltageVsOrbitNumGraph->GetXaxis()->SetTitle("orbit");
  //motorVoltageVsOrbitNumGraph->GetYaxis()->SetTitle("motor voltage [V]");
  //motorVoltageVsOrbitNumGraph->SetTitle("");
  //motorVoltageVsOrbitNumGraph->Write();

  TGraph* motorCurrentVsReelPosGraph = new TGraph(motorCurrentVals_.size(),&(*reelVals_.begin()),&(*motorCurrentVals_.begin()));
  motorCurrentVsReelPosGraph->SetName("motorCurrentVsReelPosGraph");
  motorCurrentVsReelPosGraph->Draw();
  motorCurrentVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  motorCurrentVsReelPosGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsReelPosGraph->SetTitle("");
  motorCurrentVsReelPosGraph->Write();

  //TGraph* motorVoltageVsReelPosGraph = new TGraph(motorVoltageVals_.size(),&(*reelVals_.begin()),&(*motorVoltageVals_.begin()));
  //motorVoltageVsReelPosGraph->SetName("motorVoltageVsReelPosGraph");
  //motorVoltageVsReelPosGraph->Draw();
  //motorVoltageVsReelPosGraph->GetXaxis()->SetTitle("reel [mm]");
  //motorVoltageVsReelPosGraph->GetYaxis()->SetTitle("motor voltage [V]");
  //motorVoltageVsReelPosGraph->SetTitle("");
  //motorVoltageVsReelPosGraph->Write();

  TGraph* reelVsOrbitNumGraph = new TGraph(reelVals_.size(),&(*orbitNumberSecs_.begin()),&(*reelVals_.begin()));
  reelVsOrbitNumGraph->SetName("reelVsOrbitNumGraph");
  reelVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  reelVsOrbitNumGraph->GetYaxis()->SetTitle("reel [mm]");
  reelVsOrbitNumGraph->SetTitle("");
  reelVsOrbitNumGraph->Write();

  //TGraph* triggerTimestampVsOrbitNumGraph = new TGraph(triggerTimeStampVals_.size(),&(*orbitNumberSecs_.begin()),&(*triggerTimeStampVals_.begin()));
  //triggerTimestampVsOrbitNumGraph->SetName("triggerTimestampVsOrbitNumGraph");
  //triggerTimestampVsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  //triggerTimestampVsOrbitNumGraph->GetYaxis()->SetTitle("trigger timestamp [s]");
  //triggerTimestampVsOrbitNumGraph->SetTitle("");
  //triggerTimestampVsOrbitNumGraph->Write();

  //TGraph* timeStamp1VsOrbitNumGraph = new TGraph(timeStamp1Vals_.size(),&(*orbitNumberSecs_.begin()),&(*timeStamp1Vals_.begin()));
  //timeStamp1VsOrbitNumGraph->SetName("timeStamp1VsOrbitNumGraph");
  //timeStamp1VsOrbitNumGraph->GetXaxis()->SetTitle("orbit [s]");
  //timeStamp1VsOrbitNumGraph->GetYaxis()->SetTitle("timestamp1 [s]");
  //timeStamp1VsOrbitNumGraph->SetTitle("");
  //timeStamp1VsOrbitNumGraph->Write();

  //TGraph* triggerTimeStampVsTimeStamp1Graph = new TGraph(timeStamp1Vals_.size(),&(*timeStamp1Vals_.begin()),&(*triggerTimeStampVals_.begin()));
  //triggerTimeStampVsTimeStamp1Graph->SetName("triggerTimeStampVsTimeStamp1Graph");
  //triggerTimeStampVsTimeStamp1Graph->GetXaxis()->SetTitle("timestamp1 [s]");
  //triggerTimeStampVsTimeStamp1Graph->GetYaxis()->SetTitle("trigger timestamp [s]");
  //triggerTimeStampVsTimeStamp1Graph->SetTitle("");
  //triggerTimeStampVsTimeStamp1Graph->Write();

  // vs event number
  //TGraph* messageCounterVsEventNumGraph = new TGraph(messageCounterVals_.size(),&(*evtNumbers_.begin()),&(*messageCounterVals_.begin()));
  //messageCounterVsEventNumGraph->SetName("messageCounterVsEventNumGraph");
  //messageCounterVsEventNumGraph->Draw();
  //messageCounterVsEventNumGraph->GetXaxis()->SetTitle("event");
  //messageCounterVsEventNumGraph->GetYaxis()->SetTitle("message");
  //messageCounterVsEventNumGraph->SetTitle("");
  //messageCounterVsEventNumGraph->Write();

  TGraph* indexVsEventNumGraph = new TGraph(indexVals_.size(),&(*evtNumbers_.begin()),&(*indexVals_.begin()));
  indexVsEventNumGraph->SetName("indexVsEventNumGraph");
  indexVsEventNumGraph->GetXaxis()->SetTitle("event");
  indexVsEventNumGraph->GetYaxis()->SetTitle("index");
  indexVsEventNumGraph->SetTitle("");
  indexVsEventNumGraph->Write();

  TGraph* motorCurrentVsEventNumGraph = new TGraph(motorCurrentVals_.size(),&(*evtNumbers_.begin()),&(*motorCurrentVals_.begin()));
  motorCurrentVsEventNumGraph->SetName("motorCurrentVsEventNumGraph");
  motorCurrentVsEventNumGraph->GetXaxis()->SetTitle("event");
  motorCurrentVsEventNumGraph->GetYaxis()->SetTitle("motor current [mA]");
  motorCurrentVsEventNumGraph->SetTitle("");
  motorCurrentVsEventNumGraph->Write();

  //TGraph* motorVoltageVsEventNumGraph = new TGraph(motorVoltageVals_.size(),&(*evtNumbers_.begin()),&(*motorVoltageVals_.begin()));
  //motorVoltageVsEventNumGraph->SetName("motorVoltageVsEventNumGraph");
  //motorVoltageVsEventNumGraph->Draw();
  //motorVoltageVsEventNumGraph->GetXaxis()->SetTitle("orbit");
  //motorVoltageVsEventNumGraph->GetYaxis()->SetTitle("motor voltage [V]");
  //motorVoltageVsEventNumGraph->SetTitle("");
  //motorVoltageVsEventNumGraph->Write();

  TGraph* reelVsEventNumGraph = new TGraph(reelVals_.size(),&(*evtNumbers_.begin()),&(*reelVals_.begin()));
  reelVsEventNumGraph->SetName("reelVsEventNumGraph");
  reelVsEventNumGraph->GetXaxis()->SetTitle("event");
  reelVsEventNumGraph->GetYaxis()->SetTitle("reel [mm]");
  reelVsEventNumGraph->SetTitle("");
  reelVsEventNumGraph->Write();

  //TGraph* triggerTimestampVsEventNumGraph = new TGraph(triggerTimeStampVals_.size(),&(*evtNumbers_.begin()),&(*triggerTimeStampVals_.begin()));
  //triggerTimestampVsEventNumGraph->SetName("triggerTimestampVsEventNumGraph");
  //triggerTimestampVsEventNumGraph->GetXaxis()->SetTitle("event");
  //triggerTimestampVsEventNumGraph->GetYaxis()->SetTitle("trigger timestamp [s]");
  //triggerTimestampVsEventNumGraph->SetTitle("");
  //triggerTimestampVsEventNumGraph->Write();

  //TGraph* timeStamp1VsEventNumGraph = new TGraph(timeStamp1Vals_.size(),&(*evtNumbers_.begin()),&(*timeStamp1Vals_.begin()));
  //timeStamp1VsEventNumGraph->SetName("timeStamp1VsEventNumGraph");
  //timeStamp1VsEventNumGraph->GetXaxis()->SetTitle("event");
  //timeStamp1VsEventNumGraph->GetYaxis()->SetTitle("timestamp1 [s]");
  //timeStamp1VsEventNumGraph->SetTitle("");
  //timeStamp1VsEventNumGraph->Write();
  cout << "Done making driver info plots." << endl;
  gROOT->GetListOfFiles()->Remove(outputRootFile);

  outputRootFile->Close();
  cout << "Closed root file." << endl;
}

