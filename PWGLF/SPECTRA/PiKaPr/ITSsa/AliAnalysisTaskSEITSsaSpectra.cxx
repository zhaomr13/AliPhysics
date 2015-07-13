/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////
// AliAnalysisTaskSE for the extraction of the various histograms to
// study the pt spectra of identified hadrons:
// - log(dEdx)-log(dEdxBB) distributions for pions, kaons and protons in pt bins
// - Pt distributions of pions, kaons and protons with nSigma PID
// Authors: 
// E. Biolcati, biolcati@to.infn.it
// L. Milano, milano@to.infn.it
// F. Prino, prino@to.infn.it
///////////////////////////////////////////////////////////////////////////

#include "AliTrigger.h"
#include <TH1F.h>
#include <TF1.h>
#include <TRandom3.h>
#include <TH2F.h>
#include <TChain.h>
#include <TNtuple.h>
#include <TParticle.h>
#include <Rtypes.h>
#include "AliAnalysisTaskSE.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliESDEvent.h"
#include "AliESDInputHandler.h"
#include "AliESDtrack.h"
#include "AliStack.h"
#include "AliMCEventHandler.h"
#include "AliMCEvent.h"
#include "AliPhysicsSelection.h"
#include "AliAnalysisTaskSEITSsaSpectra.h"
#include "AliESDtrackCuts.h"
#include "AliCentrality.h"
#include "AliMultiplicity.h"
#include "AliESDUtils.h"
#include "AliITSPIDResponse.h"

ClassImp(AliAnalysisTaskSEITSsaSpectra)
/* $Id$ */

//________________________________________________________________________
AliAnalysisTaskSEITSsaSpectra::AliAnalysisTaskSEITSsaSpectra():
AliAnalysisTaskSE("taskITSsaSpectra"),
  fESD(0),
  fOutput(0),
  fListCuts(0),
  fHistNEvents(0),
  fHistMult(0),
  fHistCen(0),
  fHistNTracks(0),
  fHistNTracksPos(0),
  fHistNTracksNeg(0),
  fHistDEDX(0),
  fHistDEDXdouble(0),
  fHistBeforeEvSel(0),
  fHistAfterEvSel(0),
  fDCAxyCutFunc(0x0),
  fDCAzCutFunc(0x0),
  fITSPIDResponse(0),
  fMinSPDPts(1),
  fMinNdEdxSamples(3),
  fMindEdx(0.),
  fMinNSigma(1.5),
  fMaxY(0.5),
  fMaxChi2Clu(2.5),
  fNSigmaDCAxy(7.),
  fNSigmaDCAz(7.),
  fEtaRange(0.8),
  fLowMult(-1),
  fUpMult(-1),
  fLowCentrality(-1.0),
  fUpCentrality(-1.0),
  fMultEstimator(0),
  fHImode(0),
  fYear(2010),
  fMC(kFALSE), 
  fSmearMC(kFALSE),
  fSmearP(0.),
  fSmeardEdx(0.),
  fRandGener(0),
  fFillNtuple(kFALSE),
  fLowEnergypp(kFALSE),
  fNtupleNSigma(0),
  fNtupleMC(0)
{
  // Constructor
  Double_t xbins[kNbins+1]={0.08,0.10,0.12,0.14,0.16,0.18,0.20,0.25,0.30,0.35,0.40,0.45,0.50,0.55,0.60,0.65,0.70,0.75,0.80,0.85,0.90,0.95,1.0};
  for(Int_t iBin=0; iBin<kNbins+1; iBin++) fPtBinLimits[iBin]=xbins[iBin];
  fRandGener=new TRandom3(0);

  for(Int_t j=0; j<3; j++){
    fHistPosNSigmaSep[j]=0x0;
    fHistNegNSigmaSep[j]=0x0;

    fHistPrimMCpos[j]=0x0;
    fHistPrimMCneg[j]=0x0;
    fHistSecStrMCpos[j]=0x0;
    fHistSecStrMCneg[j]=0x0;
    fHistSecMatMCpos[j]=0x0;
    fHistSecMatMCneg[j]=0x0;
    fHistPrimMCposBefEvSel[j]=0x0;
    fHistPrimMCposBefEvSelEtaY[j]=0x0;
    fHistPrimMCposBefEvSelEta[j]=0x0;
    fHistPrimMCnegBefEvSel[j]=0x0;
    fHistPrimMCnegBefEvSelEtaY[j]=0x0;
    fHistPrimMCnegBefEvSelEta[j]=0x0;
    fHistSecStrMCposBefEvSel[j]=0x0;
    fHistSecStrMCnegBefEvSel[j]=0x0;
    fHistSecMatMCposBefEvSel[j]=0x0;
    fHistSecMatMCnegBefEvSel[j]=0x0;
    fHistPrimMCposReco[j]=0x0;
    fHistPrimMCposRecoEtaY[j]=0x0;
    fHistPrimMCnegReco[j]=0x0;
    fHistPrimMCnegRecoEtaY[j]=0x0;
    fHistSecStrMCposReco[j]=0x0;
    fHistSecStrMCnegReco[j]=0x0;
    fHistSecMatMCposReco[j]=0x0;
    fHistSecMatMCnegReco[j]=0x0;
  }

  for(Int_t j=0; j<4; j++) fHistCharge[j]=0x0;

  for(Int_t j=0; j<kNbins; j++){
    fHistPosPi[j]=0x0;
    fHistPosK[j]=0x0;
    fHistPosP[j]=0x0;
    fHistNegPi[j]=0x0;
    fHistNegK[j]=0x0;
    fHistNegP[j]=0x0;
    fHistDCAPosPi[j]=0x0;
    fHistDCAPosK[j]=0x0;
    fHistDCAPosP[j]=0x0;
    fHistDCANegPi[j]=0x0;
    fHistDCANegK[j]=0x0;
    fHistDCANegP[j]=0x0;
    fHistMCPrimDCAPosPi[j]=0x0;
    fHistMCPrimDCAPosK[j]=0x0;
    fHistMCPrimDCAPosP[j]=0x0;
    fHistMCPrimDCANegPi[j]=0x0;
    fHistMCPrimDCANegK[j]=0x0;
    fHistMCPrimDCANegP[j]=0x0;
    fHistMCSecStDCAPosPi[j]=0x0;
    fHistMCSecStDCAPosK[j]=0x0;
    fHistMCSecStDCAPosP[j]=0x0;
    fHistMCSecStDCANegPi[j]=0x0;
    fHistMCSecStDCANegK[j]=0x0;
    fHistMCSecStDCANegP[j]=0x0;
    fHistMCSecMatDCAPosPi[j]=0x0;
    fHistMCSecMatDCAPosK[j]=0x0;
    fHistMCSecMatDCAPosP[j]=0x0;
    fHistMCSecMatDCANegPi[j]=0x0;
    fHistMCSecMatDCANegK[j]=0x0;
    fHistMCSecMatDCANegP[j]=0x0;
    fHistMCPosOtherHypPion[j]=0x0;
    fHistMCPosOtherHypKaon[j]=0x0;
    fHistMCPosOtherHypProton[j]=0x0;
    fHistMCPosElHypPion[j]=0x0;
    fHistMCPosElHypKaon[j]=0x0;
    fHistMCPosElHypProton[j]=0x0;
    fHistMCPosPiHypPion[j]=0x0;
    fHistMCPosPiHypKaon[j]=0x0;
    fHistMCPosPiHypProton[j]=0x0;
    fHistMCPosKHypPion[j]=0x0;
    fHistMCPosKHypKaon[j]=0x0;
    fHistMCPosKHypProton[j]=0x0;
    fHistMCPosPHypPion[j]=0x0;
    fHistMCPosPHypKaon[j]=0x0;
    fHistMCPosPHypProton[j]=0x0;
    fHistMCNegOtherHypPion[j]=0x0;
    fHistMCNegOtherHypKaon[j]=0x0;
    fHistMCNegOtherHypProton[j]=0x0;
    fHistMCNegElHypPion[j]=0x0;
    fHistMCNegElHypKaon[j]=0x0;
    fHistMCNegElHypProton[j]=0x0;
    fHistMCNegPiHypPion[j]=0x0;
    fHistMCNegPiHypKaon[j]=0x0;
    fHistMCNegPiHypProton[j]=0x0;
    fHistMCNegKHypPion[j]=0x0;
    fHistMCNegKHypKaon[j]=0x0;
    fHistMCNegKHypProton[j]=0x0;
    fHistMCNegPHypPion[j]=0x0;
    fHistMCNegPHypKaon[j]=0x0;
    fHistMCNegPHypProton[j]=0x0;
  }

  for(Int_t j=0; j<3; j++){
    fHistPosNSigmaMean[j]=0x0;
    fHistPosNSigmaMCMean[j]=0x0;
    fHistPosNSigmaPrimMean[j]=0x0;
    fHistPosNSigmaPrimMCMean[j]=0x0;
    fHistNegNSigmaMean[j]=0x0;
    fHistNegNSigmaMCMean[j]=0x0;
    fHistNegNSigmaPrimMean[j]=0x0;
    fHistNegNSigmaPrimMCMean[j]=0x0;

    fHistPosNSigma[j]=0x0;
    fHistPosNSigmaMC[j]=0x0;
    fHistPosNSigmaPrim[j]=0x0;
    fHistPosNSigmaPrimMC[j]=0x0;
    fHistNegNSigma[j]=0x0;
    fHistNegNSigmaMC[j]=0x0;
    fHistNegNSigmaPrim[j]=0x0;
    fHistNegNSigmaPrimMC[j]=0x0;
  }

  DefineInput(0, TChain::Class());
  DefineOutput(1, TList::Class());
  DefineOutput(2,TList::Class());
}

