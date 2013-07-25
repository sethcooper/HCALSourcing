{
//=========Macro generated from canvas: c1/c1
//=========  (Wed Jul 17 18:48:45 2013) by ROOT version5.32/00
   TCanvas *c1 = new TCanvas("c1", "c1",374,168,700,500);
   gStyle->SetOptFit(1);
   c1->Range(-6.054054,0.5040854,37.18919,5.81705);
   c1->SetFillColor(0);
   c1->SetBorderMode(0);
   c1->SetBorderSize(2);
   c1->SetLogy();
   c1->SetTickx(1);
   c1->SetTicky(1);
   c1->SetLeftMargin(0.14);
   c1->SetRightMargin(0.12);
   c1->SetTopMargin(0.08);
   c1->SetBottomMargin(0.15);
   c1->SetFrameFillStyle(0);
   c1->SetFrameLineWidth(2);
   c1->SetFrameBorderMode(0);
   c1->SetFrameFillStyle(0);
   c1->SetFrameLineWidth(2);
   c1->SetFrameBorderMode(0);
   
   TH1F *rawHistEvent1001_Ieta12_Iphi12_Depth1 = new TH1F("rawHistEvent1001_Ieta12_Iphi12_Depth1","rawHistEvent1001_Ieta12_Iphi12_Depth1",32,0,32);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(3,100);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(4,6732);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(5,70550);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(6,126806);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(7,53631);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(8,4270);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetBinContent(9,40);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetEntries(32);
   
   TPaveStats *ptstats = new TPaveStats(0.704023,0.5190678,0.9971264,0.9957627,"brNDC");
   ptstats->SetName("stats");
   ptstats->SetBorderSize(2);
   ptstats->SetFillColor(0);
   ptstats->SetTextAlign(12);
   ptstats->SetTextFont(42);
   TText *text = ptstats->AddText("rawHistEvent1001_Ieta12_Iphi12_Depth1");
   text->SetTextSize(0.03654661);
   text = ptstats->AddText("Entries = 32     ");
   text = ptstats->AddText("Mean  = 4.915984 #pm 0.472542");
   text = ptstats->AddText("RMS   = 0.7996093 #pm 0.3341377");
   text = ptstats->AddText("Underflow =      0");
   text = ptstats->AddText("Overflow  =      0");
   text = ptstats->AddText("Integral = 262129");
   text = ptstats->AddText("#chi^{2} / ndf = 375.7321 / 4");
   text = ptstats->AddText("Prob  =      0");
   text = ptstats->AddText("Constant = 130868.3 #pm 305.3 ");
   text = ptstats->AddText("Mean     = 5.416622 #pm 0.001561 ");
   text = ptstats->AddText("Sigma    = 0.7979335 #pm 0.0010159 ");
   ptstats->SetOptStat(2222211);
   ptstats->SetOptFit(111111);
   ptstats->Draw();
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetListOfFunctions()->Add(ptstats);
   ptstats->SetParent(rawHistEvent1001_Ieta12_Iphi12_Depth1);
   
   TF1 *gaus = new TF1("gaus","gaus",0,32);
   gaus->SetFillColor(19);
   gaus->SetFillStyle(0);
   gaus->SetMarkerStyle(8);
   gaus->SetMarkerSize(0.8);

   Int_t ci;   // for color index setting
   ci = TColor::GetColor("#ff0000");
   gaus->SetLineColor(ci);
   gaus->SetLineWidth(2);
   gaus->SetChisquare(375.7321);
   gaus->SetNDF(4);
   gaus->GetXaxis()->SetNdivisions(506);
   gaus->GetXaxis()->SetLabelFont(42);
   gaus->GetXaxis()->SetTitleSize(0.06);
   gaus->GetXaxis()->SetTitleFont(42);
   gaus->GetYaxis()->SetNdivisions(506);
   gaus->GetYaxis()->SetLabelFont(42);
   gaus->GetYaxis()->SetTitleSize(0.06);
   gaus->GetYaxis()->SetTitleOffset(1.1);
   gaus->GetYaxis()->SetTitleFont(42);
   gaus->SetParameter(0,130868.3);
   gaus->SetParError(0,305.2513);
   gaus->SetParLimits(0,0,0);
   gaus->SetParameter(1,5.416622);
   gaus->SetParError(1,0.001561259);
   gaus->SetParLimits(1,0,0);
   gaus->SetParameter(2,0.7979335);
   gaus->SetParError(2,0.001015855);
   gaus->SetParLimits(2,0,7.996093);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetListOfFunctions()->Add(gaus);

   ci = TColor::GetColor("#000099");
   rawHistEvent1001_Ieta12_Iphi12_Depth1->SetLineColor(ci);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetXaxis()->SetLabelFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetXaxis()->SetLabelSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetXaxis()->SetTitleSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetXaxis()->SetTitleFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetYaxis()->SetLabelFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetYaxis()->SetLabelSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetYaxis()->SetTitleSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetYaxis()->SetTitleFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetZaxis()->SetLabelFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetZaxis()->SetLabelSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetZaxis()->SetTitleSize(0.035);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->GetZaxis()->SetTitleFont(42);
   rawHistEvent1001_Ieta12_Iphi12_Depth1->Draw("");
   
   TPaveText *pt = new TPaveText(0.01,0.9416102,0.6104598,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   text = pt->AddText("rawHistEvent1001_Ieta12_Iphi12_Depth1");
   pt->Draw();
   c1->Modified();
   c1->cd();
   c1->SetSelected(c1);
}
