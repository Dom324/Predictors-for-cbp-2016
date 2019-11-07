///////////////////////////////////////////////////////////////////////
////  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
/////////////////////////////////////////////////////////////////////////
//
#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
#include "bt9.h"
#include "bt9_reader.h"

#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 2

#define VECTOR_LEN  21
#define BIMODAL_LEN (VECTOR_LEN - 2)

#define HIST_LEN  32
#define SHFT_AMOUNT 2

//stats
  UINT64  NumMispred = 0, NumBranches = 0, Numpht1 = 0, Numpht2 = 0, Numbimodal = 0, pht1_Absolute_Mispred = 0, pht1_Relative_Mispred =0, pht2_Absolute_Mispred = 0,
		  pht2_Relative_Mispred = 0, bimodal_Absolute_Mispred = 0, bimodal_Relative_Mispred;
//stats

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT64  ghr = 0;           // global history register
  UINT32  *pht1;          // pattern history table
  UINT32  *pht2;          // pattern history table
  UINT32  *bimodal;		  // bimodal table
  UINT32  numPhtEntries; // entries in pht 
  UINT32  numBimodalEntries;

 public:
 

  PREDICTOR(void);

  // The interface to the functions below CAN NOT be changed
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  

  // Contestants can define their own functions below
};

  void    PrintStat();


PREDICTOR::PREDICTOR(void){
	
  ghr              = 0;
  numPhtEntries    = (1 << VECTOR_LEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  

  pht1 = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht1[ii]=PHT_CTR_INIT; 
  }
  
  
  pht2 = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht2[ii]=PHT_CTR_INIT;
  }
  
  
  bimodal = new UINT32[numBimodalEntries];

  for(UINT32 ii=0; ii< numBimodalEntries; ii++){
    bimodal[ii]=PHT_CTR_INIT;
  }
  
}


