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
#define HIST_LEN  34
#define VECTOR_LEN  21
#define SHFT_AMOUNT 2


class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT64  ghr = 0;           // global history register
  UINT32  *pht1;          // pattern history table
  UINT32  *pht2;          // pattern history table
  UINT32  *pht3;          // pattern history table
  UINT32  numPhtEntries; // entries in pht 

 public:


  PREDICTOR(void);

  // The interface to the functions below CAN NOT be changed
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  

  // Contestants can define their own functions below
};

PREDICTOR::PREDICTOR(void){
	
  ghr              = 0;
  numPhtEntries    = (1 << VECTOR_LEN);
  

  pht1 = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht1[ii]=PHT_CTR_INIT; 
  }
  
  
  pht2 = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht2[ii]=PHT_CTR_INIT;
  }
  
  
  pht3 = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht3[ii]=PHT_CTR_INIT;
  }
  
}


bool   PREDICTOR::GetPrediction(UINT64 PC){
	
  numPhtEntries    = (1 << VECTOR_LEN);
  
  UINT32 majority_vote = 0;
  UINT64 f0, f1, f2;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + ghr;
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  
  
  f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
  f1 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v1;
  f2 = H_inversed (v1, VECTOR_LEN) ^ H (v2, VECTOR_LEN) ^ v2;
  
  
  UINT32 pht1_Counter = pht1[f0];
  UINT32 pht2_Counter = pht2[f1];
  UINT32 pht3_Counter = pht3[f2];


	if(pht1_Counter > PHT_CTR_MAX/2){
		
		majority_vote++;
	}
	
	if(pht2_Counter > PHT_CTR_MAX/2){
		
		majority_vote++;
	}
	
	if(pht3_Counter > PHT_CTR_MAX/2){
		
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
  
  UINT32 majority_vote = 0;
  UINT64 f0, f1, f2;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + ghr;
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  
  
  f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
  f1 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v1;
  f2 = H_inversed (v1, VECTOR_LEN) ^ H (v2, VECTOR_LEN) ^ v2;
  
  
  UINT32 pht1_Counter = pht1[f0];
  UINT32 pht2_Counter = pht2[f1];
  UINT32 pht3_Counter = pht3[f2];
  
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
	
	
	if((pht3_Counter > PHT_CTR_MAX/2) == resolveDir){
		
		if(resolveDir == TAKEN){
		
			pht3[f2] = SatIncrement(pht3_Counter, PHT_CTR_MAX);
		
		}
		else{
		
			pht3[f2] = SatDecrement(pht3_Counter);
		}
	}
	
	
  }
  else{
	  
	  if(resolveDir == TAKEN){
	  
		pht1[f0] = SatIncrement(pht1_Counter, PHT_CTR_MAX);
		pht2[f1] = SatIncrement(pht2_Counter, PHT_CTR_MAX);
		pht3[f2] = SatIncrement(pht3_Counter, PHT_CTR_MAX);
	  
	  }
	  else{
		  
		pht1[f0] = SatDecrement(pht1_Counter);
		pht2[f1] = SatDecrement(pht2_Counter);
		pht3[f2] = SatDecrement(pht3_Counter);
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

/***********************************************************/
#endif