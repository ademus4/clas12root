#include <cstdlib>
#include <iostream>
#include <chrono>
#include <TFile.h>
#include <TTree.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TH1.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TBenchmark.h>
#include "clas12reader.h"
#include "clas12writer.h"

using namespace clas12;


void SetLorentzVector(TLorentzVector &p4,clas12::region_part_ptr rp){
  p4.SetXYZM(rp->par()->getPx(),rp->par()->getPy(),
	      rp->par()->getPz(),p4.M());

}

void CLAS12Writer3Pi(std::string inFile, std::string outputFile){
  // Record start time
  auto start = std::chrono::high_resolution_clock::now();

  //initialising clas12writer with path to output file
  clas12writer c12writer(outputFile.c_str());

  //can as writer not to write certain banks
  //c12writer.skipBank("REC::Cherenkov");
  //c12writer.skipBank("REC::Scintillator");

  cout<<"Analysing hipo file "<<inFile<<endl;

  //some particles
  auto db=TDatabasePDG::Instance();
  TLorentzVector beam(0,0,10.6,10.6);
  TLorentzVector target(0,0,0,db->GetParticle(2212)->Mass());
  TLorentzVector el(0,0,0,db->GetParticle(11)->Mass());
  TLorentzVector pr(0,0,0,db->GetParticle(2212)->Mass());
  TLorentzVector g1(0,0,0,0);
  TLorentzVector g2(0,0,0,0);
  TLorentzVector pip(0,0,0,db->GetParticle(211)->Mass());
  TLorentzVector pim(0,0,0,db->GetParticle(-211)->Mass());
   
  gBenchmark->Start("timer");
 
  int counter = 0;
  int writeCounter = 0;
   

  //create the event reader
  clas12reader c12(inFile.c_str());

  //assign a reader to the writer
  c12writer.assignReader(c12);
     
      
  while(c12.next()==true){

    // get particles by type
    auto electrons=c12.getByID(11);
    auto gammas=c12.getByID(22);
    auto protons=c12.getByID(2212);
    auto pips=c12.getByID(211);
    auto pims=c12.getByID(-211);
       
    if(electrons.size()==1 && gammas.size()==2 && protons.size()==1 &&
       pips.size()==1 &&pims.size() == 1){
       
      // set the particle momentum
      SetLorentzVector(el,electrons[0]);
      SetLorentzVector(pr,protons[0]);
      SetLorentzVector(g1,gammas[0]);
      SetLorentzVector(g2,gammas[1]);
      SetLorentzVector(pip,pips[0]);
      SetLorentzVector(pim,pims[0]);
	
      TLorentzVector miss=beam+target-el-pr-g1-g2-pip-pim;
      if(TMath::Abs(miss.M2())<0.5){
	//write out an event
	c12writer.writeEvent(); 
	writeCounter++;
      }
      counter++;
    }
  }
  

  //close writer
  c12writer.closeWriter();

  gBenchmark->Stop("timer");
  gBenchmark->Print("timer");
  
  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  std::cout << "Elapsed time: " << elapsed.count()<< " read events = "<<counter<<" wrote events = "<<writeCounter<<" s\n";

}