//___________________________________________________________________________
AliAnalysisTaskSEITSsaSpectra::~AliAnalysisTaskSEITSsaSpectra(){
  // Destructor
  if (fOutput) {
    delete fOutput;
    fOutput = 0;
  }
  if (fListCuts) {
    delete fListCuts;
    fListCuts = 0;
  }
  if(fRandGener) delete fRandGener;
  if(fITSPIDResponse) delete fITSPIDResponse;
  delete fDCAxyCutFunc;
  delete fDCAzCutFunc;
}

//________________________________________________________________________
Double_t AliAnalysisTaskSEITSsaSpectra::CookdEdx(Double_t *s) const {
  // truncated mean for the dEdx
  Int_t nc=0; 
  Double_t dedx[4]={0.,0.,0.,0.};
  for (Int_t il=0; il<4; il++) { // count good (>0) dE/dx values
    if(s[il]>fMindEdx){
      dedx[nc]= s[il];
      nc++;
    }
  }
  if(nc<fMinNdEdxSamples) return -1.;
  
  Double_t tmp;
  Int_t swap; // sort in ascending order 
  do {
    swap=0;
    for (Int_t i=0; i<nc-1; i++) {
      if (dedx[i]<=dedx[i+1]) continue;
      tmp=dedx[i];
      dedx[i]=dedx[i+1];
      dedx[i+1]=tmp;
      swap++;
    } 
  } while (swap);
  
  Double_t sumamp=0,sumweight=0;
  Double_t weight[4]={1.,1.,0.,0.};
  if(nc==3) weight[1]=0.5;
  else if(nc<3) weight[1]=0.;
  for (Int_t i=0; i<nc; i++) {
    sumamp+= dedx[i]*weight[i];
    sumweight+=weight[i];
  }
  return sumamp/sumweight;
}


//________________________________________________________________________
Bool_t AliAnalysisTaskSEITSsaSpectra::DCAcut(Double_t impactXY, Double_t impactZ, Double_t pt) const {
  // cut on transverse impact parameter updaated on 20-5-2010
  // from the study of L. Milano, F. Prino on the ITS standalone tracks
  // using the common binning of the TPC tracks

  Double_t xyMax = fDCAxyCutFunc->Eval(pt); //in micron
  if((TMath::Abs(impactXY)*10000)>xyMax) return kFALSE;  
  Double_t zMax = fDCAzCutFunc->Eval(pt); //in micron
  if((TMath::Abs(impactZ)*10000)>zMax) return kFALSE;
  
  return kTRUE;
}

//________________________________________________________________________
Double_t AliAnalysisTaskSEITSsaSpectra::Eta2y(Double_t pt, Double_t m, Double_t eta) const {
  // convert eta to y
  Double_t mt = TMath::Sqrt(m*m + pt*pt);
  return TMath::ASinH(pt/mt*TMath::SinH(eta));
}


//________________________________________________________________________
void AliAnalysisTaskSEITSsaSpectra::Init(){
  //
  // Initialization
  //
  fListCuts=new TList();
  fListCuts->SetOwner();
  Double_t xyP[3];
  Double_t zP[3];
  if(fMC){
    if(fYear==2009){
      xyP[0]=88.63; //MC LHC10a12
      xyP[1]=19.57;
      xyP[2]=1.65;
      zP[0]=140.98;
      zP[1]=62.33;
      zP[2]=1.15;
    }else{
      xyP[0]=36.; //MC LHC10d1
      xyP[1]=43.9;
      xyP[2]=1.3;
      zP[0]=111.9;
      zP[1]=59.8;
      zP[2]=1.2;
    }
  }else{
    if(fYear==2009){
      xyP[0]=85.28;//DATA 900 GeV pass6
      xyP[1]=25.78;
      xyP[2]=1.55;
      zP[0]=146.80;
      zP[1]=70.07;
      zP[2]=1.11;
    }else{
      xyP[0]=32.7;//DATA 7 TeV pass2
      xyP[1]=44.8;
      xyP[2]=1.3;
      zP[0]=117.3;
      zP[1]=66.8;
      zP[2]=1.2;
    }
  }
  fDCAxyCutFunc = new TF1("fDCAxyCutFunc","[3]*([0]+[1]/TMath::Power(TMath::Abs(x),[2]))",0.05,10.);
  for(Int_t ipar=0; ipar<3; ipar++) fDCAxyCutFunc->SetParameter(ipar,xyP[ipar]);
  fDCAxyCutFunc->SetParameter(3,fNSigmaDCAxy);


						
  fDCAzCutFunc = new TF1("fDCAzCutFunc","[3]*([0]+[1]/TMath::Power(TMath::Abs(x),[2]))",0.05,10.);
  for(Int_t ipar=0; ipar<3; ipar++) fDCAzCutFunc->SetParameter(ipar,zP[ipar]);
  fDCAzCutFunc->SetParameter(3,fNSigmaDCAz);

  fListCuts->Add(fDCAxyCutFunc);
  fListCuts->Add(fDCAzCutFunc);

  PostData(2,fListCuts);
}

