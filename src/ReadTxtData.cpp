#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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

void ReadTxtData(std::string filePath, int fileSize ) {  
  
  std::cout << "Read text data! : " << filePath << std::endl;

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
  int ADCTotal = 0;
  float Ratio = 0;
  int channel = 0;  
  std::vector<int> pulse;
  
  // // ------------------------------------------------------- ouitput Ntuple
  std::string ofName = filePath.erase ( 0, filePath.rfind ( "/" )+1 );
  ofName.erase ( ofName.find ( ".xml" ) );
  ofName = "./data/" + ofName + ".root";
  TString T_ofName = ofName;
  std::cout << std::endl << T_ofName << std::endl << std::endl;
  TFile *fout = new TFile( T_ofName , "RECREATE" );
  TTree *tree = new TTree( "tree", "" );
  tree -> Branch ( "eventID", &eventID );
  tree -> Branch ( "timeStamp", &timeStamp );
  tree -> Branch ( "clockTime", &clockTime );
  tree -> Branch ( "pulse", &pulse );
  tree -> Branch ( "ADCTotal", &ADCTotal );
  tree -> Branch ( "Ratio", &Ratio );

  int grNum = 100;
  if ( grNum > fileSize ) grNum = fileSize;
  int grCnt = 0;
  TGraph * checkGr [grNum];
  
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
      	if ( lineSize>100 && dataCnt%(lineSize/100)==0 ) std::cout << "================================== Header : " << dataCnt << " | " << process*100 << "% processed." << std::endl;
	if ( lineSize<=100 ) std::cout << "================================== Header : " << dataCnt << " | " << process*100 << "% processed." << std::endl;
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
      std::vector<int> cor_data;
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
	    if ( ii<1000 ){
	      pedestalAve += data[ii];
	    }
	  }
	  pedestalAve /= 1000;
	  std::vector<int> cor_data;
	  for ( int ii=0; ii<data.size (); ii++ ) {
	    cor_data.push_back ( -1 *( data[ii] - pedestalAve ) );
	    // std::cout << "ii = " << ii << " | cor_data = " << -1 *( data[ii] - pedestalAve ) << std::endl;
	  }

	  ADCTotal = 0;
	  Ratio = 0;
	  int ADCTotalFast = 0;
	  int IntegEnd = cor_data.size ();
	  IntegEnd = 1400;
	  int maxIndex = std::distance ( cor_data.begin(), std::max_element ( cor_data.begin(), cor_data.end() ) );
	  int maxValue = *std::max_element ( cor_data.begin(), cor_data.end () );
	  int FastIntegrate = maxIndex + 40;
	  // // ------------------------------------ ADCTotal & Ratio	  
	  for ( int ii=0; ii<cor_data.size (); ii++ ) {
	    //------------------------ Mizukoshi method
	    if ( ii < IntegEnd ) {
	      ADCTotal += cor_data[ii];
	      // std::cout << ii << ", " << cor_data[ii ]<< ", " << ACTotal << std::endl;
	      if ( ii < FastIntegrate  ) {
	      	ADCTotalFast += cor_data[ii];
	      }
	    }
	  }
	  Ratio = float ( ADCTotalFast ) / float ( ADCTotal );
	  if (  0 > Ratio || Ratio >= 1.0  ) continue;
	  if (  ADCTotal < 0  ) continue;
	  //checkH2D -> Fill( float(ADCTotal)/1000, Ratio );
	  
	  
	  if (grCnt < grNum ){
	    checkGr [grCnt] = new TGraph ();
	    for ( int ii=0; ii<cor_data.size (); ii++ ) {
	      int nPt = checkGr [grCnt] -> GetN ();
	      checkGr [grCnt] -> SetPoint ( nPt, ii, float ( cor_data[ii] ) / float ( 1 ) );
	    }
	    grCnt ++;
	  }
	  
	  pulse = cor_data;
	  tree -> Fill ();
	  headerFlag = false;
	  dataFlag = false;
	  pulse.clear ();
	}
      }      
      lineCnt ++;    
    }

  std::cout << "================================== wrote to TFile: " << ofName << " ." << std::endl;
  tree -> Write ();
  fout -> Close();  
  
  std::cout << std::endl << " ====================================================================== done!" << std::endl << std::endl;

  TCanvas *c_Decode = new TCanvas ( "c_Decode", "", 1000, 600 );
  TH2D *frame = new TH2D ( "frame", "", 1, 0, 8192, 1, -1000, 1500 );
  frame -> SetXTitle ( "time [ns]" );
  frame -> SetYTitle ( "ADC [counts]" );
  frame -> Draw ();  
  for ( int ii=0; ii<grCnt; ii++ ) {
    checkGr[ii] -> SetLineColor ( 2 );
    checkGr[ii] -> Draw ("L Same");
  }
  c_Decode -> Print ( "./imgs/DecodeTest.png" );
  
}
