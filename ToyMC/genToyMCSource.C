#include <sstream>
#include "TF1.h"
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"

std::string intToString(int num)
{
    using namespace std;
    ostringstream myStream;
    myStream << num << flush;
    return (myStream.str ());
}

// make gaus+pois function
Double_t myPedPlusSourceFunc(Double_t *x, Double_t *par)
{
  double x1 = x[0];
  if(x1 < 4)
    return par[0]*TMath::Gaus(x1,par[1],par[2]);
  else
    return par[0]*TMath::Gaus(x1,par[1],par[2]) + par[3]*TMath::Poisson(x1-4,par[4]);
  //  return par[0]*(TMath::Gaus(x1,par[1],par[2]) + TMath::Poisson(x1-1,par[3]));
    //return par[0]*TMath::Gaus(x1,par[1],par[2]) + par[0]*TMath::Exp(par[3])*TMath::Poisson(x1-par[1],par[3]);
}


void genToyMCSource()
{
  using namespace std;
  TFile* rootFile = new TFile("toyMCSource.root","recreate");
  rootFile->cd();

  int ieta=10;
  int iphi=12;
  int depth=1;

  TF1* myPed = new TF1("myPed","gaus(0)",0,32);
  myPed->SetParameters(131000,5.42,0.8);
  myPed->FixParameter(1,5.42);
  myPed->FixParameter(2,0.8);
  //
  TF1* myPedPlusSource = new TF1("myPedPlusSource",myPedPlusSourceFunc,0,32,5);
  myPedPlusSource->SetParameters(13000,5.42,0.8,1000,0.3);
  myPedPlusSource->SetParNames("GNorm","GMean","GSigma","PNorm","PMean");
  //TF1* myPedPlusSource = new TF1("myPedPlusSource",myPedPlusSourceFunc,0,32,4);
  //myPedPlusSource->SetParameters(131000,5.42,0.8,0.3);
  //myPedPlusSource->SetParNames("Norm","GMean","GSigma","PMean");
  myPedPlusSource->FixParameter(1,5.42);
  myPedPlusSource->FixParameter(2,0.8);
  //
  TF1* mySource = new TF1("mySource","[0]*TMath::Poisson(x,[1])",0,32);
  mySource->SetParameters(1000,0.3);

  TH1F* histSave;
  for(int event = 1; event <= 1000; ++event)
  {
    string histName = "rawHistEvent";
    histName+=intToString(event);
    histName+="_Ieta";
    histName+=intToString(ieta);
    histName+="_Iphi";
    histName+=intToString(iphi);
    histName+="_Depth";
    histName+=intToString(depth);

    TH1F* hist = new TH1F(histName.c_str(),histName.c_str(),32,0,32);
    if(event <= 100) // pedestal
    {
      hist->FillRandom("myPed",65535);
    }
    else // pedestal+source
    {
      //hist->FillRandom("myPedPlusSource",65535);
      for(int i=0; i < 65535; ++i)
      {
        double myEvent = myPed->GetRandom();
        myEvent+= mySource->GetRandom();
        hist->Fill(myEvent);
      }
    }
    // testing
    if(event==100)
    {
      histSave = (TH1F*)hist->Clone();
      histSave->SetName("ped");
    }
    if(event <= 100) continue;
    histSave->SetLineColor(kBlue);
    histSave->Draw();
    //hist->Fit("myPed");
    //myPed->SetLineColor(kBlue);
    //myPed->SetLineStyle(kDashed);
    //myPed->Draw("same");
    mySource->SetLineColor(kGreen);
    mySource->Draw("same");
    return;
    hist->Fit("myPedPlusSource");
    hist->Draw("same");

    //myPedPlusSource->Draw("same");
    //myPedPlusSource->SetParameter(3,0);
    //myPedPlusSource->SetLineColor(kRed);
    //myPedPlusSource->Draw("same");
    //myPed->SetLineColor(kBlue);
    //myPed->Draw("same");
    //mySourceShift->SetParameter(0,myPedPlusSource->GetParameter(3));
    //mySourceShift->SetParameter(1,myPedPlusSource->GetParameter(4));
    //mySourceShift->SetLineColor(kGreen);
    //mySourceShift->SetLineStyle(kDashed);
    //mySourceShift->Draw("same");
    //myPedPlusSource->SetParameters(1e10,5.42,0.74,5.72);
    //myPedPlusSource->SetLineColor(kBlue);
    //myPedPlusSource->Draw();
    return;
    //hist->Write();
    //delete hist;
  }
}