bool   PREDICTOR::GetPrediction(UINT64 PC){
	
  numPhtEntries    = (1 << VECTOR_LEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  
  UINT32 majority_vote = 0;
  UINT64 f0, f1;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + ghr;
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  
  
  f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
  f1 = H (v2, VECTOR_LEN) ^ H_inversed (v1, VECTOR_LEN) ^ v1;
  
 
  UINT32 pht1_Counter = pht1[f0];
  UINT32 pht2_Counter = pht2[f1];
  UINT32 bimodal_Counter = bimodal[bimodal_index];


	if(pht1_Counter > PHT_CTR_MAX/2){
		
		majority_vote++;
	}
	
	if(pht2_Counter > PHT_CTR_MAX/2){
		
		majority_vote++;
	}
	
	if(bimodal_Counter > PHT_CTR_MAX/2){
		
		majority_vote++;
	}
	
	if(majority_vote == 3 || majority_vote == 2){
		
		return TAKEN;
	}
	else{
		
		return NOT_TAKEN;
	}

}


void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){
	
  numPhtEntries    = (1 << VECTOR_LEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  
  UINT32 majority_vote = 0;
  UINT64 f0, f1;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + ghr;
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  
  
  f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
  f1 = H (v2, VECTOR_LEN) ^ H_inversed (v1, VECTOR_LEN) ^ v1;
 
  
  UINT32 pht1_Counter = pht1[f0];
  UINT32 pht2_Counter = pht2[f1];
  UINT32 bimodal_Counter = bimodal[bimodal_index];
  
//My stats
NumBranches++;

if((pht1_Counter > PHT_CTR_MAX/2) != resolveDir)
	pht1_Absolute_Mispred++;

if((pht2_Counter > PHT_CTR_MAX/2) != resolveDir)
	pht2_Absolute_Mispred++;

if((bimodal_Counter > PHT_CTR_MAX/2) != resolveDir)
	bimodal_Absolute_Mispred++;

if(		((pht1_Counter > PHT_CTR_MAX/2) != resolveDir)	&&	((pht1_Counter > PHT_CTR_MAX/2) == predDir)		)
	pht1_Relative_Mispred++;

if(		((pht2_Counter > PHT_CTR_MAX/2) != resolveDir)	&&	((pht2_Counter > PHT_CTR_MAX/2) == predDir)		)
	pht2_Relative_Mispred++;

if(		((bimodal_Counter > PHT_CTR_MAX/2) != resolveDir)	&&	((bimodal_Counter > PHT_CTR_MAX/2) == predDir)		)
	bimodal_Relative_Mispred++;

if((pht1_Counter > PHT_CTR_MAX/2) == predDir)
	Numpht1++;

if((pht2_Counter > PHT_CTR_MAX/2) == predDir)
	Numpht2++;

if((bimodal_Counter > PHT_CTR_MAX/2) == predDir)
	Numbimodal++;
  
  
//Update
  
  if(predDir == resolveDir){
	  
	  
	if((pht1_Counter > PHT_CTR_MAX/2) == resolveDir){
		
		if(resolveDir == TAKEN){
		
			pht1[f0] = SatIncrement(pht1_Counter, PHT_CTR_MAX);
		
		}
		else{
		
			pht1[f0] = SatDecrement(pht1_Counter);
		}
	}
	
	
	if((pht2_Counter > PHT_CTR_MAX/2) == resolveDir){
		
		if(resolveDir == TAKEN){
		
			pht2[f1] = SatIncrement(pht2_Counter, PHT_CTR_MAX);
		
		}
		else{
		
			pht2[f1] = SatDecrement(pht2_Counter);
		}
	}
	
	
	if((bimodal_Counter > PHT_CTR_MAX/2) == resolveDir){
		
		if(resolveDir == TAKEN){
		
			bimodal[bimodal_index] = SatIncrement(bimodal_Counter, PHT_CTR_MAX);
		
		}
		else{
		
			bimodal[bimodal_index] = SatDecrement(bimodal_Counter);
		}
	}
	
	
  }
  else{
	  
	  NumMispred++;
	  
	  if(resolveDir == TAKEN){
	  
		pht1[f0] = SatIncrement(pht1_Counter, PHT_CTR_MAX);
		pht2[f1] = SatIncrement(pht2_Counter, PHT_CTR_MAX);
		bimodal[bimodal_index] = SatIncrement(bimodal_Counter, PHT_CTR_MAX);
	  
	  }
	  else{
		  
		pht1[f0] = SatDecrement(pht1_Counter);
		pht2[f1] = SatDecrement(pht2_Counter);
		bimodal[bimodal_index] = SatDecrement(bimodal_Counter);
	  }
  }
  

    // update the GHR
    ghr = (ghr << 1);

    if(resolveDir == TAKEN){
      ghr++;
    }
	
	UINT64 x = 1;
	
	ghr = ghr % (x << HIST_LEN);
  
}


void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;

}

void PrintStat(){
	
	
//Absolute mispred = actual mispred of a bank, purely theoretical
//Relative mispred = mispred by which the bank contributed to bad prediction	 	 

printf("\n");
printf("\nPredictor_accurancy:%9.5f %",  100.0*(double)(NumMispred)/NumBranches);
printf("\n");

printf("\npht1_absolute_mispred:%9.5f %", 100.0*(double)(pht1_Absolute_Mispred)/NumBranches);
printf("\npht1_relative_mispred:%9.5f %",   100.0*(double)(pht1_Relative_Mispred)/Numpht1);
printf("\npht1_how often used:%9.5f %",   100.0*(double)(Numpht1)/NumBranches);
printf("\n");

printf("\npht2_absolute_mispred:%9.5f %", 100.0*(double)(pht2_Absolute_Mispred)/NumBranches);
printf("\npht2_relative_mispred:%9.5f %", 100.0*(double)(pht2_Relative_Mispred)/Numpht2);
printf("\npht2_how often used:%9.5f %", 100.0*(double)(Numpht2)/NumBranches);
printf("\n");

printf("\nbimodal_absolute_mispred:%9.5f %", 100.0*(double)(bimodal_Absolute_Mispred)/NumBranches);
printf("\nbimodal_relative_mispred:%9.5f %", 100.0*(double)(bimodal_Relative_Mispred)/Numbimodal);
printf("\nbimodal_how often used:%9.5f %", 100.0*(double)(Numbimodal)/NumBranches);
printf("\n");

	return;
}

/***********************************************************/
#endif