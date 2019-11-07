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

#define PHT_CTR_MAX  7
#define PHT_CTR_INIT 5

#define SHFT_AMOUNT 2
#define HIST_LEN  16


class PREDICTOR{


 private:
  UINT32  *pht;
  UINT32  ghr;				//global history register
  UINT32  numPhtEntries;
 public:


  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
};




PREDICTOR::PREDICTOR(void){
	
	numPhtEntries = 1 << HIST_LEN;
	
	pht = new UINT32[numPhtEntries];
	
	for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht[ii]=PHT_CTR_INIT; 
    }
	
}





bool   PREDICTOR::GetPrediction(UINT64 PC){
  
  UINT32 phtIndex   = (PC ^ ghr) % numPhtEntries;
  UINT32 phtCounter = pht[phtIndex];


  if(phtCounter > PHT_CTR_MAX/2){
	  
	return TAKEN;
	
  }
  else{
	  
	return NOT_TAKEN;
	
  }
  
}


  
  
void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){

  UINT32 phtIndex   = (PC ^ ghr) % numPhtEntries;
  UINT32 phtCounter = pht[phtIndex];
  
  if(resolveDir == TAKEN){

	if(phtCounter != 2 && phtCounter != PHT_CTR_MAX){
		
		pht[phtIndex]++;
		
	}
	else if(phtCounter == 2){
		
		pht[phtIndex] = 4;
		
	}
	
  }
  else{
	  
	if(phtCounter != 3 && phtCounter != 0){
		
		pht[phtIndex]--;
		
	}
	else if(phtCounter == 3){
		
		pht[phtIndex] = 1;
		
	}
	
  }
  
  
  ghr = ghr << 1;
  if(resolveDir == TAKEN){
	  
	ghr++;
  }

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;
}

void PrintStat(){
	

	return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
#endif