//________________________________________________________________________
void AliAnalysisTaskSEITSsaSpectra::UserCreateOutputObjects(){
  // Create a TList with histograms and a TNtuple
  // Called once

  fOutput = new TList();
  fOutput->SetOwner();
  fOutput->SetName("Spiderman");
  
  fHistNEvents = new TH1F("fHistNEvents", "Number of processed events",9,-1.5,7.5);
  fHistNEvents->Sumw2();
  fHistNEvents->SetMinimum(0);
  fHistNEvents->GetXaxis()->SetBinLabel(1,"Read from ESD");
  fHistNEvents->GetXaxis()->SetBinLabel(2,"Pass Phys. Sel. + Trig");
  fHistNEvents->GetXaxis()->SetBinLabel(3,"SDD read out");
  fHistNEvents->GetXaxis()->SetBinLabel(4,"In mult. range");
  fHistNEvents->GetXaxis()->SetBinLabel(5,"With SPD vertex");
  fHistNEvents->GetXaxis()->SetBinLabel(6,"Vertex contributors >0");
  fHistNEvents->GetXaxis()->SetBinLabel(7,"|zVertex|<10");
  fHistNEvents->GetXaxis()->SetBinLabel(8,"Error on zVertex<0.5");
  fHistNEvents->GetXaxis()->SetBinLabel(9,"Good Z vertex");
  fOutput->Add(fHistNEvents);
  
  fHistMult = new TH1F("fHistMult", "Event Multiplicity",3000,-0.5,2999.5);
  fHistMult->Sumw2();
  fHistMult->SetMinimum(0);
  if(fMultEstimator==0) fHistMult->GetXaxis()->SetTitle("Multiplicity |#eta|<0.8");
  else if(fMultEstimator==1) fHistMult->GetXaxis()->SetTitle("Tracklets |#eta|<0.8");
  else if(fMultEstimator==2) fHistMult->GetXaxis()->SetTitle("Clusters on SPD1");
  fOutput->Add(fHistMult);
  
  fHistCen = new TH1F("fHistCen", "Event Centrality",101,-0.5,100.5);
  fHistCen->Sumw2();
  fHistCen->SetMinimum(0);
  fOutput->Add(fHistCen);
  
  fHistNTracks = new TH1F("fHistNTracks", "Number of ITSsa tracks",20,0.5,20.5);
  fHistNTracks->Sumw2();
  fHistNTracks->SetMinimum(0);
  fOutput->Add(fHistNTracks);
  
  fHistNTracksPos = new TH1F("fHistNTracksPos", "Number of positive ITSsa tracks",20,0.5,20.5);
  fHistNTracksPos->Sumw2();
  fHistNTracksPos->SetMinimum(0);
  fOutput->Add(fHistNTracksPos);
  
  fHistNTracksNeg = new TH1F("fHistNTracksNeg", "Number of negative ITSsa tracks",20,0.5,20.5);
  fHistNTracksNeg->Sumw2();
  fHistNTracksNeg->SetMinimum(0);
  fOutput->Add(fHistNTracksNeg);
  
  //binning for the histogram
  const Int_t hnbins=400;
  Double_t hxmin = 0.01;
  Double_t hxmax = 10;
  Double_t hlogxmin = TMath::Log10(hxmin);
  Double_t hlogxmax = TMath::Log10(hxmax);
  Double_t hbinwidth = (hlogxmax-hlogxmin)/hnbins;
  Double_t hxbins[hnbins+1];
  hxbins[0] = 0.01; 
  for (Int_t i=1;i<=hnbins;i++) {
    hxbins[i] = hxmin + TMath::Power(10,hlogxmin+i*hbinwidth);
  }
  
  fHistDEDX = new TH2F("fHistDEDX","",hnbins,hxbins,900,0,1000);
  fOutput->Add(fHistDEDX);
  
  fHistDEDXdouble = new TH2F("fHistDEDXdouble","",500,-5,5,900,0,1000);
  fOutput->Add(fHistDEDXdouble);
  
  fHistBeforeEvSel = new TH1F("fHistBeforeEvSel","fHistBeforeEvSel",kNbins,fPtBinLimits);
  fHistAfterEvSel = new TH1F("fHistAfterEvSel","fHistAfterEvSel",kNbins,fPtBinLimits);
  fOutput->Add(fHistBeforeEvSel);
  fOutput->Add(fHistAfterEvSel);
  
  
  
  for(Int_t j=0;j<3;j++){
    
    fHistPosNSigmaSep[j] = new TH2F(Form("fHistPosNSigmaSep%d",j),"",hnbins,hxbins,1000,-10,10);
    fOutput->Add(fHistPosNSigmaSep[j]);
    fHistNegNSigmaSep[j] = new TH2F(Form("fHistNegNSigmaSep%d",j),"",hnbins,hxbins,1000,-10,10);
    fOutput->Add(fHistNegNSigmaSep[j]);
    
    fHistPrimMCpos[j] = new TH1F(Form("fHistPrimMCpos%d",j),Form("fHistPrimMCpos%d",j),kNbins,fPtBinLimits);
    fHistPrimMCneg[j] = new TH1F(Form("fHistPrimMCneg%d",j),Form("fHistPrimMCneg%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistPrimMCneg[j]);
    fOutput->Add(fHistPrimMCpos[j]);
    fHistSecStrMCpos[j] = new TH1F(Form("fHistSecStrMCpos%d",j),Form("fHistSecStrMCpos%d",j),kNbins,fPtBinLimits);
    fHistSecStrMCneg[j] = new TH1F(Form("fHistSecStrMCneg%d",j),Form("fHistSecStrMCneg%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecStrMCneg[j]);
    fOutput->Add(fHistSecStrMCpos[j]);
    fHistSecMatMCpos[j] = new TH1F(Form("fHistSecMatMCpos%d",j),Form("fHistSecMatMCpos%d",j),kNbins,fPtBinLimits);
    fHistSecMatMCneg[j] = new TH1F(Form("fHistSecMatMCneg%d",j),Form("fHistSecMatMCneg%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecMatMCneg[j]);
    fOutput->Add(fHistSecMatMCpos[j]);
    //
    fHistPrimMCposBefEvSel[j] = new TH1F(Form("fHistPrimMCposBefEvSel%d",j),Form("fHistPrimMCposBefEvSel%d",j),kNbins,fPtBinLimits);
    fHistPrimMCposBefEvSelEta[j] = new TH1F(Form("fHistPrimMCposBefEvSelEta%d",j),Form("fHistPrimMCposBefEvSelEta%d",j),kNbins,fPtBinLimits);
    fHistPrimMCposBefEvSelEtaY[j] = new TH1F(Form("fHistPrimMCposBefEvSelEtaY%d",j),Form("fHistPrimMCposBefEvSelEtaY%d",j),kNbins,fPtBinLimits);
    fHistPrimMCnegBefEvSel[j] = new TH1F(Form("fHistPrimMCnegBefEvSel%d",j),Form("fHistPrimMCnegBefEvSel%d",j),kNbins,fPtBinLimits);
    fHistPrimMCnegBefEvSelEta[j] = new TH1F(Form("fHistPrimMCnegBefEvSelEta%d",j),Form("fHistPrimMCnegBefEvSelEta%d",j),kNbins,fPtBinLimits);
    fHistPrimMCnegBefEvSelEtaY[j] = new TH1F(Form("fHistPrimMCnegBefEvSelEtaY%d",j),Form("fHistPrimMCnegBefEvSelEtaY%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistPrimMCposBefEvSel[j]);
    fOutput->Add(fHistPrimMCposBefEvSelEta[j]);
    fOutput->Add(fHistPrimMCposBefEvSelEtaY[j]);
    fOutput->Add(fHistPrimMCnegBefEvSel[j]);
    fOutput->Add(fHistPrimMCnegBefEvSelEta[j]);
    fOutput->Add(fHistPrimMCnegBefEvSelEtaY[j]);
    fHistSecStrMCposBefEvSel[j] = new TH1F(Form("fHistSecStrMCposBefEvSel%d",j),Form("fHistSecStrMCposBefEvSel%d",j),kNbins,fPtBinLimits);
    fHistSecStrMCnegBefEvSel[j] = new TH1F(Form("fHistSecStrMCnegBefEvSel%d",j),Form("fHistSecStrMCnegBefEvSel%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecStrMCnegBefEvSel[j]);
    fOutput->Add(fHistSecStrMCposBefEvSel[j]);
    fHistSecMatMCposBefEvSel[j] = new TH1F(Form("fHistSecMatMCposBefEvSel%d",j),Form("fHistSecMatMCposBefEvSel%d",j),kNbins,fPtBinLimits);
    fHistSecMatMCnegBefEvSel[j] = new TH1F(Form("fHistSecMatMCnegBefEvSel%d",j),Form("fHistSecMatMCnegBefEvSel%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecMatMCnegBefEvSel[j]);
    fOutput->Add(fHistSecMatMCposBefEvSel[j]);
    //
    fHistPrimMCposReco[j] = new TH1F(Form("fHistPrimMCposReco%d",j),Form("fHistPrimMCposReco%d",j),kNbins,fPtBinLimits);
    fHistPrimMCposRecoEtaY[j] = new TH1F(Form("fHistPrimMCposRecoEtaY%d",j),Form("fHistPrimMCposRecoEtaY%d",j),kNbins,fPtBinLimits);
    fHistPrimMCnegReco[j] = new TH1F(Form("fHistPrimMCnegReco%d",j),Form("fHistPrimMCnegReco%d",j),kNbins,fPtBinLimits);
    fHistPrimMCnegRecoEtaY[j] = new TH1F(Form("fHistPrimMCnegRecoEtaY%d",j),Form("fHistPrimMCnegRecoEtaY%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistPrimMCposReco[j]);
    fOutput->Add(fHistPrimMCposRecoEtaY[j]);
    fOutput->Add(fHistPrimMCnegReco[j]);
    fOutput->Add(fHistPrimMCnegRecoEtaY[j]);
    fHistSecStrMCposReco[j] = new TH1F(Form("fHistSecStrMCposReco%d",j),Form("fHistSecStrMCposReco%d",j),kNbins,fPtBinLimits);
    fHistSecStrMCnegReco[j] = new TH1F(Form("fHistSecStrMCnegReco%d",j),Form("fHistSecStrMCnegReco%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecStrMCnegReco[j]);
    fOutput->Add(fHistSecStrMCposReco[j]);
    fHistSecMatMCposReco[j] = new TH1F(Form("fHistSecMatMCposReco%d",j),Form("fHistSecMatMCposReco%d",j),kNbins,fPtBinLimits);
    fHistSecMatMCnegReco[j] = new TH1F(Form("fHistSecMatMCnegReco%d",j),Form("fHistSecMatMCnegReco%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistSecMatMCnegReco[j]);
    fOutput->Add(fHistSecMatMCposReco[j]);

  }
  
  for(Int_t i=0; i<4; i++){
    fHistCharge[i] = new TH1F(Form("fHistChargeLay%d",i),Form("fHistChargeLay%d",i),100,0,300);
    fOutput->Add(fHistCharge[i]);
  }
  
  for(Int_t i=0; i<kNbins; i++){
    fHistPosPi[i] = new TH1F(Form("fHistPosPi%d",i),Form("fHistPosPi%d",i),175,-3.5,3.5);	
    fHistPosK[i]  = new TH1F(Form("fHistPosK%d",i),Form("fHistPosK%d",i),175,-3.5,3.5);	
    fHistPosP[i]  = new TH1F(Form("fHistPosP%d",i),Form("fHistPosP%d",i),175,-3.5,3.5);	
    fHistNegPi[i] = new TH1F(Form("fHistNegPi%d",i),Form("fHistNegPi%d",i),175,-3.5,3.5);	
    fHistNegK[i]  = new TH1F(Form("fHistNegK%d",i),Form("fHistNegK%d",i),175,-3.5,3.5);	
    fHistNegP[i]  = new TH1F(Form("fHistNegP%d",i),Form("fHistNegP%d",i),175,-3.5,3.5);	
    
    fHistDCAPosPi[i] = new TH1F(Form("fHistDCAPosPi%d",i),Form("fHistDCAPosPi%d",i),2000,-1,1);  //DCA distr. with NSigma PID
    fHistDCAPosK[i]  = new TH1F(Form("fHistDCAPosK%d",i),Form("fHistDCAPosK%d",i),2000,-1,1);	
    fHistDCAPosP[i]  = new TH1F(Form("fHistDCAPosP%d",i),Form("fHistDCAPosP%d",i),2000,-1,1);	
    fHistDCANegPi[i] = new TH1F(Form("fHistDCANegPi%d",i),Form("fHistDCANegPi%d",i),2000,-1,1);	
    fHistDCANegK[i]  = new TH1F(Form("fHistDCANegK%d",i),Form("fHistDCANegK%d",i),2000,-1,1);	
    fHistDCANegP[i]  = new TH1F(Form("fHistDCANegP%d",i),Form("fHistDCANegP%d",i),2000,-1,1);	
    
    fHistMCPrimDCAPosPi[i] = new TH1F(Form("fHistMCPrimDCAPosPi%d",i),Form("fHistMCPrimDCAPosPi%d",i),2000,-1,1);  //DCA distr. with MC truth
    fHistMCPrimDCAPosK[i]  = new TH1F(Form("fHistMCPrimDCAPosK%d",i),Form("fHistMCPrimDCAPosK%d",i),2000,-1,1);	
    fHistMCPrimDCAPosP[i]  = new TH1F(Form("fHistMCPrimDCAPosP%d",i),Form("fHistMCPrimDCAPosP%d",i),2000,-1,1);	
    fHistMCPrimDCANegPi[i] = new TH1F(Form("fHistMCPrimDCANegPi%d",i),Form("fHistMCPrimDCANegPi%d",i),2000,-1,1);	
    fHistMCPrimDCANegK[i]  = new TH1F(Form("fHistMCPrimDCANegK%d",i),Form("fHistMCPrimDCANegK%d",i),2000,-1,1);	
    fHistMCPrimDCANegP[i]  = new TH1F(Form("fHistMCPrimDCANegP%d",i),Form("fHistMCPrimDCANegP%d",i),2000,-1,1);	
    
    fHistMCSecStDCAPosPi[i] = new TH1F(Form("fHistMCSecStDCAPosPi%d",i),Form("fHistMCSecStDCAPosPi%d",i),2000,-1,1);  //DCA distr. with MC truth
    fHistMCSecStDCAPosK[i]  = new TH1F(Form("fHistMCSecStDCAPosK%d",i),Form("fHistMCSecStDCAPosK%d",i),2000,-1,1);	
    fHistMCSecStDCAPosP[i]  = new TH1F(Form("fHistMCSecStDCAPosP%d",i),Form("fHistMCSecStDCAPosP%d",i),2000,-1,1);	
    fHistMCSecStDCANegPi[i] = new TH1F(Form("fHistMCSecStDCANegPi%d",i),Form("fHistMCSecStDCANegPi%d",i),2000,-1,1);	
    fHistMCSecStDCANegK[i]  = new TH1F(Form("fHistMCSecStDCANegK%d",i),Form("fHistMCSecStDCANegK%d",i),2000,-1,1);	
    fHistMCSecStDCANegP[i]  = new TH1F(Form("fHistMCSecStDCANegP%d",i),Form("fHistMCSecStDCANegP%d",i),2000,-1,1);	
    
    fHistMCSecMatDCAPosPi[i] = new TH1F(Form("fHistMCSecMatDCAPosPi%d",i),Form("fHistMCSecMatDCAPosPi%d",i),2000,-1,1);  //DCA distr. with MC truth
    fHistMCSecMatDCAPosK[i]  = new TH1F(Form("fHistMCSecMatDCAPosK%d",i),Form("fHistMCSecMatDCAPosK%d",i),2000,-1,1);	
    fHistMCSecMatDCAPosP[i]  = new TH1F(Form("fHistMCSecMatDCAPosP%d",i),Form("fHistMCSecMatDCAPosP%d",i),2000,-1,1);	
    fHistMCSecMatDCANegPi[i] = new TH1F(Form("fHistMCSecMatDCANegPi%d",i),Form("fHistMCSecMatDCANegPi%d",i),2000,-1,1);	
    fHistMCSecMatDCANegK[i]  = new TH1F(Form("fHistMCSecMatDCANegK%d",i),Form("fHistMCSecMatDCANegK%d",i),2000,-1,1);	
    fHistMCSecMatDCANegP[i]  = new TH1F(Form("fHistMCSecMatDCANegP%d",i),Form("fHistMCSecMatDCANegP%d",i),2000,-1,1);	
    
    fHistMCPosOtherHypPion[i] = new TH1F(Form("fHistMCPosOtherHypPion%d",i),Form("fHistMCPosOtherHypPion%d",i),175,-3.5,3.5);	//MC truth
    fHistMCPosOtherHypKaon[i] = new TH1F(Form("fHistMCPosOtherHypKaon%d",i),Form("fHistMCPosOtherHypKaon%d",i),175,-3.5,3.5);
    fHistMCPosOtherHypProton[i] = new TH1F(Form("fHistMCPosOtherHypProton%d",i),Form("fHistMCPosOtherHypProton%d",i),175,-3.5,3.5);
    fHistMCPosElHypPion[i] = new TH1F(Form("fHistMCPosElHypPion%d",i),Form("fHistMCPosElHypPion%d",i),175,-3.5,3.5);	
    fHistMCPosElHypKaon[i] = new TH1F(Form("fHistMCPosElHypKaon%d",i),Form("fHistMCPosElHypKaon%d",i),175,-3.5,3.5);
    fHistMCPosElHypProton[i] = new TH1F(Form("fHistMCPosElHypProton%d",i),Form("fHistMCPosElHypProton%d",i),175,-3.5,3.5);
    fHistMCPosPiHypPion[i] = new TH1F(Form("fHistMCPosPiHypPion%d",i),Form("fHistMCPosPiHypPion%d",i),175,-3.5,3.5);	
    fHistMCPosPiHypKaon[i] = new TH1F(Form("fHistMCPosPiHypKaon%d",i),Form("fHistMCPosPiHypKaon%d",i),175,-3.5,3.5);
    fHistMCPosPiHypProton[i] = new TH1F(Form("fHistMCPosPiHypProton%d",i),Form("fHistMCPosPiHypProton%d",i),175,-3.5,3.5);
    fHistMCPosKHypPion[i] = new TH1F(Form("fHistMCPosKHypPion%d",i),Form("fHistMCPosKHypPion%d",i),175,-3.5,3.5);	
    fHistMCPosKHypKaon[i]  = new TH1F(Form("fHistMCPosKHypKaon%d",i),Form("fHistMCPosKHypKaon%d",i),175,-3.5,3.5);	
    fHistMCPosKHypProton[i] = new TH1F(Form("fHistMCPosKHypProton%d",i),Form("fHistMCPosKHypProton%d",i),175,-3.5,3.5);
    fHistMCPosPHypPion[i] = new TH1F(Form("fHistMCPosPHypPion%d",i),Form("fHistMCPosPHypPion%d",i),175,-3.5,3.5);	
    fHistMCPosPHypKaon[i] = new TH1F(Form("fHistMCPosPHypKaon%d",i),Form("fHistMCPosPHypKaon%d",i),175,-3.5,3.5);
    fHistMCPosPHypProton[i]  = new TH1F(Form("fHistMCPosPHypProton%d",i),Form("fHistMCPosPHypProton%d",i),175,-3.5,3.5);	
    
    fHistMCNegOtherHypPion[i] = new TH1F(Form("fHistMCNegOtherHypPion%d",i),Form("fHistMCNegOtherHypPion%d",i),175,-3.5,3.5);	//MC truth
    fHistMCNegOtherHypKaon[i] = new TH1F(Form("fHistMCNegOtherHypKaon%d",i),Form("fHistMCNegOtherHypKaon%d",i),175,-3.5,3.5);
    fHistMCNegOtherHypProton[i] = new TH1F(Form("fHistMCNegOtherHypProton%d",i),Form("fHistMCNegOtherHypProton%d",i),175,-3.5,3.5);
    fHistMCNegElHypPion[i] = new TH1F(Form("fHistMCNegElHypPion%d",i),Form("fHistMCNegElHypPion%d",i),175,-3.5,3.5);
    fHistMCNegElHypKaon[i] = new TH1F(Form("fHistMCNegElHypKaon%d",i),Form("fHistMCNegElHypKaon%d",i),175,-3.5,3.5);
    fHistMCNegElHypProton[i] = new TH1F(Form("fHistMCNegElHypProton%d",i),Form("fHistMCNegElHypProton%d",i),175,-3.5,3.5);
    fHistMCNegPiHypPion[i] = new TH1F(Form("fHistMCNegPiHypPion%d",i),Form("fHistMCNegPiHypPion%d",i),175,-3.5,3.5);
    fHistMCNegPiHypKaon[i] = new TH1F(Form("fHistMCNegPiHypKaon%d",i),Form("fHistMCNegPiHypKaon%d",i),175,-3.5,3.5);
    fHistMCNegPiHypProton[i] = new TH1F(Form("fHistMCNegPiHypProton%d",i),Form("fHistMCNegPiHypProton%d",i),175,-3.5,3.5);
    fHistMCNegKHypPion[i] = new TH1F(Form("fHistMCNegKHypPion%d",i),Form("fHistMCNegKHypPion%d",i),175,-3.5,3.5);	
    fHistMCNegKHypKaon[i]  = new TH1F(Form("fHistMCNegKHypKaon%d",i),Form("fHistMCNegKHypKaon%d",i),175,-3.5,3.5);	
    fHistMCNegKHypProton[i] = new TH1F(Form("fHistMCNegKHypProton%d",i),Form("fHistMCNegKHypProton%d",i),175,-3.5,3.5);
    fHistMCNegPHypPion[i] = new TH1F(Form("fHistMCNegPHypPion%d",i),Form("fHistMCNegPHypPion%d",i),175,-3.5,3.5);	
    fHistMCNegPHypKaon[i] = new TH1F(Form("fHistMCNegPHypKaon%d",i),Form("fHistMCNegPHypKaon%d",i),175,-3.5,3.5);
    fHistMCNegPHypProton[i]  = new TH1F(Form("fHistMCNegPHypProton%d",i),Form("fHistMCNegPHypProton%d",i),175,-3.5,3.5);	
    
    
    fOutput->Add(fHistPosPi[i]);
    fOutput->Add(fHistPosK[i]);
    fOutput->Add(fHistPosP[i]);
    fOutput->Add(fHistNegPi[i]);
    fOutput->Add(fHistNegK[i]);
    fOutput->Add(fHistNegP[i]);
    
    fOutput->Add(fHistDCAPosPi[i]); //DCA distr
    fOutput->Add(fHistDCAPosK[i]);
    fOutput->Add(fHistDCAPosP[i]);
    fOutput->Add(fHistDCANegPi[i]);
    fOutput->Add(fHistDCANegK[i]);
    fOutput->Add(fHistDCANegP[i]);
    
    fOutput->Add(fHistMCPrimDCAPosPi[i]);//DCA distr.
    fOutput->Add(fHistMCPrimDCAPosK[i]);
    fOutput->Add(fHistMCPrimDCAPosP[i]);
    fOutput->Add(fHistMCPrimDCANegPi[i]);
    fOutput->Add(fHistMCPrimDCANegK[i]);
    fOutput->Add(fHistMCPrimDCANegP[i]);

    fOutput->Add(fHistMCSecStDCAPosPi[i]);//DCA distr.
    fOutput->Add(fHistMCSecStDCAPosK[i]);
    fOutput->Add(fHistMCSecStDCAPosP[i]);
    fOutput->Add(fHistMCSecStDCANegPi[i]);
    fOutput->Add(fHistMCSecStDCANegK[i]);
    fOutput->Add(fHistMCSecStDCANegP[i]);

    fOutput->Add(fHistMCSecMatDCAPosPi[i]);//DCA distr.
    fOutput->Add(fHistMCSecMatDCAPosK[i]);
    fOutput->Add(fHistMCSecMatDCAPosP[i]);
    fOutput->Add(fHistMCSecMatDCANegPi[i]);
    fOutput->Add(fHistMCSecMatDCANegK[i]);
    fOutput->Add(fHistMCSecMatDCANegP[i]);

    fOutput->Add(fHistMCPosOtherHypPion[i]);//MC truth
    fOutput->Add(fHistMCPosOtherHypKaon[i]);
    fOutput->Add(fHistMCPosOtherHypProton[i]);
    fOutput->Add(fHistMCPosElHypPion[i]);
    fOutput->Add(fHistMCPosElHypKaon[i]);
    fOutput->Add(fHistMCPosElHypProton[i]);
    fOutput->Add(fHistMCPosPiHypPion[i]);
    fOutput->Add(fHistMCPosPiHypKaon[i]);
    fOutput->Add(fHistMCPosPiHypProton[i]);
    fOutput->Add(fHistMCPosKHypPion[i]);
    fOutput->Add(fHistMCPosKHypKaon[i]);
    fOutput->Add(fHistMCPosKHypProton[i]);
    fOutput->Add(fHistMCPosPHypPion[i]);
    fOutput->Add(fHistMCPosPHypKaon[i]);
    fOutput->Add(fHistMCPosPHypProton[i]);
    
    fOutput->Add(fHistMCNegOtherHypPion[i]);//MC truth
    fOutput->Add(fHistMCNegOtherHypKaon[i]);
    fOutput->Add(fHistMCNegOtherHypProton[i]);
    fOutput->Add(fHistMCNegElHypPion[i]);
    fOutput->Add(fHistMCNegElHypKaon[i]);
    fOutput->Add(fHistMCNegElHypProton[i]);
    fOutput->Add(fHistMCNegPiHypPion[i]);
    fOutput->Add(fHistMCNegPiHypKaon[i]);
    fOutput->Add(fHistMCNegPiHypProton[i]);
    fOutput->Add(fHistMCNegKHypPion[i]);
    fOutput->Add(fHistMCNegKHypKaon[i]);
    fOutput->Add(fHistMCNegKHypProton[i]);
    fOutput->Add(fHistMCNegPHypPion[i]);
    fOutput->Add(fHistMCNegPHypKaon[i]);
    fOutput->Add(fHistMCNegPHypProton[i]);
    
  }
  
  //NSigma Histos
  for(Int_t j=0;j<3;j++){
    
    fHistPosNSigmaMean[j] = new TH1F(Form("hHistPosNSigmaMean%d",j),Form("hHistPosNSigmaMean%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaMean[j] = new TH1F(Form("hHistNegNSigmaMean%d",j),Form("hHistNegNSigmaMean%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaMCMean[j] = new TH1F(Form("hHistPosNSigmaMCMean%d",j),Form("hHistPosNSigmaMCMean%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaMCMean[j] = new TH1F(Form("hHistNegNSigmaMCMean%d",j),Form("hHistNegNSigmaMCMean%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaPrimMean[j] = new TH1F(Form("hHistPosNSigmaPrimMean%d",j),Form("hHistPosNSigmaPrimMean%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaPrimMean[j] = new TH1F(Form("hHistNegNSigmaPrimMean%d",j),Form("hHistNegNSigmaPrimMean%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaPrimMCMean[j] = new TH1F(Form("hHistPosNSigmaPrimMCMean%d",j),Form("hHistPosNSigmaPrimMCMean%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaPrimMCMean[j] = new TH1F(Form("hHistNegNSigmaPrimMCMean%d",j),Form("hHistNegNSigmaPrimMCMean%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistPosNSigmaMean[j]);
    fOutput->Add(fHistNegNSigmaMean[j]);
    fOutput->Add(fHistPosNSigmaMCMean[j]);
    fOutput->Add(fHistNegNSigmaMCMean[j]);
    fOutput->Add(fHistPosNSigmaPrimMean[j]);
    fOutput->Add(fHistNegNSigmaPrimMean[j]);
    fOutput->Add(fHistPosNSigmaPrimMCMean[j]);
    fOutput->Add(fHistNegNSigmaPrimMCMean[j]);

    fHistPosNSigma[j] = new TH1F(Form("hHistPosNSigma%d",j),Form("hHistPosNSigma%d",j),kNbins,fPtBinLimits);
    fHistNegNSigma[j] = new TH1F(Form("hHistNegNSigma%d",j),Form("hHistNegNSigma%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaMC[j] = new TH1F(Form("hHistPosNSigmaMC%d",j),Form("hHistPosNSigmaMC%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaMC[j] = new TH1F(Form("hHistNegNSigmaMC%d",j),Form("hHistNegNSigmaMC%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaPrim[j] = new TH1F(Form("hHistPosNSigmaPrim%d",j),Form("hHistPosNSigmaPrim%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaPrim[j] = new TH1F(Form("hHistNegNSigmaPrim%d",j),Form("hHistNegNSigmaPrim%d",j),kNbins,fPtBinLimits);
    fHistPosNSigmaPrimMC[j] = new TH1F(Form("hHistPosNSigmaPrimMC%d",j),Form("hHistPosNSigmaPrimMC%d",j),kNbins,fPtBinLimits);
    fHistNegNSigmaPrimMC[j] = new TH1F(Form("hHistNegNSigmaPrimMC%d",j),Form("hHistNegNSigmaPrimMC%d",j),kNbins,fPtBinLimits);
    fOutput->Add(fHistPosNSigma[j]);
    fOutput->Add(fHistNegNSigma[j]);
    fOutput->Add(fHistPosNSigmaMC[j]);
    fOutput->Add(fHistNegNSigmaMC[j]);
    fOutput->Add(fHistPosNSigmaPrim[j]);
    fOutput->Add(fHistNegNSigmaPrim[j]);
    fOutput->Add(fHistPosNSigmaPrimMC[j]);
    fOutput->Add(fHistNegNSigmaPrimMC[j]);

  }
  
  fNtupleNSigma = new TNtuple("fNtupleNSigma","fNtupleNSigma","p:pt:dedx:dedx3:dedx4:dedx5:dedx6:ncls:nclspid:sign:run:eta:impactXY:impactZ:isph:pdgcode:mfl:uniqueID:chi2ncls");
  fOutput->Add(fNtupleNSigma);
  fNtupleMC = new TNtuple("fNtupleMC","fNtupleMC","ptMC:pdgcode:signMC:etaMC:yMC:isph:evSel:run");
  fOutput->Add(fNtupleMC);

  
 
  // Post output data.
  PostData(1,fOutput);

  Printf("end of CreateOutputObjects");
}

//________________________________________________________________________
void AliAnalysisTaskSEITSsaSpectra::UserExec(Option_t *){
  // Main loop
  // Called for each event
  
  ///////////////////////////////////////
  //variables
  Float_t pdgmass[4]={0.13957,0.493677,0.938272,1.8756}; //mass for pi, K, P (Gev/c^2)
  Int_t listcode[3]={211,321,2212};//code for pi, K, P (Gev/c^2)
  Double_t dedxLay[4];
  Float_t ptMC=-999;
  Int_t code=-999, signMC=-999,isph=-999,mfl=-999,uniqueID=-999;
  Float_t impactXY=-999, impactZ=-999;
  Int_t evSel=1;
  AliESDtrack* track;
  UInt_t status; 
  AliStack* stack=0;
  TParticle *part=0;
  TParticlePDG *pdgPart=0;
	
  /////////////////////

  fESD=(AliESDEvent*)InputEvent();
  if(!fESD) {
    printf("AliAnalysisTaskSDDRP::Exec(): bad ESD\n");
    return;
  } 
  fHistNEvents->Fill(-1);
  
  AliBits maskPhysSel = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
  TString firedTriggerClasses=fESD->GetFiredTriggerClasses();
  //  if(!firedTriggerClasses.Contains("CINT1B")) return;
  if((maskPhysSel & AliTrigger::kMB)==0) return;
  fHistNEvents->Fill(0);

  if(fLowEnergypp && !fMC){ // remove events without SDD in pp 2.76 TeV
    if(!firedTriggerClasses.Contains("ALL")) return;
  }
  fHistNEvents->Fill(1);


  if(fMC){
    AliMCEventHandler* eventHandler = dynamic_cast<AliMCEventHandler*> (AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler());
    if (!eventHandler) {
      Printf("ERROR: Could not retrieve MC event handler");
      return;
    }
    AliMCEvent* mcEvent = eventHandler->MCEvent();
    if (!mcEvent) {
      Printf("ERROR: Could not retrieve MC event");
      return;
    }
    stack = mcEvent->Stack();
    if (!stack) {
      printf("ERROR: stack not available\n");
      return;
    }
  }
  if(!fITSPIDResponse){
    fITSPIDResponse=new AliITSPIDResponse(fMC); 
  }

  //flags for MC
  Int_t nTrackMC=0; 
  if(stack) nTrackMC = stack->GetNtrack();	
  const AliESDVertex *vtx =  fESD->GetPrimaryVertexSPD();

  ///////////selection of the centrality or multiplicity bin

  //selection on the event centrality
  if(fHImode){
    if(!(fLowCentrality<0.0)&&fUpCentrality>0.0)
      {
	AliCentrality *centrality = fESD->GetCentrality();
	if(!centrality->IsEventInCentralityClass(fLowCentrality,fUpCentrality,"V0M"))
	  return;
	Printf("Centrality of the event: %.1f",centrality->GetCentralityPercentile("V0M"));
	Printf("Centrality cut: %.1f to %.1f",fLowCentrality,fUpCentrality);
	fHistCen->Fill(centrality->GetCentralityPercentile("V0M"));
      }
  }
  
  //selection on the event multiplicity based on global tracks
  Int_t multiplicity = -1;  
  if(fMultEstimator==0){
    // tracks+tracklets
    multiplicity=AliESDtrackCuts::GetReferenceMultiplicity(fESD, AliESDtrackCuts::kTrackletsITSTPC, 0.8); 
  }else if(fMultEstimator==1){
    // tracklets
    multiplicity=AliESDtrackCuts::GetReferenceMultiplicity(fESD, AliESDtrackCuts::kTracklets, 0.8);    
  }else if(fMultEstimator==2){
    // clusters in SPD1
    const AliMultiplicity *mult = fESD->GetMultiplicity();
    Float_t nClu1 = (Float_t)mult->GetNumberOfITSClusters(1);
    multiplicity =(Int_t)(AliESDUtils::GetCorrSPD2(nClu1,vtx->GetZ())+0.5);
  }
  if(fLowMult>-1 && multiplicity<fLowMult) return;
  if(fUpMult>-1 && multiplicity>fUpMult) return;
  fHistMult->Fill(multiplicity);
  fHistNEvents->Fill(2);
  
  //event selection
  if(!vtx)evSel=0;
  else{
    fHistNEvents->Fill(3);
    if(vtx->GetNContributors()<0) evSel=0;
    else{
      fHistNEvents->Fill(4);
      if(TMath::Abs(vtx->GetZ())>10) evSel=0;
      else{
	fHistNEvents->Fill(5);
	if(vtx->GetZRes()>0.5) evSel=0;
	else{
	  fHistNEvents->Fill(6);
	  if(vtx->IsFromVertexerZ() && vtx->GetDispersion()>0.03) evSel=0;
	  else fHistNEvents->Fill(7);
	}
      }
    }
  }


  /////first loop on stack, before event selection, filling MC ntuple
  
  for(Int_t imc=0; imc<nTrackMC; imc++){
    part = stack->Particle(imc);
    isph=1;    
    if(!stack->IsPhysicalPrimary(imc)) isph=0;
    pdgPart = part->GetPDG();
    if(!pdgPart)continue;
    if(pdgPart->Charge()==0) continue; //no neutral particles
    Float_t yMC=-999.;
    if(part->Energy() != TMath::Abs(part->Pz())) yMC = 0.5*TMath::Log((part->Energy()+part->Pz())/(part->Energy()-part->Pz()));
    if(pdgPart->Charge()>0) signMC=1;
    else signMC=-1;
    ptMC=part->Pt();
    code=pdgPart->PdgCode();
    Int_t jpart=-1;
    for(Int_t j=0; j<3; j++){
      if(TMath::Abs(code)==listcode[j]){
	jpart=j;
	break;
      }
    }
    
    if(jpart>=0 && stack->IsPhysicalPrimary(imc) && TMath::Abs(part->Eta())< fEtaRange){
      if(signMC>0) fHistPrimMCposBefEvSelEta[jpart]->Fill(ptMC);
      else  fHistPrimMCnegBefEvSelEta[jpart]->Fill(ptMC);	    
    }
    
    if(TMath::Abs(yMC) > fMaxY) continue; //rapidity cut
    
    Int_t indexMoth=part->GetFirstMother();
    if(indexMoth>=0){
      TParticle* moth = stack->Particle(indexMoth);
      Float_t codemoth = TMath::Abs(moth->GetPdgCode());
      mfl = Int_t (codemoth/ TMath::Power(10, Int_t(TMath::Log10(codemoth))));
    }
    uniqueID = part->GetUniqueID();
    
    //filling MC ntuple
    if(TMath::Abs(code)==211 || TMath::Abs(code)==321 || TMath::Abs(code)==2212){
      Float_t xntMC[8];
      Int_t indexMC=0;
      xntMC[indexMC++]=(Float_t)ptMC;
      xntMC[indexMC++]=(Float_t)code;
      xntMC[indexMC++]=(Float_t)signMC;
      xntMC[indexMC++]=(Float_t)part->Eta();
      xntMC[indexMC++]=(Float_t)yMC;
      xntMC[indexMC++]=(Float_t)isph;
      xntMC[indexMC++]=(Float_t)evSel;
      xntMC[indexMC++]=(Float_t)fESD->GetRunNumber();
      
      if(fFillNtuple) fNtupleMC->Fill(xntMC);
    }
    
    if(jpart>=0){
      if(stack->IsPhysicalPrimary(imc)){
	if(signMC>0){
	  fHistPrimMCposBefEvSel[jpart]->Fill(ptMC);
	  if(TMath::Abs(part->Eta())<fEtaRange) fHistPrimMCposBefEvSelEtaY[jpart]->Fill(ptMC);
	}
	else{
	  fHistPrimMCnegBefEvSel[jpart]->Fill(ptMC);
	  if(TMath::Abs(part->Eta())<fEtaRange) fHistPrimMCnegBefEvSelEtaY[jpart]->Fill(ptMC);
	}
	if(evSel==1){
	  if(signMC>0) fHistPrimMCpos[jpart]->Fill(ptMC);
	  else  fHistPrimMCneg[jpart]->Fill(ptMC);
	}
      }else{
	if(mfl==3 && uniqueID == kPDecay){ // If a particle is not a physical primary, check if it comes from weak decay
	  if(signMC>0) fHistSecStrMCposBefEvSel[jpart]->Fill(ptMC);
	  else  fHistSecStrMCnegBefEvSel[jpart]->Fill(ptMC);	    
	  if(evSel==1){
	    if(signMC>0) fHistSecStrMCpos[jpart]->Fill(ptMC);
	    else  fHistSecStrMCneg[jpart]->Fill(ptMC);	    
	  }
	}else{
	  if(signMC>0) fHistSecMatMCposBefEvSel[jpart]->Fill(ptMC);
	  else  fHistSecMatMCnegBefEvSel[jpart]->Fill(ptMC);	    
	  if(evSel==1){
	    if(signMC>0) fHistSecMatMCpos[jpart]->Fill(ptMC);
	    else  fHistSecMatMCneg[jpart]->Fill(ptMC);	    
	  }
	}
      }
    }
  }	
  
  
  if(evSel==0)return;  //event selection
  

  //loop on tracks
  for (Int_t iTrack=0; iTrack<fESD->GetNumberOfTracks(); iTrack++) {  
    isph=-999;
    code=-999;
    mfl=-999;
    uniqueID=-999;
    
    track = (AliESDtrack*)fESD->GetTrack(iTrack);      
    if (!track) continue;
    
    //track selection
    Int_t countBinTrk=1;
    TString label;
    
    label="no selection";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    status=track->GetStatus();
    if((status&AliESDtrack::kITSpureSA)==0) continue; //its standalone
  
    label="ITSsa";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;    
    
    if((status&AliESDtrack::kITSrefit)==0) continue; //its refit
    
    label="ITSrefit";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;
    
    if(TMath::Abs(track->GetSign())<0.0001) continue; //no neutral particles
    
    label="neutral particle";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    //cluster in ITS
    UInt_t clumap = track->GetITSClusterMap();
    Int_t nSPD=0;
    for(Int_t il=0; il<2; il++) if(TESTBIT(clumap,il)) nSPD++;
    if(nSPD<fMinSPDPts) continue;
    
    label="SPDcls";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    Int_t nPtsForPid=0;
    for(Int_t j=2;j<6;j++) if(TESTBIT(clumap,j)) nPtsForPid++;
    if(nPtsForPid<fMinNdEdxSamples) continue; //at least 3 points on SSD/SDD
    
    label="SDD+SSD cls";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    //chisquare/nclusters	
    Int_t nclu=nSPD+nPtsForPid;
    if(track->GetITSchi2()/nclu > fMaxChi2Clu) continue; 
    
    label="chi2/ncls";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    //pseudorapidity and rapidity
    if(TMath::Abs(track->Eta()) > fEtaRange) continue;
    
    label="eta";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    //truncated mean
    track->GetITSdEdxSamples(dedxLay);
    Double_t dedx = CookdEdx(dedxLay);
    if(dedx<0) continue;

    label="de/dx<0";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  
    
    Float_t pt = track->Pt();
    Int_t theBin=-1;
    for(Int_t m=0; m<kNbins; m++){
      if(TMath::Abs(pt) > fPtBinLimits[m] && TMath::Abs(pt) < fPtBinLimits[m+1]){
	theBin=m;
	break;
      }
    }
    track->GetImpactParameters(impactXY, impactZ);
    
    //Filling Ntuple
    //information from the MC kinematics
    if(fMC){
      if(track->GetLabel()<0)isph=-1;
      if(track->GetLabel()>=0){
	part = (TParticle*)stack->Particle(track->GetLabel());
	pdgPart = part->GetPDG();
	code = pdgPart->PdgCode();
	if(stack->IsPhysicalPrimary(track->GetLabel())) isph=1;
	else{ 
	  isph=0;
	  Int_t indexMoth=part->GetFirstMother();
	  if(indexMoth>=0){
	    TParticle* moth = stack->Particle(indexMoth);
	    Float_t codemoth = TMath::Abs(moth->GetPdgCode());
	    mfl = Int_t (codemoth/ TMath::Power(10, Int_t(TMath::Log10(codemoth))));
	  }
	  uniqueID = part->GetUniqueID();
	}
	
	//Filling DCA distribution with MC truth
	
	if(theBin>=0 && theBin<kNbins){
	  if(isph==1){//primaries in MC
	    if(track->GetSign()>0){
	      if(TMath::Abs(code)==listcode[0]) fHistMCPrimDCAPosPi[theBin]->Fill(impactXY);
	      if(TMath::Abs(code)==listcode[1]) fHistMCPrimDCAPosK[theBin]->Fill(impactXY);
	      if(TMath::Abs(code)==listcode[2]) fHistMCPrimDCAPosP[theBin]->Fill(impactXY);
	    }else{
	      if(TMath::Abs(code)==listcode[0]) fHistMCPrimDCANegPi[theBin]->Fill(impactXY);
	      if(TMath::Abs(code)==listcode[1]) fHistMCPrimDCANegK[theBin]->Fill(impactXY);
	      if(TMath::Abs(code)==listcode[2]) fHistMCPrimDCANegP[theBin]->Fill(impactXY);
	    }
	  }
	  
	  if(isph==0){//primaries in MC
	    if(mfl==3 && uniqueID == kPDecay){
	      if(track->GetSign()>0){
		if(TMath::Abs(code)==listcode[0]) fHistMCSecStDCAPosPi[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[1]) fHistMCSecStDCAPosK[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[2]) fHistMCSecStDCAPosP[theBin]->Fill(impactXY);
	      }else{
		if(TMath::Abs(code)==listcode[0]) fHistMCSecStDCANegPi[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[1]) fHistMCSecStDCANegK[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[2]) fHistMCSecStDCANegP[theBin]->Fill(impactXY);
	      }
	    }else{
	      if(track->GetSign()>0){
		if(TMath::Abs(code)==listcode[0]) fHistMCSecMatDCAPosPi[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[1]) fHistMCSecMatDCAPosK[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[2]) fHistMCSecMatDCAPosP[theBin]->Fill(impactXY);
	      }else{
		if(TMath::Abs(code)==listcode[0]) fHistMCSecMatDCANegPi[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[1]) fHistMCSecMatDCANegK[theBin]->Fill(impactXY);
		if(TMath::Abs(code)==listcode[2]) fHistMCSecMatDCANegP[theBin]->Fill(impactXY);
	      }
	    }
	  }
	}
      }
    }
    Float_t xnt[19];
    Int_t index=0;
    xnt[index++]=(Float_t)track->GetP();
    xnt[index++]=(Float_t)track->Pt();
    xnt[index++]=(Float_t)dedx;
    xnt[index++]=(Float_t)dedxLay[0];
    xnt[index++]=(Float_t)dedxLay[1];
    xnt[index++]=(Float_t)dedxLay[2];
    xnt[index++]=(Float_t)dedxLay[3];
    xnt[index++]=(Float_t)nclu;
    xnt[index++]=(Float_t)nPtsForPid;
    xnt[index++]=(Float_t)track->GetSign();
    xnt[index++]=(Float_t)fESD->GetRunNumber();
    xnt[index++]=(Float_t)track->Eta();
    xnt[index++]=(Float_t)impactXY;
    xnt[index++]=(Float_t)impactZ;
    xnt[index++]=(Float_t)isph;
    xnt[index++]=(Float_t)code;
    xnt[index++]=(Float_t)mfl;
    xnt[index++]=(Float_t)uniqueID;
    xnt[index]=(Float_t)track->GetITSchi2()/nclu;
	  
    if(fFillNtuple) fNtupleNSigma->Fill(xnt);
    
    
	
    //Compute y and bb
    Double_t y[4],bbtheo[4],logdiff[4];
    Float_t p=track->GetP();
    if(fMC && fSmearMC){
      dedx=fRandGener->Gaus(dedx,fSmeardEdx*dedx);
      p=fRandGener->Gaus(p,fSmearP*p);     
    }
    
    //Nsigma Method
    Float_t resodedx[4];
    for(Int_t ires=0;ires<4;ires++){
      resodedx[ires]=fITSPIDResponse->GetResolution(1,ires+1,kTRUE);
    }
    
    for(Int_t i=0;i<4;i++){
      y[i] = Eta2y(pt,pdgmass[i],track->Eta());
      //bbtheo[i]=fITSPIDResponse->Bethe(p,pdgmass[i],kTRUE); //Pure PHOBOS BB
      bbtheo[i]=fITSPIDResponse->BetheITSsaHybrid(p,pdgmass[i]); //PHOBOS + polinomial at low pt (below beta*gamma = 0.76)
      logdiff[i]=TMath::Log(dedx) - TMath::Log(bbtheo[i]);
    }
    
    Int_t resocls=(Int_t)nPtsForPid-1;
    
    //NSigma Method, with asymmetric bands
    Int_t minPosMean=-1;
    for(Int_t isp=0; isp<3; isp++){
      if(dedx<bbtheo[0])continue;
      Double_t bb=bbtheo[isp];
      if(dedx<bb){
	Double_t bbdistance=TMath::Abs((bbtheo[isp]-bbtheo[isp-1])/2);
	Double_t nsigma=TMath::Abs((dedx-bb)/bbdistance);
	if(nsigma<1.)minPosMean=isp;
      }
      else{
	Double_t bbdistance=TMath::Abs((bbtheo[isp]-bbtheo[isp+1])/2);
	Double_t nsigma=TMath::Abs((dedx-bb)/bbdistance);
	if(nsigma<1.)minPosMean=isp;
      }
    }
    if(dedx<bbtheo[0] && TMath::Abs((dedx-bbtheo[0])/(resodedx[resocls]*bbtheo[0]))<2)minPosMean=0;
    
    //NSigma method with simmetric bands
    
    Double_t nsigmas[3];
    Double_t min=999999.;
    Int_t minPos=-1;
    for(Int_t isp=0; isp<3; isp++){
      Double_t bb=bbtheo[isp];
      nsigmas[isp]=TMath::Abs((dedx-bb)/(resodedx[resocls]*bb));
      if(nsigmas[isp]<min){
	min=nsigmas[isp];
	minPos=isp;
      }
      //Filling histos with nsigma separation
      if(track->GetSign()>0)fHistPosNSigmaSep[isp]->Fill(track->GetP(),((dedx-bb)/(resodedx[resocls]*bb)));
      else fHistNegNSigmaSep[isp]->Fill(track->GetP(),((dedx-bb)/(resodedx[resocls]*bb)));
    }
    
    // y calculation
    Double_t yPartMean=-999;
    Double_t yPart=-999;
    if(minPosMean>-1) yPartMean=y[minPosMean];
    if(minPos>-1) yPart=y[minPos];
    
    if(TMath::Abs(yPartMean)<fMaxY){     
      //DCA distributions, before the DCA cuts, based on asymmetrinc nsigma approach
      if(theBin>=0 && theBin<kNbins){
	if(track->GetSign()>0){
	  if(minPosMean==0) fHistDCAPosPi[theBin]->Fill(impactXY);
	  else if(minPosMean==1) fHistDCAPosK[theBin]->Fill(impactXY);
	  else if(minPosMean==2) fHistDCAPosP[theBin]->Fill(impactXY);
	}else{
	  if(minPosMean==0) fHistDCANegPi[theBin]->Fill(impactXY);
	  else if(minPosMean==1) fHistDCANegK[theBin]->Fill(impactXY);
	  else if(minPosMean==2) fHistDCANegP[theBin]->Fill(impactXY);
	}
      } 
    }
    
    //DCA cut on xy and z
    if(!DCAcut(impactXY,impactZ,pt)) continue;
    
    label="DCA";
    fHistNTracks->Fill(countBinTrk);
    if(track->GetSign()>0)fHistNTracksPos->Fill(countBinTrk);
    if(track->GetSign()<0)fHistNTracksNeg->Fill(countBinTrk);
    fHistNTracks->GetXaxis()->SetBinLabel(fHistNTracks->FindBin(countBinTrk),label.Data());
    fHistNTracksPos->GetXaxis()->SetBinLabel(fHistNTracksPos->FindBin(countBinTrk),label.Data());
    fHistNTracksNeg->GetXaxis()->SetBinLabel(fHistNTracksNeg->FindBin(countBinTrk),label.Data());
    countBinTrk++;  


    Int_t jpart=-1;
    
    //Filling Histos for Reco Efficiency
    //information from the MC kinematics
    if(fMC){
      if(track->GetLabel()<0)isph=-1;
      if(track->GetLabel()>=0){
	part = (TParticle*)stack->Particle(track->GetLabel());
	pdgPart = part->GetPDG();
	code = pdgPart->PdgCode();
	for(Int_t j=0; j<3; j++){
	  if(TMath::Abs(code)==listcode[j]){
	    jpart=j;
	    break;
	  }
	}
	if(jpart>=0){
	  if(pdgPart->Charge()>0) signMC=1;
	  else signMC=-1;
	  ptMC=part->Pt();
	  if(stack->IsPhysicalPrimary(track->GetLabel())){
	    if(signMC>0){
	      fHistPrimMCposReco[jpart]->Fill(ptMC);
	      if(TMath::Abs(y[jpart])<fMaxY) fHistPrimMCposRecoEtaY[jpart]->Fill(ptMC);
	    }
	    else{
	      fHistPrimMCnegReco[jpart]->Fill(ptMC);
	      if(TMath::Abs(y[jpart])<fMaxY) fHistPrimMCnegRecoEtaY[jpart]->Fill(ptMC);
	    }
	  }else{ 
	    Int_t indexMoth=part->GetFirstMother();
	    if(indexMoth>=0){
	      TParticle* moth = stack->Particle(indexMoth);
	      Float_t codemoth = TMath::Abs(moth->GetPdgCode());
	      mfl = Int_t (codemoth/ TMath::Power(10, Int_t(TMath::Log10(codemoth))));
	    }
	    uniqueID = part->GetUniqueID();
	    if(mfl==3 && uniqueID == kPDecay){ // strangeness
	      if(signMC>0) fHistSecStrMCposReco[jpart]->Fill(ptMC);
	      else  fHistSecStrMCnegReco[jpart]->Fill(ptMC);	    
	    }else{
	      if(signMC>0) fHistSecMatMCposReco[jpart]->Fill(ptMC);
	      else  fHistSecMatMCnegReco[jpart]->Fill(ptMC);	    
	    }
	  }
	}
      }
    }
    
    //Nsigma histos with MC truth
    
    //asymmetric approach
    if(TMath::Abs(yPartMean)<fMaxY && minPosMean>-1){
      //nsigma histos
      if(track->GetSign()>0) fHistPosNSigmaMean[minPosMean]->Fill(pt);
      else fHistNegNSigmaMean[minPosMean]->Fill(pt);
      if(fMC){
	//nsigma histos with MC truth on PID
	if(TMath::Abs(code)==listcode[minPosMean]){
	  if(track->GetSign()>0) fHistPosNSigmaMCMean[minPosMean]->Fill(pt);
	  else fHistNegNSigmaMCMean[minPosMean]->Fill(pt);
	}
	//nsigma histos with MC truth on IsPhysicalPrimary
	if(isph==1){
	  if(track->GetSign()>0) fHistPosNSigmaPrimMean[minPosMean]->Fill(pt);
	  else fHistNegNSigmaPrimMean[minPosMean]->Fill(pt);
	  //nsigma histos with MC truth on IsPhysicalPrimary and PID
	  if(TMath::Abs(code)==listcode[minPosMean]){
	    if(track->GetSign()>0) fHistPosNSigmaPrimMCMean[minPosMean]->Fill(pt);
	    else fHistNegNSigmaPrimMCMean[minPosMean]->Fill(pt);
	  }
	}
      }
    }
    
    //symmetric bands
    if(min<fMinNSigma && TMath::Abs(yPart)<fMaxY){
      //nsigma histos
      if(track->GetSign()>0) fHistPosNSigma[minPos]->Fill(pt);
      else fHistNegNSigma[minPos]->Fill(pt);
      if(fMC){
	//nsigma histos with MC truth on PID
	if(TMath::Abs(code)==listcode[minPos]){
	  if(track->GetSign()>0) fHistPosNSigmaMC[minPos]->Fill(pt);
	  else fHistNegNSigmaMC[minPos]->Fill(pt);
	}
	//nsigma histos with MC truth on IsPhysicalPrimary
	if(isph==1){
	  if(track->GetSign()>0) fHistPosNSigmaPrim[minPos]->Fill(pt);
	  else fHistNegNSigmaPrim[minPos]->Fill(pt);
	  //nsigma histos with MC truth on IsPhysicalPrimary and PID
	  if(TMath::Abs(code)==listcode[minPos]){
	    if(track->GetSign()>0) fHistPosNSigmaPrimMC[minPos]->Fill(pt);
	    else fHistNegNSigmaPrimMC[minPos]->Fill(pt);
	  }
	}
      }
    }
    
    
    //integral approach histograms
    if(theBin>=0 && theBin<kNbins){
      if(track->GetSign()>0){
	if(TMath::Abs(y[0]) < fMaxY)fHistPosPi[theBin]->Fill(logdiff[0]);
	if(TMath::Abs(y[1]) < fMaxY)fHistPosK[theBin]->Fill(logdiff[1]);
	if(TMath::Abs(y[2]) < fMaxY)fHistPosP[theBin]->Fill(logdiff[2]);
	if(fMC){
	  if(TMath::Abs(y[0])<fMaxY){
	    if(TMath::Abs(code)!=11 && jpart<0)fHistMCPosOtherHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==11)fHistMCPosElHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==211)fHistMCPosPiHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==321)fHistMCPosKHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==2212)fHistMCPosPHypPion[theBin]->Fill(logdiff[0]);
	  }
	  if(TMath::Abs(y[1])<fMaxY){
	    if(TMath::Abs(code)!=11 && jpart<0)fHistMCPosOtherHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==11)fHistMCPosElHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==211)fHistMCPosPiHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==321)fHistMCPosKHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==2212)fHistMCPosPHypKaon[theBin]->Fill(logdiff[1]);
	  }
	  if(TMath::Abs(y[2])<fMaxY){
	      if(TMath::Abs(code)!=11 && jpart<0)fHistMCPosOtherHypProton[theBin]->Fill(logdiff[2]);
	      if(TMath::Abs(code)==11)fHistMCPosElHypProton[theBin]->Fill(logdiff[2]);
	      if(TMath::Abs(code)==211)fHistMCPosPiHypProton[theBin]->Fill(logdiff[2]);
	      if(TMath::Abs(code)==321)fHistMCPosKHypProton[theBin]->Fill(logdiff[2]);
	      if(TMath::Abs(code)==2212)fHistMCPosPHypProton[theBin]->Fill(logdiff[2]);
	  }
	}
      }else{
	if(TMath::Abs(y[0]) < fMaxY)fHistNegPi[theBin]->Fill(logdiff[0]);
	if(TMath::Abs(y[1]) < fMaxY)fHistNegK[theBin]->Fill(logdiff[1]);
	if(TMath::Abs(y[2]) < fMaxY)fHistNegP[theBin]->Fill(logdiff[2]);
	if(fMC){
	  if(TMath::Abs(y[0])<fMaxY){
	    if(TMath::Abs(code)!=11 && jpart<0)fHistMCNegOtherHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==11)fHistMCNegElHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==211)fHistMCNegPiHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==321)fHistMCNegKHypPion[theBin]->Fill(logdiff[0]);
	    if(TMath::Abs(code)==2212)fHistMCNegPHypPion[theBin]->Fill(logdiff[0]);
	  }
	  if(TMath::Abs(y[1])<fMaxY){
	    if(TMath::Abs(code)!=11 && jpart<0)fHistMCNegOtherHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==11)fHistMCNegElHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==211)fHistMCNegPiHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==321)fHistMCNegKHypKaon[theBin]->Fill(logdiff[1]);
	    if(TMath::Abs(code)==2212)fHistMCNegPHypKaon[theBin]->Fill(logdiff[1]);
	  }
	  if(TMath::Abs(y[2])<fMaxY){
	    if(TMath::Abs(code)!=11 && jpart<0)fHistMCNegOtherHypProton[theBin]->Fill(logdiff[2]);
	    if(TMath::Abs(code)==11)fHistMCNegElHypProton[theBin]->Fill(logdiff[2]);
	    if(TMath::Abs(code)==211)fHistMCNegPiHypProton[theBin]->Fill(logdiff[2]);
	    if(TMath::Abs(code)==321)fHistMCNegKHypProton[theBin]->Fill(logdiff[2]);
	    if(TMath::Abs(code)==2212)fHistMCNegPHypProton[theBin]->Fill(logdiff[2]);
	  }
	}
      }
    }							     
  
	  
    //fill propaganda plot with dedx
    fHistDEDX->Fill(track->GetP(),dedx);
    fHistDEDXdouble->Fill(track->GetP()*track->GetSign(),dedx);
	  
    //fill charge distribution histo to check the calibration
    for(Int_t j=0;j<4;j++){
      if(dedxLay[j]<5) continue;
      fHistCharge[j]->Fill(dedxLay[j]);
    }
  }
		
  // Post output data.
  PostData(1,fOutput);
  PostData(2,fListCuts);
  //  Printf("............. end of Exec");
}      

//________________________________________________________________________
void AliAnalysisTaskSEITSsaSpectra::Terminate(Option_t *) {
  // Merge output
  // Called once at the end of the query
  
  fOutput = dynamic_cast<TList*>(GetOutputData(1));
  if (!fOutput) {
    printf("ERROR: fOutput not available\n");
    return;
  }
  if(fHistNEvents){
    fHistNEvents = dynamic_cast<TH1F*>(fOutput->FindObject("fHistNEvents"));
    printf("Number of Analyzed Events = %f\n",fHistNEvents->GetBinContent(1));
  }else{
    printf("ERROR: fHistNEvents not available\n");
  }

  Printf("end of Terminate");
  return;
}

