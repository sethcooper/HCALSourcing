// -*- C++ -*-
//
// Package:    RawHistogramDataDumper
// Class:      RawHistogramDataDumper
// 
/**\class RawHistogramDataDumper RawHistogramDataDumper.cc subsystem/RawHistogramDataDumper/src/RawHistogramDataDumper.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Drew Baden
//         Created:  Fri Nov 23 07:42:39 EST 2012
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <DataFormats/FEDRawData/interface/FEDRawDataCollection.h>
#include <DataFormats/FEDRawData/interface/FEDHeader.h>
#include <DataFormats/FEDRawData/interface/FEDTrailer.h>
#include <DataFormats/FEDRawData/interface/FEDNumbering.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <vector>
#include "TH1.h"
#include "TH2.h"
#include <TRandom.h>
#include <TProfile.h>

using namespace edm;
using namespace std;
int prtlev = 2;
int this_evn = 0;
int this_run = 0;
int nevents = 0;
int nFEDs = 0;
int hspigot[18];
int wspigot[18];
int ospigot[18];
int evn, evn1, w3top, dltr, od, w3bot, htrn, orn, fwv, bcn, hf1, hf2, dll, ttcr, fpgaV, fpgaT, fiberror;
uint32_t* hdata;  //here is the 16-bit HTR payload data
uint32_t n16;     //number of 16-bit words for this HTR payload
//
// here are the FEDs.  you can loop over these to decode the various DCC payloads
//
#define NFED_HBHE 18
int HBHE_feds[18] = {700,701,702,703,704,705,706,707,708,709,710,711,712,713,714,715,716,717};
#define NFED_HF 6
int HF_feds[6] = {718,719,720,721,722,723};

void payload_return(uint32_t s,
 	int* nwords,int* htrerr,int* lrberr,int* ee,int* ep,int* eb,int* ev,int* et);
void print_htr_payload_headers(bool header, int spigot, bool extra);
void htr_data_from_payload(const uint32_t *payload, int spigot);
void htr_data_delete();
void htr_fill_header();
void print_hist(char* cout, unsigned int data);

//
// get a handle on the raw data (pun intended)
//
Handle<FEDRawDataCollection> rawdata;

//
// class declaration
//

class RawHistogramDataDumper : public edm::EDAnalyzer {
   public:
      explicit RawHistogramDataDumper(const edm::ParameterSet&);
      ~RawHistogramDataDumper();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      std::set<int> FEDids_;
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      // ----------member data ---------------------------
};
void payload_return(uint32_t s,
 	int* nwords,int* htrerr,int* lrberr,int* ee,int* ep,int* eb,int* ev,int* et) {
	*nwords = (s & 0x3FF);
	*htrerr = (s>>24) & 0xFF;
	*lrberr = (s>>16) & 0xFF;
	*ee = (s>>15) & 0x1;
	*ep = (s>>14) & 0x1;
	*eb = (s>>13) & 0x1;
	*ev = (s>>12) & 0x1;
	*et = (s>>11) & 0x1;
}
void htr_data_from_payload(const uint32_t *payload, int spigot) {
	//
	// sheesh, now go to 16 bit words. so we can follow the documentation
	//
	int iptr = ospigot[spigot];
	int nwords = wspigot[spigot];
	n16 = 2*nwords;
	hdata = new uint32_t [n16];
	for (int j=0; j<nwords; j++) {
	    hdata[2*j] = payload[iptr+j] & 0xFFFF;
//		    printf("%d %d 0x%x\n",j,2*j,hdata[2*j]);
	    hdata[2*j+1] = (payload[iptr+j] >> 16) & 0xFFFF;
//		    printf("%d %d 0x%x\n",j,2*j+1,hdata[2*j+1]);
	}
}

void htr_data_delete() {
	delete [] hdata;
}

void htr_fill_header() {
//  for (int i=0; i<8; i++) printf("header %d = 0x%4.4X\n",i+1,hdata[i]);
  evn1 = (hdata[0] & 0xFF);

  evn = evn1 + (hdata[1] * 256);

  w3top = (hdata[2] >> 9) & 0xF;
  dltr = (hdata[2] >> 8) & 0x1;
  od = (hdata[2] >> 7) & 1;
  w3bot = hdata[2] & 0x7F;

  htrn = hdata[3] & 0x3FF;
  orn = (hdata[3] >> 10 ) & 0x3F;

  fwv = (hdata[4] >> 12) & 0xF;
  bcn = hdata[4] & 0xFFF;
//  1111 11
//  5432 1098 7654 3210
//  0fff 0fff 0000 00DT
  hf1 = (hdata[5] >> 8) & 0x7;
  hf2 = (hdata[5] >> 12) & 0x7;
  dll = (hdata[5] >> 1) & 1;
  ttcr = hdata[5] & 1;
  
  fpgaV = hdata[6] & 0x3FFF; 
  fpgaT = (hdata[6] >> 13) & 0x7;
  
  fiberror = hdata[7] & 0xFF;

}
void print_htr_payload_headers(bool header, int spigot, bool extra) {
//	if (header) {
          printf(" Spigot    EvN    BcN   HTR   fwV fpgaT    fpgaV  f1  f2  DLL TTCr  DLTR  OD  fibErr\n");
//        }
	htr_fill_header();
	printf("   %2d    %4d   %5d  %4d  0x%2.2X 0x%2.2X     0x%2.2x  %1d   %1d    %1d   %1d    %1d     %1d  0x%2.2X\n",
	    spigot,evn,bcn,htrn,fwv,fpgaT,fpgaV,hf1,hf2,dll,ttcr,dltr,od,fiberror);
}
void print_hist(char* cout, unsigned int data) {

        //    012345678901234
	if (data > 39999)      sprintf(cout,"|         %5d",data);
	else if (data > 19999) sprintf(cout,"|        %5d ",data);
	else if (data >  9999) sprintf(cout,"|       %5d  ",data);
	else if (data >  4999) sprintf(cout,"|      %4d    ",data);
	else if (data >   999) sprintf(cout,"|     %4d     ",data);
	else if (data >   499) sprintf(cout,"|    %3d       ",data);
	else if (data >    99) sprintf(cout,"|   %3d        ",data);
	else if (data >    49) sprintf(cout,"|  %2d          ",data);
	else if (data >     9) sprintf(cout,"| %2d           ",data);
	else                   sprintf(cout,"|%1d             ",data);
}

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
RawHistogramDataDumper::RawHistogramDataDumper(const edm::ParameterSet& iConfig)

{
   //now do what ever initialization is needed

}


RawHistogramDataDumper::~RawHistogramDataDumper()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void RawHistogramDataDumper::analyze(const edm::Event& e, const edm::EventSetup& iSetup) {
  //  using namespace edm;

  //cout << "Type anything to continue: ";
  //int temp;
  //scanf("%d",&temp);
  nevents++;
  this_evn = e.id().event();
  this_run = e.id().run();
  cout << "---> Run " << this_run << " Event " << this_evn << endl;
  e.getByType(rawdata);
  //
  // loop over all feds, see if they are there
  //
  nFEDs = 0;
  for (int i = 700; i<732; i++) {
    const FEDRawData& data = rawdata->FEDData(i);
    size_t size=data.size();
    if (size>0 && (FEDids_.empty() || FEDids_.find(i)!=FEDids_.end())) {
      cout << " Found FED# " << setw(4) << i << " " << setw(8) << size << " bytes " << endl;
      FEDHeader header(data.data());
      FEDTrailer trailer(data.data()+size-8);
      //	    cout << " L1Id: " << setw(8) << header.lvl1ID() << " BXId: " << setw(4) << header.bxID() << endl;
      const uint32_t* payload = (uint32_t*) (data.data());
      //
      // very first 4 words...
      //
      int fedn = (payload[0] >> 8) & 0xFFF;  // 12 bits
      int bcn =  (payload[0] >> 20) & 0xFFF;  // 12 bits
      int evn =  payload[1] & 0xFFFFFF; // 24 bits
      cout << " Fed " << fedn << "  BcN " << bcn << "  EvN " << evn;
      int orn = (payload[2] >> 4) & 0xFFFFFFFF;
      cout << "  OrN " << orn << endl;
      //
      // next comes the dcc header, which we don't really care about
      //
      cout << " DCC stuff: " << hex << payload[4] << " " << payload[5] << dec << endl;
      //
      // next comes the "spigot"s, 32 bits each
      //
      int spigots = 0;
      int nwords,htrerr,lrberr,ee,ep,eb,ev,et;
      cout << "  Spigot  #Words  HTRerr  LRBerr  E-P-B-V-T " << endl;
      for (int i=0; i<18; i++) {
        uint32_t s = payload[i+6] & 0xFFFFFFFF;
        payload_return(s,&nwords,&htrerr,&lrberr,&ee,&ep,&eb,&ev,&et);
        if (nwords > 0) {
          hspigot[spigots] = s;
          wspigot[spigots] = nwords;
          printf("  %4d    %5d    0x%2.2x    0x%2.2x   %d-%d-%d-%d-%d\n",
              spigots++,nwords,htrerr,lrberr,ee,ep,eb,ev,et);
        }
      }
      cout << " There are " << spigots << " spigots" << endl;
      ospigot[0] = 24;
      for (int i=1; i<spigots; i++) ospigot[i] = ospigot[i-1] + wspigot[i-1];
      //
      // formatted dump of the headers for all HTRs (spigots)
      // 
      for (int i=0; i<spigots; i++) {
        //
        // extract the header for this spigot
        //
        htr_data_from_payload(payload,i);
        //
        // print it out
        //
        print_htr_payload_headers(i==0,i,false);
        //
        // loop over the 2 fibers and store into a humongous array:
        //	icap_data[ifiber=0,1][iqie=0,1,2][icap=0,1,2,3][bins]
        //  also store the sums in a similar array
        //
        int ifiber;
        int ic = 8;
        unsigned int icap_data[2][3][4][32];
        int sums[2][3][4];
        uint32_t dataH, fdata;
        for (int ifib=0; ifib<2; ifib++) {
          //
          // loop over the 3 QIE's
          //
          for (int iqie=0; iqie<3; iqie++) {
            //
            // collect the data from the 4 capid's for printout
            //
            for (int icap=0; icap<4; icap++) {
              sums[ifib][iqie][icap] = 0;
              //	cout << endl;
              for (int ib=0; ib<32; ib++) {
                fdata = hdata[ic++];
                dataH = fdata & 0xFFFF;
                icap_data[ifib][iqie][icap][ib] = dataH;
                //	  				  printf("cap %d bin %d icnt %d data %d\n",icap,ib,icnt,dataH);
                sums[ifib][iqie][icap] = sums[ifib][iqie][icap] + dataH;
              }
            }
          }
        }
        //
        // each bin is a row and do all the capid's
        //
        if (prtlev > 1) {
          char c0[25],c1[25],c2[25],c3[25];
          for (int ifib=0; ifib<2; ifib++) {
            //ifiber = (ifib == 0) ? hf1 : hf2;
            ifiber = (ifib == 0) ? hf1+1 : hf2+1;
            for (int iqie=0; iqie<3; iqie++) {
              printf("------------------------------------------------------------------\n");
              //printf("       Fiber %d (of 0-7 range) QIE %d\n",ifiber,iqie);
              printf("       Fiber %d (1-8) QIE %d\n",ifiber,iqie);
              printf(" Bin | ----CAP0---- | ----CAP1---- | ----CAP2---- | ----CAP3---- |\n");
              for (int ib=0; ib<32; ib++) {
                unsigned int data0 = icap_data[ifib][iqie][0][ib];
                unsigned int data1 = icap_data[ifib][iqie][1][ib];
                unsigned int data2 = icap_data[ifib][iqie][2][ib];
                unsigned int data3 = icap_data[ifib][iqie][3][ib];
                sprintf(c0,"|%6d  %4x",data0,data0);
                sprintf(c1,"|%6d  %4x",data1,data1);
                sprintf(c2,"|%6d  %4x",data2,data2);
                sprintf(c3,"|%6d  %4x",data3,data3);
                print_hist(c0,data0);
                print_hist(c1,data1);
                print_hist(c2,data2);
                print_hist(c3,data3);
                //	                cout << ib << "   " << c0 << c1 << c2 << c3 << endl;
                printf("%2d   %s%s%s%s|\n",ib,c0,c1,c2,c3);
              }
              //
              // printout the sums
              //
              printf("Sums:|    %6d    |    %6d    |    %6d    |    %6d    |\n",
                  sums[ifib][iqie][0],sums[ifib][iqie][1],sums[ifib][iqie][2],sums[ifib][iqie][3]);
            }
          }
        }
        //
        // now delete the spigot
        //
        htr_data_delete();
      }
    }
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
RawHistogramDataDumper::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
RawHistogramDataDumper::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
void 
RawHistogramDataDumper::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
RawHistogramDataDumper::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
RawHistogramDataDumper::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
RawHistogramDataDumper::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
RawHistogramDataDumper::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(RawHistogramDataDumper);
