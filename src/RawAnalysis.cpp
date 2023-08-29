#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include "TApplication.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLine.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TTree.h"
#include "TProfile.h"
#include "TLegend.h"

void RawAnalysis(std::string filePath, int fileSize ) {  
  
  std::cout << "data : " << filePath << std::endl;
  std::ifstream ifs ( filePath );
  std::string line, str_buf;
  int lineSize = fileSize;
  int lineCnt = 0;
  int dataCnt = 0;
  bool headerFlag = false;
  bool dataFlag   = false;      
  unsigned long int eventID;
  unsigned long int timeStamp;
  unsigned long int clockTime;
  unsigned long int timeStampGlobal = 0;
  unsigned long int timeStampPrev = 0;
  unsigned long int timeStampMax = 10000;
  unsigned long int clockTimeGlobal = 0;
  unsigned long int clockTimePrev = 0;
  unsigned long int clockTimeMax = 10000;  
  int channel = 0;

  int grNum = 1;
  if ( grNum > fileSize ) grNum = fileSize;
  int grCnt = 0;
  int grCnt2 = 0;
  TGraph * checkGr [grNum];
  TGraph * checkGr2 [grNum];
  TGraph * pedestalGr = new TGraph ();
  TH1D * pedeH1D = new TH1D ( "pedeH1D","", 40, 3680, 3720 );
  TH1D * peakPosH1D =  new TH1D ( "peakPosH1D","",  50, 150, 300 );
  TH1D * peakPosH1D2 = new TH1D ( "peakPosH1D2","", 50, 150, 300 );
  TH1D * peakPosH1D3 = new TH1D ( "peakPosH1D3","", 50, 150, 300 );
  TH2D * pedeH2D = new TH2D ( "pedeH2D","", 10000, 0, fileSize, 40, 3680, 3720 );
  TH2D * EnergyRatioH2D = new TH2D ( "EnergyRatioH2D","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyRatioH2D2 = new TH2D ( "EnergyRatioH2D2","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyRatioH2D3 = new TH2D ( "EnergyRatioH2D3","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyAfterH2D = new TH2D ( "EnergyAfterH2D","", 1000, 0, 60, 100, 0, 10 );  
  TH2D * EventIDRatioH2D = new TH2D ( "EventIDRatioH2D","", 1000, 0, fileSize, 100, 0, 1 );
  TH2D * EventIDADCTotalH2D = new TH2D ( "EventIDADCTotalH2D","", 1000, 0, fileSize, 200, 0, 1 );
  TH2D * EnergyFallingH2D = new TH2D ( "EnergyFallingH2D","", 1000, 0, 60, 80, 0, 80 );  
  
  int FastCnt = 80;    
  float avePulse1[1024], avePulse2[1024];
  
  while ( getline ( ifs, line ) )
    {
      // --------------------------------------------------- file size cut
      if ( dataCnt > lineSize ) break;
      // std::cout << "line = " << lineCnt << " | " << line << std::endl;
      std::istringstream i_stream ( line );      

      // --------------------------------------------------- header
      int counter = 0;
      if ( line.find ( "<event" ) != std::string::npos ){
	headerFlag = true;
	float process =  float ( dataCnt ) / float ( lineSize );
	if ( lineSize > 100 ){ if ( dataCnt%(lineSize/100)==0 ) std::cout << "================================== Header : " << dataCnt << " | " << process*100 << "% processed." << std::endl; }
  	while ( getline ( i_stream, str_buf, ' ' ) ) {	  
	  // std::cout << "   L str = " << str_buf << " | " << counter  << std::endl;
	  int index = str_buf.find ( "=" );
	  if ( index > 0 ) {
	    std::string subStr = str_buf.substr( index + 1 );
	    subStr.pop_back ();
	    subStr.erase( subStr.begin () );
	    if ( counter==1 ) {
	      eventID = std::stoul ( subStr ) ;
	      // std::cout << "   L " << eventID << std::endl;
	    } else if ( counter==4 ) {
	      timeStamp = std::stoul ( subStr );
	      timeStampGlobal += timeStamp - timeStampPrev;
	      // std::cout << "   L " << timeStamp << " - " << timeStampPrev << " = " <<timeStampGlobal << std::endl;
	      if ( timeStamp - timeStampPrev < 0 ) {		
	      	std::cout << timeStamp << " - " << timeStampPrev << " = " <<timeStampGlobal << std::endl;
	      }
	      timeStampPrev = timeStamp;
	    } else if ( counter==5 ) {
	      subStr.erase ( subStr.length()-2, subStr.length() );
	      clockTime = std::stoul ( subStr );
	      clockTimeGlobal += clockTime - clockTimePrev;
	      // std::cout << "   L " << clockTime << " - " << clockTimePrev << " = " <<clockTimeGlobal << std::endl;
	      if ( clockTime - clockTimePrev < 0 ) {
	      	std::cout << "   L " << clockTime << " - " << clockTimePrev << " = " <<clockTimeGlobal << std::endl;
	      }
	      clockTimePrev = clockTime;
	    }
	  }
  	  counter ++ ;
	  
	}
	dataCnt ++;	
	continue;
      }
      
      counter = 0;
      std::vector<int> data;
      // // --------------------------------------------------- data
      if ( headerFlag ){
      	// std::cout << "line = " << lineCnt << " | " << line << std::endl;
      	if ( line.find ( "<trace" ) != std::string::npos ){
      	  dataFlag = true;
	  int value = 0;
      	  while ( getline ( i_stream, str_buf, ' ' ) ) {
	    // std::cout << "   L str = " << str_buf << " " << counter  << std::endl;
	    if ( counter < 2 ){
	      counter ++;
	      continue;	      
	    }
	    // ------------------------------------ push back data
	    if ( counter==2 ){
	      int index = str_buf.find ( "=" );
	      std::string subStr = str_buf.substr( index + 2 );
	      int index2 = subStr.find ( ">" );
	      std::string subStr2 = subStr.substr( index2 + 1 );	      
	      subStr = subStr.erase ( index2-1, subStr.length () );
	      channel = std::stoi ( subStr );
	      value = std::stoi ( subStr2 );
	      // std::cout << "   |   L counter =  " << ", " << counter << channel << std::endl;
	      // std::cout << "   |   L counter =  " << ", " << counter << value << std::endl;
	      data.push_back ( value );
	    } else {
	      value = std::stoi ( str_buf );
	      // std::cout << "   |   L " << counter << ", " << value << std::endl;
	      data.push_back ( value );
	    }
	    counter ++;
	  }

	  // -------------------------------------------------------------------- data check
	  int pedestalAve = 0;
	  for ( int ii=0; ii<data.size (); ii++ ) {
	    // std::cout << ii << ", " << data[ii] << std::endl;
	    // ------------------------------------ pedestal average (1)
	    if ( ii<150 ){
	      pedestalAve += data[ii];
	      pedeH1D -> Fill ( data [ii] );
	      pedeH2D -> Fill ( eventID, data [ii] );	      
	      }
	  }
	  pedestalAve /= 150;
	  int pedeCnt = 0;
	  float sigma = 1.375;
	  float pedestalAve2 = 0;
	  std::vector<int> cor_data;
	  for ( int ii=0; ii<data.size (); ii++ ) {
	    // std::cout << ii << ", " << data[ii] << std::endl;
	    // ------------------------------------ pedestal average (2)
	    if ( ii<150 ){
	      if ( abs(data[ii]-pedestalAve) < 5*sigma ) {
		pedestalAve2 += data[ii];
		pedeCnt ++;
	      }
	    }
	    cor_data.push_back ( -1 *( data[ii] - pedestalAve ) );    
	  }
	  pedestalAve2 /= pedeCnt;
	  int nPt = pedestalGr -> GetN ();
	  // std::cout << eventID << ", " << pedestalAve << std::endl;
	  pedestalGr -> SetPoint ( nPt, eventID, pedestalAve2 );

	  int ADCTotal = 0;
	  int ADCTotalFast = 0;
	  int ADCTotalFast2 = 0;
	  int ADCTotalSlow = 0;
	  int ADCAfter = 0;
	  float Ratio;
	  float Ratio2;
	  float Ratio3;
	  int IntegEnd = cor_data.size ();
	  IntegEnd = 450;
	  int maxIndex = std::distance ( cor_data.begin(), std::max_element ( cor_data.begin(), cor_data.end() ) );
	  int maxValue = *std::max_element ( cor_data.begin(), cor_data.end () );
	  int FastIntegrate = maxIndex + 80;
	  int FastIntegrate2 = maxIndex + 10;
	  int SlowInteg = maxIndex + 80;
	  float fallingRatio = 0.5;
	  int fallingCnt = 0;
	  bool fallingFlag = true;
	  // std::cout << "max index = " << maxIndex << ", Fast integrate = " << FastIntegrate << std::endl;
	  // // ------------------------------------ ADCTotal & Ratio	  
	  for ( int ii=0; ii<cor_data.size (); ii++ ) {
	    //------------------------ Mizukoshi method
	    if ( ii < IntegEnd ) {
	      ADCTotal += cor_data[ii];
	      if ( ii < FastIntegrate  ) {
	      	ADCTotalFast += cor_data[ii];
	      }
	      if ( ii < FastIntegrate2  ) {
	      	ADCTotalFast2 += cor_data[ii];
	      }
	      if ( SlowInteg < ii ) {
		ADCTotalSlow += cor_data[ii];
	      }
	    }
	    if ( IntegEnd < ii ) {
		ADCAfter += cor_data[ii];
	      }	      
	    if ( fallingFlag && ii>maxIndex && cor_data[ii] < maxValue*fallingRatio ) {
	      fallingCnt = ii - maxIndex;
	      // std::cout << "maxIndex, maxValue, " << maxIndex << ", " << maxValue << " | falling ii = " << ii << ", valule = " << cor_data[ii]  << ", fallingCnt = " << fallingCnt << std::endl;
	      fallingFlag = false;
	    }
	  }
	  Ratio = float ( ADCTotalFast ) / float ( ADCTotal );
	  Ratio2 = float ( ADCTotalFast2 ) / float ( ADCTotal );
	  Ratio3 = float ( ADCTotalSlow ) / float ( ADCTotal );
	  // std::cout << "   L " << ADCTotal << ", " << ADCTotalFast << ", " << Ratio << std::endl;
	  // if (  0 < Ratio && Ratio < 1.0  ) continue;

	  // ------------------------------------ Cut
	  int ADCCut = 2000;
	  int ADCRegion1_1 = 17500;
	  int ADCRegion1_2 = 18500;
	  int ADCRegion2_1 = 14000;
	  int ADCRegion2_2 = 15500;
	  float RatioRegion1_1 = 0.5;
	  float RatioRegion1_2 = 0.58;
	  float RatioRegion2_1 = 0.7;
	  float RatioRegion2_2 = 0.8;
	  if (  0 > Ratio  ) continue;
	  if (  1 < Ratio  ) continue;
	  if (  ADCCut > ADCTotal  ) continue;
	  //	  if (  maxValue > 250  ) continue;
	  EnergyRatioH2D -> Fill ( float(ADCTotal)/1000, Ratio );
	  EnergyRatioH2D3 -> Fill ( float(ADCTotal)/1000, Ratio3 );
	  EnergyAfterH2D -> Fill ( float(ADCTotal)/1000, float(ADCAfter)/1000 );
	  EventIDRatioH2D -> Fill ( eventID, Ratio );
	  EventIDADCTotalH2D -> Fill ( eventID, float(ADCTotal)/1000 );
	  EnergyFallingH2D -> Fill ( float(ADCTotal)/1000, fallingCnt );
	  peakPosH1D -> Fill ( maxIndex );
	  
	  if ( ADCRegion1_1 < ADCTotal && ADCTotal < ADCRegion1_2 && RatioRegion1_1 < Ratio  && Ratio < RatioRegion1_2 ) {	  
	    peakPosH1D2 -> Fill ( FastIntegrate  );
	    // EnergyRatioH2D -> Fill ( float(ADCTotal)/1000, Ratio );
	    // EnergyRatioH2D2 -> Fill ( float(ADCTotal)/1000, Ratio2 );
	  }	  
	  if ( ADCRegion2_1 < ADCTotal && ADCTotal < ADCRegion2_2 && RatioRegion2_1 < Ratio && Ratio < RatioRegion2_2 ) {
	    peakPosH1D3 -> Fill ( FastIntegrate );
	    // EnergyRatioH2D -> Fill ( float(ADCTotal)/1000, Ratio );
	    // EnergyRatioH2D2 -> Fill ( float(ADCTotal)/1000, Ratio2 );
	  }
	  
	  if (grCnt < grNum ){
	    //	    if ( 8000 < ADCTotal && ADCTotal < 10000 && Ratio < 0.6 ) {
	    if ( ADCRegion1_1 < ADCTotal && ADCTotal < ADCRegion1_2 && RatioRegion1_1 < Ratio  && Ratio < RatioRegion1_2 ) {
	      checkGr [grCnt] = new TGraph ();
	      for ( int ii=0; ii<cor_data.size (); ii++ ) {
	  	// std::cout << ii << ", " << cor_data[ii] << std::endl;
	  	nPt = checkGr [grCnt] -> GetN ();
	  	checkGr [grCnt] -> SetPoint ( nPt, ii*4, float ( cor_data[ii] ) / float ( 1 ) );
	      }
	      grCnt ++;
	    }
	  }
	  if (grCnt2 < grNum ){
	    // if ( 4000 < ADCTotal && ADCTotal < 6000 && 0.7 < Ratio && Ratio < 0.8 ) {
	    if ( ADCRegion2_1 < ADCTotal && ADCTotal < ADCRegion2_2 && RatioRegion2_1 < Ratio && Ratio < RatioRegion2_2 ) {
	      checkGr2 [grCnt2] = new TGraph ();
	      for ( int ii=0; ii<cor_data.size (); ii++ ) {
	  	// std::cout << ii << ", " << cor_data[ii] << std::endl;
	  	nPt = checkGr2 [grCnt2] -> GetN ();
	  	checkGr2 [grCnt2] -> SetPoint ( nPt, ii, float ( cor_data[ii] ) / float ( 1 ) );
	      }
	      grCnt2 ++;
	    }
	  }
	    
	  headerFlag = false;
	  dataFlag = false;
	}
      }      
      lineCnt ++;    
    }
  
  gStyle -> SetOptStat ( 0 );
  // gStyle -> SetPadGridX ( 1 );
  // gStyle -> SetPadGridY ( 1 );
  
  TCanvas *c = new TCanvas ( "c", "", 1000, 600 );
  TH2D *frame = new TH2D ( "frame", "", 1, 0, 1024*4, 1, -10, 200 );
  frame -> SetXTitle ( "time [ns]" );
  frame -> SetYTitle ( "ADC [counts]" );
  frame -> Draw ();  
  for ( int ii=0; ii<grCnt; ii++ ) {
    checkGr[ii] -> SetLineColor ( 2 );
    checkGr[ii] -> Draw ("L Same");
  }
  // for ( int ii=0; ii<grCnt2; ii++ ) {
  //   checkGr2[ii] -> SetLineColor ( 4 );
  //   checkGr2[ii] -> Draw ("L Same");
  // }

  // std::cout << std::endl << "grCnt1, grCnt2 = " << grCnt << ", " << grCnt2 << std::endl << std::endl;
  // TLegend * led1 = new TLegend ( 0.65, 0.65, 0.85, 0.85);
  // led1 -> AddEntry ( checkGr[0], "Region 1",  "l" );
  //  led1 -> AddEntry ( checkGr2[0], "Region 2",  "l" );
  //  led1 -> Draw ();  

  TGraph * grLine = new TGraph ();
  grLine -> SetPoint ( 0, (169+80)*4, -100 );
  grLine -> SetPoint ( 1, (169+80)*4, 1500 );
  grLine -> SetLineColor ( 1 );
  grLine -> SetLineStyle ( 2 );
  grLine -> Draw ( "L Same" );

  TGraph * grLine2 = new TGraph ();
  grLine2 -> SetPoint ( 0, 500*4, -100 );
  grLine2 -> SetPoint ( 1, 500*4, 1500 );
  grLine2 -> SetLineColor ( 1 );
  grLine2 -> SetLineStyle ( 2 );
  grLine2 -> Draw ( "L Same" );
  
  // TGraph * grLine3 = new TGraph ();
  // grLine3 -> SetPoint ( 0, 169+50, -100 );
  // grLine3 -> SetPoint ( 1, 169+50, 1500 );
  // grLine3 -> SetLineColor ( 1 );
  // grLine3 -> SetLineStyle ( 2 );
  // grLine3 -> Draw ( "L Same" );
  c -> Print ( "./imgs/test.png" );
  
  gStyle -> SetPadGridX ( 0 );
  gStyle -> SetPadGridY ( 0 );
  TCanvas *c2 = new TCanvas ( "c2", "", 1000, 600 );
  TH2D *frame2 = new TH2D ( "frame2", "", 1, 0, fileSize, 1, 3680, 3720 );
  frame2 -> SetXTitle ( "event ID" );
  frame2 -> SetYTitle ( "pedestal average [counts]" );
  frame2 -> Draw ();  
  pedestalGr -> Draw ("L Same");
  c2 -> Print ( "./imgs/pedestal.png" );
  
  TCanvas *c3 = new TCanvas ( "c3", "", 1000, 600 );
  gStyle -> SetOptFit (1111);
  pedeH1D -> SetXTitle ( "pedestal [counts]" );
  pedeH1D -> SetYTitle ( "events" );
  pedeH1D -> Fit ( "gaus" );
  pedeH1D -> Draw ();
  c3 -> Print ( "./imgs/pedeH1D.png" );

  TCanvas *c4 = new TCanvas ( "c4", "", 1000, 600 );
  gStyle -> SetOptFit (1111);
  pedeH2D -> SetXTitle ( "eventID" );
  pedeH2D -> SetYTitle ( "pedestal [counts]" );
  pedeH2D -> Draw ( "COLZ" );
  c4 -> Print ( "./imgs/pedeH2D.png" );
  
  TCanvas *c5 = new TCanvas ( "c5", "", 1000, 800 );
  EnergyRatioH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyRatioH2D -> SetYTitle ( "Ratio" );
  EnergyRatioH2D -> Draw ( "COLZ" );
  EnergyRatioH2D2 -> Draw ( "Same COLZ" );
  c5 -> Print ( "./imgs/Energy_vs_Ratio.png" );

  TCanvas *c5_2 = new TCanvas ( "c5_2", "", 1000, 800 );
  EnergyRatioH2D3 -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyRatioH2D3 -> SetYTitle ( "Ratio2" );
  EnergyRatioH2D3 -> Draw ( "COLZ" );
  c5_2 -> Print ( "./imgs/Energy_vs_Ratio2.png" );
  
  TCanvas *c5_3 = new TCanvas ( "c5_3", "", 1000, 800 );
  EnergyAfterH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyAfterH2D -> SetYTitle ( "ADCAfter #times 10^{3} [counts]" );
  EnergyAfterH2D -> Draw ( "COLZ" );
  c5_3 -> Print ( "./imgs/Energy_vs_ADCAfter.png" );

  gStyle -> SetPadGridX ( 1 );
  gStyle -> SetPadGridY ( 1 );
  TCanvas *c6 = new TCanvas ( "c6", "", 1000, 800 );
  EventIDRatioH2D -> SetXTitle ( "event ID" );
  EventIDRatioH2D -> SetYTitle ( "Ratio" );
  EventIDRatioH2D -> Draw ( "COLZ" );
  c6 -> Print ( "./imgs/EventIDRatioH2D_1.png" );
  TProfile * pf6 = EventIDRatioH2D -> ProfileX ();
  pf6 -> SetMarkerStyle ( 22 );
  pf6 -> SetMarkerSize ( 1 );
  pf6 -> SetMarkerColor ( 2 );
  pf6 -> SetLineColor ( 2 );
  pf6 -> Draw ( "AP Same" );
  c6 -> Print ( "./imgs/EventIDRatioH2D_2.png" );
  
  TCanvas *c7 = new TCanvas ( "c7", "", 1000, 800 );
  EventIDADCTotalH2D -> SetXTitle ( "event ID" );
  EventIDADCTotalH2D -> SetYTitle ( "ADCTotal #times 10^{3} [counts]" );
  EventIDADCTotalH2D -> Draw ( "COLZ" );
  c7 -> Print ( "./imgs/EventIDADCTotalH2D_1.png" );
  TProfile * pf7 = EventIDADCTotalH2D -> ProfileX ();
  pf7 -> SetMarkerStyle ( 22 );
  pf7 -> SetMarkerSize ( 1 );
  pf7 -> SetMarkerColor ( 2 );
  pf7 -> SetLineColor ( 2 );
  pf7 -> Draw ( "AP Same" );
  c7 -> Print ( "./imgs/EventIDADCTotalH2D_2.png" );

  TCanvas *c8 = new TCanvas ( "c8", "", 1000, 800 );
  EnergyFallingH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyFallingH2D -> SetYTitle ( "falling Cnt [sampling]" );
  EnergyFallingH2D -> Draw ( "COLZ" );
  c8 -> Print ( "./imgs/EnergyFallingH2D.png" );
  
  TCanvas *c9 = new TCanvas ( "c9", "", 1000, 600 );
  c9 -> SetLogy (1);  gStyle -> SetOptFit (1111);
  peakPosH1D -> SetXTitle ( "peak position [counts]" );
  peakPosH1D -> SetYTitle ( "events" );
  peakPosH1D -> SetLineColor ( 1 );
  peakPosH1D2 -> SetLineColor ( 2 );
  peakPosH1D3 -> SetLineColor ( 4);
  // peakPosH1D -> Fit ( "gaus" );  
  peakPosH1D -> Draw ();
  peakPosH1D2 -> Draw ( "Same" );
  peakPosH1D3 -> Draw ( "Same" );
  
  c9 -> Print ( "./imgs/peakPosH1D.png" );

  
  std::cout << std::endl << " ====================================================================== done!" << std::endl << std::endl;
  
}
