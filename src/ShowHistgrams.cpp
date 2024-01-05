#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>
#include <TF1.h>
#include <TCanvas.h>
#include "../include/ShowHistgrams.hpp"


float convertADCtoEnergy ( int ADC ) {
  float slope = 23.907;
  float intercept = -286.62;
  float energy = ( float ( ADC ) - intercept ) / slope;
  return energy;
}

void GetHistgrams ( std::string filePath ) {
  // ROOTファイルを開
  TString fName = filePath; //"./data/savedHistgrams.root";
  TFile *rootFile = new TFile(fName, "READ");  
  if (!rootFile->IsOpen()) {
    std::cerr << "Failed to open ROOT file!" << std::endl;
    return;
  }
  // ヒストグラムを取得
  TString histName1 = "timeDifH1D;1"; 
  TH1D *h1 = dynamic_cast<TH1D*>(rootFile->Get(histName1));  
  if (!h1) {
    std::cerr << "Failed to retrieve histogram from ROOT file!" << std::endl;
    return;
  }
  // ヒストグラムを表示
  TCanvas *c1 = new TCanvas("c1", "");
  h1 -> Draw();
  TF1 *f1 = new TF1 ( "f1", "[0]*exp(-1*x/[1])" );
  f1 -> SetParameters ( 1, 0.5 );
  h1 -> Fit ( f1, "", "", 1.2, 5 );
  c1-> Print ( "./imgs/timeDifH1D.png" );
  // c1->Update();  
  // ループを実行してウィンドウを開いたままにする
  // c1 -> WaitPrimitive();  
  // ファイルを閉じる
  rootFile->Close();
  delete rootFile;
}

void GetNtuples ( std::string filePath ) {
  // ファイルを開く
  TString fName = filePath; //"./data/savedHistgrams.root";
  TFile *file = TFile::Open(fName);  
  // TTreeを取得
  TTree *tree;
  file->GetObject("tree", tree);
  // データを読み込む変数を定義
  Int_t eventID;
  Float_t timeDif;
  Int_t ADCTotal;
  Float_t Ratio;  
  // ブランチに変数を関連付ける
  tree->SetBranchAddress("eventID", &eventID);
  tree->SetBranchAddress( "timeStampGlobalDif", &timeDif );
  tree->SetBranchAddress("ADCTotal", &ADCTotal);
  tree->SetBranchAddress("Ratio", &Ratio);
  
  // イベントをループしてデータを読み込む
  Long64_t nEntries = tree->GetEntries();
  std::cout << "================================================================ loop = " << nEntries << std::endl;
  for (Long64_t i = 0; i < nEntries; i++) {
    tree->GetEntry(i);
    float energy = convertADCtoEnergy ( ADCTotal ) / 1000; // MeV
    // 例：各イベントのデータを表示
    std::cout << std::endl << "------------------------------------------- Event ID: " << eventID << std::endl
	      << "ADCtotal : " << ADCTotal << std::endl
      	      << "ADCtotal : " << energy << " MeV" << std::endl
    	      << "Ratio    : " << Ratio << std::endl
	      << "Time Dif : " << timeDif/1e6 << " ms" << std::endl;
  }  
  // ファイルを閉じる
  file->Close();
}


void ShowHistgrams ( std::string filePath, std::string filePath2 ) {
  std::cout << "Show Histgrams! >> " << filePath << std::endl;
  // GetHistgrams ( filePath ) ;
  GetNtuples ( filePath2 ) ;
}
