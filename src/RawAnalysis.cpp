#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
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

float convertADCtoEnergy ( float ADC ) {
  float slope = 23.907;
  float intercept = -286.62;
  // slope = 75;
  // intercept = 0;
  float energy = ( ADC - intercept ) / slope;
  return energy;
}

std::tm convertUnixTimeToTm(unsigned long int unixTime) {
    // Unix時間をsystem_clockのtime_pointに変換
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(unixTime));
    // system_clock::time_pointをtm構造体に変換
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm gmtm = *std::gmtime(&tt);
    return gmtm;
}

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
  unsigned long int MaxeventID = 0;
  unsigned long int timeStamp;
  unsigned long int clockTime;
  unsigned long int timeStampGlobal = 0;
  unsigned long int timeStampPrev = 0;
  unsigned long int timeStampGlobalPrev = 0;
  unsigned long int timeStampMax = 10000;
  float timeStampGlobalDif = 0;  // ns
  float cnt_rate = 0;
  float sec_per_cnt = 8; //  [ ns/cnt]
  long long int stampCheck = std::pow( 2 , 31 );
  long long int stampCheck2 = std::pow( 2 , 32 ) - 1;
  unsigned long int clockTimeStart;
  unsigned long int clockTimePrev;
  long long int timeStampDif = 0;
  long long int dif_temp_max = 0;
  // unsigned long int clockTimePassed;
  int channel = 0;
  int timeWindow = 8192;
  
  int grNum = 100;
  if ( grNum > fileSize ) grNum = fileSize;
  int grCnt = 0;
  int grCnt2 = 0;
  TGraph * timeStampGr = new TGraph ();
  TGraph * timeStampGr2 = new TGraph ();
  TGraph * timeStampGr3 = new TGraph ();
  TGraph * timeStampGr4 = new TGraph ();
  TGraph * timeClockGr = new TGraph ();
  TGraph * timeClockGr2 = new TGraph ();
  TGraph * checkGr [grNum];
  TGraph * checkGr2 [grNum];
  TGraph * pedestalGr = new TGraph ();
  TH1D * pedeH1D = new TH1D ( "pedeH1D","", 40, 3680, 3720 );
  TH1D * peakPosH1D =  new TH1D ( "peakPosH1D","",  50, 150, 300 );
  TH1D * peakPosH1D2 = new TH1D ( "peakPosH1D2","", 50, 150, 300 );
  TH1D * peakPosH1D3 = new TH1D ( "peakPosH1D3","", 50, 150, 300 );
  TH1D * timeDifH1D = new TH1D ( "timeDifH1D","", 100, 0, 1 );
  TH2D * pedeH2D = new TH2D ( "pedeH2D","", 10000, 0, fileSize, 40, 3680, 3720 );
  TH2D * EnergyRatioH2D = new TH2D ( "EnergyRatioH2D","", 1000, 0, 100, 100, 0, 1 );
  TH2D * EnergyRatioH2D2 = new TH2D ( "EnergyRatioH2D2","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyRatioH2D3 = new TH2D ( "EnergyRatioH2D3","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyAfterH2D = new TH2D ( "EnergyAfterH2D","", 1000, 0, 60, 100, 0, 10 );  
  TH2D * EventIDRatioH2D = new TH2D ( "EventIDRatioH2D","", 1000, 0, fileSize, 100, 0, 1 );
  TH2D * EventIDADCTotalH2D = new TH2D ( "EventIDADCTotalH2D","", 1000, 0, fileSize, 200, 0, 1 );
  TH2D * EnergyFallingH2D = new TH2D ( "EnergyFallingH2D","", 1000, 0, 60, 75, 150, 300 );
  TH2D * EnergyRisingH2D = new TH2D ( "EnergyRisingH2D","", 1000, 0, 60, 200, 0, 200 );
  TH2D * EnergyTOTH2D = new TH2D ( "EnergyTOTH2D","", 1000, 0, 60, 100, 0, 200 );
  TH2D * EnergyPeakH2D = new TH2D ( "EnergyPeak","", 1000, 0, 60, 50, 150, 200 );
  //  TH2D * EnergyIncreaseH2D = new TH2D ( "EnergyIncrease","", 1000, 0, 60, 100, 0, 1 );
  TH2D * EnergyIncreaseH2D = new TH2D ( "EnergyIncrease","", 1000, 0, 60, 100, 0, 20 );
  TH1D * ADCTotalSeparationH1D = new TH1D ( "ADCTotalSeparationH1D","", 100, 10, 25 );
  TH1D * RatioSeparationH1D = new TH1D ( "RatioSeparationH1D","", 200, 0.2, 0.6 );
  unsigned long int hist_max = 4e3;
  TH1D * maxValueH1D = new TH1D ( "maxValueH1D","", 100, hist_max*0., hist_max*1.05 );
  TH1D * spectrumH1D = new TH1D ( "spectrumH1D","", 1000, 0, 60000 );
  TH2D * afterAlphaH2D = new TH2D ( "afterAlphaH2D","", 50, 0, 2, 100, 0, 50 );
  TH1D * beforeBetaH1D = new TH1D ( "beforeBetaH1D","", 100, 0, 4 );
  TGraph * afterAlphaGr = new TGraph ();
  TGraph * beforeBetaGr = new TGraph ();

  int ADCTotal = 0;
  float Ratio;  
  int FastCnt = 80;    
  float energyPrev = 0;
  float ratioPrev = 0;
  float energyCut1_lower= 1.22; // Alpha cut ( after quench )
  float energyCut1_upper = 1.5;  // Alpha cut ( after quench )
  float energyCut2_lower = 0.5;   // Beta cut
  float energyCut2_upper = 3.52;  // Beta cut
  energyCut1_lower= 1.0; // Alpha cut ( after quench )
  energyCut1_upper = 1.6;  // Alpha cut ( after quench )
  energyCut2_lower = 0.5;   // Beta cut
  energyCut2_upper = 3.6;  // Beta cut
  float BetaCut_lower = 0.54;
  float BetaCut_upper = 0.66;
  float AlphaCut_lower = 0.35;
  float AlphaCut_upper = 0.53;
  float timeDifCut = 1; // ms
  float  Fit_ADCTotal1 = 10;
  float  Fit_ADCTotal2 = 16;
  float  Fit_ADCTotal3 = 17;
  float  Fit_ADCTotal4 = 25;

  float  Fit_Ratio1 = 0.2;
  float  Fit_Ratio2 = 0.38;
  float  Fit_Ratio3 = 0.45;
  float  Fit_Ratio4 = 0.6;  

  TFile * tupleFile = new TFile ( "./data/root_file/Ntuples.root", "RECREATE" );
  TTree * tree = new TTree ( "tree", "tree" );
  tree -> Branch ( "eventID", &eventID, "eventID/I" );
  tree -> Branch ( "timeStampGlobalDif", &timeStampGlobalDif, "timeStampGlobalDif/F" );
  // tree -> Branch ( "ADCTotal", &ADCTotal, "ADCTotal/I" );
  // tree -> Branch ( "Ratio", &Ratio, "Ratio/F" );  
  
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
	if ( lineSize > 100 ){ if ( dataCnt%(lineSize/150)==0 ) std::cout << "================================== Header : " << dataCnt << " | " << process*100 << "% processed." << std::endl; }
  	while ( getline ( i_stream, str_buf, ' ' ) ) {	  
	  // std::cout << "   L ------------ str = " << str_buf << " | " << counter  << std::endl;
	  int index = str_buf.find ( "=" );
	  if ( index > 0 ) {
	    std::string subStr = str_buf.substr( index + 1 );
	    subStr.pop_back ();
	    subStr.erase( subStr.begin () );
	    if ( counter==1 ) {
	      eventID = std::stoull ( subStr ) ;
	      if ( eventID > MaxeventID ) MaxeventID = eventID;
	      // std::cout << "   L event ID = " << eventID << std::endl;
	    } else if ( counter==4 ) {
	      timeStamp = std::stoul ( subStr );
	      // std::cout << "   L timeStamp = " << timeStamp << std::endl;	      
	      timeStampDif =  timeStamp - timeStampPrev;	   
	      if ( timeStampDif < 0) {
		// std::cout << "dif = " << timeStampDif << std::endl;
		if ( timeStamp >= stampCheck ) {
		  timeStampDif = (stampCheck2 - timeStampPrev ) + (timeStamp - stampCheck );
		}
		if ( timeStamp < stampCheck ) {
		  timeStampDif = timeStamp*2;
		}
	      } 
	      timeStampGlobal += timeStampDif;
	      if ( dif_temp_max < timeStampDif ) dif_temp_max = timeStampDif;
	      // std::cout << "   L cnt, " << dataCnt << " | " << timeStamp << " - " << timeStampPrev << " = " << timeStampDif << std::endl;
	      timeStampPrev = timeStamp;
	    } else if ( counter==5 ) {	      
	      subStr.erase ( subStr.length()-2, subStr.length() );
	      clockTime = std::stoul ( subStr );
	      if ( dataCnt == 0 ) {
		clockTimeStart = clockTime;
		std::tm result_first = convertUnixTimeToTm( clockTimeStart );
		int fyear = result_first.tm_year + 1900;
		int fmonth = result_first.tm_mon + 1;
		int fday = result_first.tm_mday;
		int fhour = result_first.tm_hour;
		int fminutes = result_first.tm_min;
		int fsecond = result_first.tm_sec;
	      }
	      // std::cout << "   L clockTime = " << clockTime << std::endl;
	    }
	  }
  	  counter ++ ;	  
	}	
	
	timeStampGr -> SetPoint ( timeStampGr->GetN(), eventID, timeStamp );
	timeStampGr2 -> SetPoint ( timeStampGr2->GetN(), eventID, timeStampGlobal );
	timeClockGr -> SetPoint ( timeStampGr->GetN(), eventID, clockTime );
	// 年月日時を表示       
	std::tm result = convertUnixTimeToTm( clockTime );	
	int year = result.tm_year + 1900;
	int month = result.tm_mon + 1;
	int day = result.tm_mday;
	int hour = result.tm_hour;
	int minutes = result.tm_min;
	int second = result.tm_sec;	  
	int clockTimePassed = clockTime - clockTimeStart;
	cnt_rate =  float ( clockTimePassed ) * 1e9 / float (timeStampGlobal ); // ns/cnt
	// std::cout << std::endl << " ================= eventID / clockTimePassed / timeStampGlobal / timeStampDif / cnt_rate = " << std::endl
	// 	  << eventID << " / " << clockTimePassed << " / " << timeStampGlobal << " / " << timeStampDif << " / " << cnt_rate << std::endl;
	  
	  // if ( dataCnt > 0 ) std::cout << "  L ======================================== eventID / clock time passed / time stamp : " << eventID << " / " << clockTimePassed << " sec / " << timeStampGlobal << " cnt / " <<  cnt_rate << " ns/cnt"  << std::endl;	
	// std::cout << "   L年: " << year << std::endl;
	// std::cout << "   L月: " << month << std::endl;
	// std::cout << "   L日: " << day << std::endl;
	// std::cout << "   L時: " << hour << std::endl;
	// std::cout << "   L分: " << minutes << std::endl;
	// std::cout << "   L秒: " << second << std::endl;
	timeStampGr3 -> SetPoint ( timeStampGr3->GetN(), eventID, cnt_rate );
	timeStampGr4 -> SetPoint ( timeStampGr4->GetN(), eventID, timeStampDif );
	dataCnt ++;	
	continue;
      }

      timeStampGlobalDif = float ( timeStampDif ) * sec_per_cnt; // ns
      // std::cout << " =========================== dif = " << timeStampGlobalDif << "ns" << std::endl;
      // timeDifH1D -> Fill ( timeStampGlobalDif / 1e6 ); // ms
      timeStampGlobalPrev = timeStampGlobal;
	
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
	  float pedestalAve = 0;
	  int pedeCalc = 950;
	  int startCnt = 0;
	  int risingIndex = 0;
	  for ( int ii=0; ii<data.size (); ii++ ) {
	    // std::cout << ii << ", " << data[ii] << std::endl;
	    // ------------------------------------ pedestal average (1)
	    if (  5 < ii && ii < pedeCalc ){
	      // std::cout << "============================================================================= ii, pedestal ave = " << ii << ", " << data [ii] << std::endl; 
	      pedestalAve += data[ii];
	      pedeH1D -> Fill ( data [ii] );
	      pedeH2D -> Fill ( eventID, data [ii] );	      
	      }
	    if ( ii > pedeCalc && data[ii] > data[ii-1] ) {
	      startCnt += 1;
	      // std::cout << "ii, startCnt = " << ii << ", " << startCnt << std::endl;	    
	      if ( startCnt > 5) {
		risingIndex = ii - 6 - 15;
		// std::cout << "   L rising Index = " << ii << ", " << risingIndex << std::endl;
		break;
	      }
	    }
	  }	  
	  pedestalAve /= ( pedeCalc - 6 );
	  if ( pedestalAve > 0 ) {
	    // std::cout << "============================================================================= eventID, pedestal ave = " << eventID << ", " << pedestalAve << std::endl; 
	  }
	  int pedeCnt = 0;
	  float sigma = 1.375;
	  float pedestalAve2 = 0;
	  std::vector<int> cor_data;
	  // float intercept = 3702.86;	  
	  // float sloap = 1.78998e-05;
	  float intercept = 3703.62;	  
	  float sloap = 7.18288e-06;
	  // pedestalAve =  sloap*eventID + intercept;
	  // std::cout << "============================================================================= eventID, pedestal ave = " << eventID << ", " << pedestalAve << std::endl; 
	  for ( int ii=0; ii<data.size (); ii++ ) {
	    // std::cout << ii << ", " << data[ii] << std::endl;
	    // ------------------------------------ pedestal average (2)
	    if ( ii<pedeCalc ){
	      if ( abs(data[ii]-pedestalAve) < 3*sigma ) {
		pedestalAve2 += data[ii];
		pedeCnt ++;
	      }
	    }
	    cor_data.push_back ( -1 *( data[ii] - pedestalAve ) );    
	  }
	  pedestalAve2 /= pedeCnt;
	  int nPt = pedestalGr -> GetN ();
	  // std::cout << eventID << ", " << pedestalAve << std::endl;
	  // pedestalGr -> SetPoint ( nPt, eventID, pedestalAve2 );
	  timeWindow = cor_data.size();	  	  
	  
	  int ADCTotalIncrease = 0;
	  int ADCTotalFast = 0;
	  int ADCTotalFast2 = 0;
	  int ADCTotal3 = 0;
	  int ADCFast3 = 0;
	  int ADCTotalSlow = 0;
	  int ADCAfter = 0;
	  int increase = 0;
	  float Ratio2;
	  float Ratio3;	  
	  float RatioIncrease;
	  int IntegEnd = cor_data.size ();
	  int IntegEnd2 = cor_data.size ();
	  int maxIndex = std::distance ( cor_data.begin(), std::max_element ( cor_data.begin(), cor_data.end() ) );	  
	  int maxValue = *std::max_element ( cor_data.begin(), cor_data.end () );
	  
	  IntegEnd = 1800;
	  IntegEnd2 = IntegEnd; 	  
	  int FastIntegrate = 1290;
	  FastIntegrate = 1205;	  	 
	  
	  int FastIntegrate2 = FastIntegrate;
	  int SlowInteg = FastIntegrate;
	  float fallingRatio = 0.2;
	  float risingRatio = 0.2;
	  int fallingCnt = 0;
	  int risingCnt = 0;
	  int TOTCnt = 0;
	  bool fallingFlag = true;
	  bool risingFlag = true;
	  maxValueH1D -> Fill ( maxValue );

	  ADCTotal = 0;
	  Ratio = 0;
	  // std::cout << "max index = " << maxIndex << ", Fast integrate = " << FastIntegrate << std::endl;
	  // // ------------------------------------ ADCTotal & Ratio	  
	  for ( int ii=0; ii<cor_data.size (); ii++ ) {
	    //------------------------ Mizukoshi method
	    if ( risingIndex < ii && ii < IntegEnd ) {
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
	    if ( risingFlag && ii<maxIndex && cor_data[ii] > maxValue*risingRatio ) {
	      risingCnt = ii;
	      risingFlag = false;
	    }
	    if ( fallingFlag && ii>maxIndex && cor_data[ii] < maxValue*fallingRatio ) {
	      fallingCnt = ii;
	      TOTCnt = fallingCnt - risingCnt;
	      fallingFlag = false;
	    }
	    if ( risingFlag==false && fallingFlag==true ) {
	      ADCTotal3 += cor_data[ii];
	      if ( ii < FastIntegrate  ) {
	    	ADCFast3 += cor_data[ii];
	      }
	    }
	    if ( IntegEnd < ii && ii < IntegEnd2 ) {
	      increase += cor_data[ii];
	    }
	  }
	  Ratio = float ( ADCTotalFast ) / float ( ADCTotal );
	  Ratio2 = float ( ADCTotalFast2 ) / float ( ADCTotal );
	  Ratio3 = float ( ADCTotalSlow ) / float ( ADCTotal );
	  RatioIncrease = float ( increase ) / float ( ADCTotal );
	  float RatioThreshold = float ( ADCFast3) / float (ADCTotal3);	  
	  float energy = convertADCtoEnergy ( ADCTotal ) / 1000; // MeV

	  if ( ADCTotal < 1000 ) continue;	  
	  //======================================== sequential pulse analysis cut
	  //--------------------------------- 3.272 MeV Beta from 214_Bi
	  if ( energyCut2_lower < energyPrev && energyPrev < energyCut2_upper && BetaCut_lower < ratioPrev && ratioPrev < BetaCut_upper ) {
	    ;
	  } else {
	    energyPrev = energy;
	    ratioPrev = Ratio;
	    continue;	     
	  }	  
	  // //--------------------------------- 1.364 MeV Alpha from 214_Po
	  if ( energyCut1_lower < energy && energy < energyCut1_upper && AlphaCut_lower < Ratio && Ratio < AlphaCut_upper  ) { // 1.364 MeV Alpha
	    // if ( AlphaCut_lower < Ratio && Ratio < AlphaCut_upper  ) { // 1.364 MeV Alpha
	    ;
	  } else {
	    energyPrev = energy;
	    ratioPrev = Ratio;
	    continue;
	  };
	  //---------------------------------- less than 5ms
	  if ( timeStampGlobalDif/1e6 > timeDifCut ) {  // ms
	  // if ( timeStampGlobalDif/1e6 > 3 && timeStampGlobalDif/1e6 < 10 ) {  // ms
	    energyPrev = energy;
	    ratioPrev = Ratio;
	    continue;
	  }
	  //======================================== sequential pulse analysis cut
	  
	  // std::cout << "----------------------- time_dif = " << timeStampGlobalDif/1e6 << " ms" << std::endl;
	  // std::cout << "   L Energy prev = " << energyPrev << " MeV" << std::endl;
	  // std::cout << "   L Ratio_prev  = " << ratioPrev << std::endl;
	  // std::cout << "   L Energy = " << energy << " MeV" << std::endl;
	  // std::cout << "   L Ratio  = " << Ratio << std::endl;
	  // afterAlphaH2D -> Fill ( energy, timeStampGlobalDif/1e6 );
	  afterAlphaGr -> SetPoint ( afterAlphaGr->GetN(), energy, timeStampGlobalDif/1e6 );
	  beforeBetaH1D -> Fill ( energyPrev );
	  timeDifH1D -> Fill ( timeStampGlobalDif / 1e6 ); // ms
	  // std::cout << "   L energy, timeStampGlobalDif/1e6  = " << energy << ", " << timeStampGlobalDif/1e6 << std::endl;
	  
	  // ------------------------------------ Cut
	  int ADCCut = 10000;
	  int ADCRegion1_1 = 10000;
	  int ADCRegion1_2 = 20000;
	  int ADCRegion2_1 = 50000;
	  int ADCRegion2_2 = 60000;
	  float RatioRegion1_1 = 0.4;
	  float RatioRegion1_2 = 0.55;
	  float RatioRegion2_1 = 0.0;
	  float RatioRegion2_2 = 1.0;
	  if (  0 > Ratio  ) continue;
	  if (  1 < Ratio  ) continue;
	  // if (  ADCCut > ADCTotal  ) continue;
	  // if (  risingCnt < 100  ) continue;
	  // if (  risingCnt < 150  ) continue;
	  // if (  maxValue < 200  ) continue;
	  // if (  ADCTotal < 2000  ) continue;
	  // if (  pedestalAve < 3702.78+float(eventID)*1.79609e-5  ) continue;

	  EnergyRatioH2D -> Fill( float(ADCTotal)/1000, Ratio );
	  EnergyRatioH2D3 -> Fill ( float(ADCTotal)/1000, Ratio3 );
	  EnergyAfterH2D -> Fill ( float(ADCTotal)/1000, float(ADCAfter)/1000 );
	  EventIDRatioH2D -> Fill ( eventID, Ratio );
	  EventIDADCTotalH2D -> Fill ( eventID, float(ADCTotal)/1000 );
	  EnergyFallingH2D -> Fill ( float(ADCTotal)/1000, fallingCnt );
	  EnergyRisingH2D -> Fill ( float(ADCTotal)/1000, risingCnt );
	  EnergyTOTH2D -> Fill ( float(ADCTotal)/1000, TOTCnt );
	  peakPosH1D -> Fill ( maxIndex );
	  EnergyPeakH2D -> Fill ( float(ADCTotal)/1000, maxIndex );
	  EnergyIncreaseH2D -> Fill ( float(ADCTotal)/1000, float(increase)/1000 );
	  // EnergyIncreaseH2D -> Fill ( float(ADCTotal)/1000, RatioIncrease );
	  pedestalGr -> SetPoint ( nPt, eventID, pedestalAve );
	  spectrumH1D -> Fill ( ADCTotal );
	  if ( 10000 < ADCTotal && Ratio  < 0.4 ) {
	    ADCTotalSeparationH1D -> Fill ( float ( ADCTotal ) / 1000 );
	  }
	  RatioSeparationH1D -> Fill ( Ratio );	  
	  
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
	    // std::cout << "--------------------------------------- grCnt = " << grCnt << std::endl;
	    checkGr [grCnt] = new TGraph ();
	    for ( int ii=0; ii<cor_data.size (); ii++ ) {
	      nPt = checkGr [grCnt] -> GetN ();
	      checkGr [grCnt] -> SetPoint ( nPt, ii, float ( cor_data[ii] ) / float ( 1 ) );
	      // std::cout << "ii, data = " << ii << ", " << cor_data[ii] << std::endl;
	    }
	    grCnt ++;
	  }
	  
	  if (grCnt2 < grNum ){
	    if (1) {
	      checkGr2 [grCnt2] = new TGraph ();
	      for ( int ii=0; ii<cor_data.size (); ii++ ) {
	  	checkGr2 [grCnt2] -> SetPoint ( checkGr2 [grCnt2] -> GetN (), ii*4, float ( cor_data[ii] ) / float ( 1 ) );
	      }
	      grCnt2 ++;
	    }
	  }
	  tree -> Fill ();
	  energyPrev = energy;
	  ratioPrev = Ratio;
	  headerFlag = false;
	  dataFlag = false;
	}
      }      
      lineCnt ++;    
    }

  tree -> Write ();
  tupleFile -> Close ();

  std::cout << "------------------------------------------------------ Max Event ID = " << MaxeventID << std::endl;
  
  TFile * saveFile = new TFile ( "./data/savedHistgrams.root", "RECREATE" );  
  gStyle -> SetOptStat ( 0 );
  // gStyle -> SetPadGridX ( 1 );
  // gStyle -> SetPadGridY ( 1 );
  
  TCanvas *c = new TCanvas ( "c", "", 1000, 600 );
  TH2D *frame = new TH2D ( "frame", "", 1, 0, timeWindow, 1, -100, 4000 );
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

  // TGraph * grLine = new TGraph ();
  // grLine -> SetPoint ( 0, (169+80)*4, -100 );
  // grLine -> SetPoint ( 1, (169+80)*4, 1500 );
  // grLine -> SetLineColor ( 1 );
  // grLine -> SetLineStyle ( 2 );
  // grLine -> Draw ( "L Same" );

  // TGraph * grLine2 = new TGraph ();
  // grLine2 -> SetPoint ( 0, 500*4, -100 );
  // grLine2 -> SetPoint ( 1, 500*4, 1500 );
  // grLine2 -> SetLineColor ( 1 );
  // grLine2 -> SetLineStyle ( 2 );
  // grLine2 -> Draw ( "L Same" );
  
  // TGraph * grLine3 = new TGraph ();
  // grLine3 -> SetPoint ( 0, 800*4, -100 );
  // grLine3 -> SetPoint ( 1, 800*4, 1500 );
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
  pedestalGr -> Fit ("pol1");
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
  
  TCanvas *c10 = new TCanvas ( "c10", "", 1000, 800 );
  EnergyRisingH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyRisingH2D -> SetYTitle ( "rising Cnt [sampling]" );
  EnergyRisingH2D -> Draw ( "COLZ" );
  c10 -> Print ( "./imgs/EnergyRising2D.png" );
  
  TCanvas *c11 = new TCanvas ( "c11", "", 1000, 800 );
  EnergyTOTH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyTOTH2D -> SetYTitle ( "TOT Cnt [sampling]" );
  EnergyTOTH2D -> Draw ( "COLZ" );
  c11 -> Print ( "./imgs/EnergyTOTH2D.png" );

  TCanvas *c12 = new TCanvas ( "c12", "", 1000, 800 );
  EnergyPeakH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  EnergyPeakH2D -> SetYTitle ( "Peak Cnt [sampling]" );
  EnergyPeakH2D -> Draw ( "COLZ" );
  c12 -> Print ( "./imgs/EnergyPeakH2D.png" );
  
  TCanvas *c13 = new TCanvas ( "c13", "", 1000, 800 );
  EnergyIncreaseH2D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  // EnergyIncreaseH2D -> SetYTitle ( "Increasing Count Ratio to ADCTotal" );
  EnergyIncreaseH2D -> SetYTitle ( "Increasing Count [counts]" );
  EnergyIncreaseH2D -> Draw ( "COLZ" );
  c13 -> Print ( "./imgs/EnergyIncreaseH2D.png" );


  TCanvas *c14 = new TCanvas ( "c14", "", 1200, 600 );
  c14 -> Divide ( 2, 1 );
  c14 -> cd ( 1 );
  ADCTotalSeparationH1D -> SetXTitle ( "ADCTotal #times 10^{3} [counts]" );
  ADCTotalSeparationH1D -> SetYTitle ( "Events" );
  TF1 * f_ADCTotal14_1 = new TF1 ( "f14_1", "gaus", Fit_ADCTotal1, Fit_ADCTotal2 );
  ADCTotalSeparationH1D -> Fit ( f_ADCTotal14_1, "0", "", Fit_ADCTotal1, Fit_ADCTotal2 );
  float mu_ADCTotal14_1 = f_ADCTotal14_1 -> GetParameter ( 1 );
  float sigma_ADCTotal14_1 = f_ADCTotal14_1 -> GetParameter ( 2 );
  TF1 * f_ADCTotal14_2 = new TF1 ( "f14_2", "gaus", Fit_ADCTotal3, Fit_ADCTotal4 );
  ADCTotalSeparationH1D -> Fit ( f_ADCTotal14_2, "0", "", Fit_ADCTotal3, Fit_ADCTotal4 );
  float mu_ADCTotal14_2 = f_ADCTotal14_2 -> GetParameter ( 1 );
  float sigma_ADCTotal14_2 = f_ADCTotal14_2 -> GetParameter ( 2 );
  ADCTotalSeparationH1D -> Draw ();
  f_ADCTotal14_1 -> Draw ( "L Same" );
  f_ADCTotal14_1 -> SetLineColor  ( 4 );
  f_ADCTotal14_2 -> Draw ( "L Same" );
  float ADCTotal_border1 = mu_ADCTotal14_1 + sigma_ADCTotal14_1;
  float ADCTotal_border2 = mu_ADCTotal14_2 - sigma_ADCTotal14_2;
  float ADCTotal_separation = ADCTotal_border2 - ADCTotal_border1;
  c14 -> cd ( 2 );
  RatioSeparationH1D -> SetXTitle ( "Ratio" );
  RatioSeparationH1D -> SetYTitle ( "Events" );
  TF1 * f_Ratio14_1 = new TF1 ( "f14_3", "gaus", Fit_Ratio1, Fit_Ratio2 );
  RatioSeparationH1D -> Fit ( f_Ratio14_1, "0", "", Fit_Ratio1, Fit_Ratio2 );
  float mu_Ratio14_1 = f_Ratio14_1 -> GetParameter ( 1 );
  float sigma_Ratio14_1 = f_Ratio14_1 -> GetParameter ( 2 );
  TF1 * f_Ratio14_2 = new TF1 ( "f14_4", "gaus", Fit_Ratio3, Fit_Ratio4 );
  RatioSeparationH1D -> Fit ( f_Ratio14_2, "0", "", Fit_Ratio3, Fit_Ratio4 );
  float mu_Ratio14_2 = f_Ratio14_2 -> GetParameter ( 1 );
  float sigma_Ratio14_2 = f_Ratio14_2 -> GetParameter ( 2 );  
  RatioSeparationH1D -> Draw ();
  f_Ratio14_1 -> SetLineColor  ( 4 );
  f_Ratio14_1 -> Draw ( "L Same" );
  f_Ratio14_2 -> Draw ( "L Same" );
  float Ratio_border1 = mu_Ratio14_1 + sigma_Ratio14_1;
  float Ratio_border2 = mu_Ratio14_2 - sigma_Ratio14_2;
  float Ratio_separation = Ratio_border2 - Ratio_border1;    
  c14 -> Print ( "./imgs/Separation.png" );

  TCanvas *c15 = new TCanvas ( "c15", "", 1000, 600 );
  // TH2D *frame15 = new TH2D ( "frame15", "", 1, 0, fileSize, 1, 0, pow ( 2., 32) );
  TH2D *frame15 = new TH2D ( "frame15", "", 1, 0, fileSize, 1, 0, timeStampGlobal );
  frame15 -> SetXTitle ( "event ID" );
  frame15 -> SetYTitle ( "time stamp" );
  frame15 -> Draw ();  
  timeStampGr -> Draw ("L Same");
  timeStampGr2 -> SetMarkerSize ( 2 );
  timeStampGr2 -> SetMarkerStyle ( 20 );
  timeStampGr2 -> SetMarkerColor ( 2 );
  timeStampGr2 -> SetLineColor ( kRed );  
  timeStampGr2 -> Draw ("PL Same");
  std::cout << std::endl << "========================================================== stamp global = " <<timeStampGlobal << std::endl;
  c15 -> Print ( "./imgs/timestamp.png" );


  TCanvas *c15_2 = new TCanvas ( "c15_2", "", 1000, 600 );
  TH2D *frame15_2 = new TH2D ( "frame15_2", "", 1, 0, fileSize, 1, 0, clockTime );
  frame15_2 -> SetXTitle ( "event ID" );
  frame15_2 -> SetYTitle ( "time clock" );
  frame15_2 -> Draw ();
  timeClockGr -> Draw ("L Same");
  std::cout << std::endl << "========================================================== stamp global = " <<timeStampGlobal << std::endl;
  c15_2 -> Print ( "./imgs/clockTime.png" );

  TCanvas *c15_3 = new TCanvas ( "c15_3", "", 1000, 600 );
  TH2D *frame15_3 = new TH2D ( "frame15_3", "", 1, 0, fileSize, 1, 0, 20 );
  frame15_3 -> SetXTitle ( "event ID" );
  frame15_3 -> SetYTitle ( "time/cnt [ns/cnt]" );
  frame15_3 -> Draw ();
  timeStampGr3 -> Draw ("L Same");
  c15_3 -> Print ( "./imgs/cnt_rate.png" );

  TCanvas *c15_4 = new TCanvas ( "c15_4", "", 1000, 600 );
  timeDifH1D -> SetXTitle ( "time dif [ms]" );
  timeDifH1D -> SetYTitle ( "Event" );
  timeDifH1D -> Draw ();
  TF1 *f15_4 = new TF1 ( "f15_4", "[0]*exp(-1*x/[1])" );
  timeDifH1D -> Fit ( f15_4 );
  c15_4 -> Print ( "./imgs/time_diff.png" );
  timeDifH1D -> Write ();
  
  TCanvas *c15_5 = new TCanvas ( "c15_5", "", 1000, 600 );
  TH2D *frame15_5 = new TH2D ( "frame15_5", "", 1, 0, fileSize, 1, 0, dif_temp_max );
  frame15_5 -> SetXTitle ( "event ID" );
  frame15_5 -> SetYTitle ( "time Stamp Dif [cnt]" );
  frame15_5 -> Draw ();
  timeStampGr4 -> Draw ("L Same");
  c15_5 -> Print ( "./imgs/time_stamp_dif.png" );

  gStyle -> SetPadGridX ( 0 );
  gStyle -> SetPadGridY ( 0 );
  TCanvas *c15_6 = new TCanvas ( "c15_6", "", 1000, 600 );
  afterAlphaH2D -> SetXTitle ( "Energy [MeV]" );
  afterAlphaH2D -> SetYTitle ( "Time Delay [ms]" );
  afterAlphaH2D -> Draw ( "" );
  afterAlphaGr -> SetMarkerSize ( 1 );
  afterAlphaGr -> SetMarkerStyle ( 22 );
  afterAlphaGr -> Draw ( "P Same" );
  c15_6 -> Print ( "./imgs/after_alpha_h2d.png" );

  TCanvas *c15_7 = new TCanvas ( "c15_7", "", 1000, 600 );
  beforeBetaH1D -> SetXTitle ( "Energy [MeV]" );
  beforeBetaH1D -> SetYTitle ( "Events" );
  beforeBetaH1D -> Draw ();
  c15_7 -> Print ( "./imgs/before_beta_h1d.png" );
  
  TCanvas *c16 = new TCanvas ( "c16", "", 1000, 600 );
  maxValueH1D -> SetXTitle ( "pulse height" );
  maxValueH1D -> SetYTitle ( "Events" );
  maxValueH1D -> Draw ();
  std::cout << std::endl << "######################################" <<
    std::endl << " max pulhei average = " << maxValueH1D -> GetMean () <<
    std::endl << "###################################### " << std::endl;
  c16 -> Print ( "./imgs/maxValueH1D.png" );

  TCanvas *c17 = new TCanvas ( "c17", "", 1000, 600 );
  spectrumH1D -> SetXTitle ( "ADCTotal" );
  spectrumH1D -> SetYTitle ( "Events" );
  int fitMin17 = 4000;
  int fitMax17 = 12000;
  TF1 * f_ADCTotal17 = new TF1 ( "f17", "gaus", fitMin17, fitMax17 );
  // spectrumH1D -> Fit ( f_ADCTotal17, "0", "",   fitMin17, fitMax17 );  
  spectrumH1D -> Draw ();
  f_ADCTotal17 -> Draw ( "Same" );
  c17 -> Print ( "./imgs/spectrumH1D.png" );

  saveFile -> Close ();
  
  std::cout << std::endl << std::endl << "==================================================" << std::endl
    // << "ADCTotal_border1, ADCTotal_border2 = " << ADCTotal_border1 << ", " << ADCTotal_border2 << std::endl
	    << "separation ADCTotal = " << ADCTotal_separation << std::endl
    //	    << "Ratio_border1, Ratio_border2 = " << Ratio_border1 << ", " << Ratio_border2 << std::endl
	    << "separation Ratio = " << Ratio_separation << std::endl
	    << "==================================================" << std::endl <<std::endl;    
  std::cout << std::endl << " ====================================================================== done!" << std::endl << std::endl;

  // // 関数を呼び出してtm構造体を取得
  // std::tm result = convertUnixTimeToTm( clockTime );
  // // 年月日時を表示
  // std::cout << "年: " << result.tm_year + 1900 << std::endl;
  // std::cout << "月: " << result.tm_mon + 1 << std::endl;
  // std::cout << "日: " << result.tm_mday << std::endl;
  // std::cout << "時: " << result.tm_hour << std::endl;
  // std::cout << "分: " << result.tm_min << std::endl;
  // std::cout << "秒: " << result.tm_sec << std::endl;

}
