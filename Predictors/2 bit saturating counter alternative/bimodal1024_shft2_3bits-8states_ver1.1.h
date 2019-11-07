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
#define NUM_BTB_ENTRIES 1024
#define SHFT_AMOUNT 2


class PREDICTOR{


 private:
  UINT32  *bimodal;
 public:


  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
};




PREDICTOR::PREDICTOR(void){
	
	bimodal = new UINT32[NUM_BTB_ENTRIES];
	
	for(UINT32 ii=0; ii< NUM_BTB_ENTRIES; ii++){
    bimodal[ii]=PHT_CTR_INIT; 
    }
	
}





bool   PREDICTOR::GetPrediction(UINT64 PC){
  
  UINT32 BimodalIndex   = (PC << SHFT_AMOUNT) % (NUM_BTB_ENTRIES);
  UINT32 bimodalCounter = bimodal[BimodalIndex];


  if(bimodalCounter > PHT_CTR_MAX/2){
	  
	return TAKEN;
	
  }
  else{
	  
	return NOT_TAKEN;
	
  }
  
}


  
  
void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){

  UINT32 BimodalIndex   = (PC << SHFT_AMOUNT) % (NUM_BTB_ENTRIES);
  UINT32 bimodalCounter = bimodal[BimodalIndex];
  
  if(resolveDir == TAKEN){

	if(bimodalCounter != 3 && bimodalCounter != PHT_CTR_MAX){
		
		bimodalCounter++;
		
	}
	else if(bimodalCounter == 3){
		
		bimodalCounter = 5;
		
	}
	
  }
  else{
	  
	if(bimodalCounter != 4 && bimodalCounter != 0){
		
		bimodalCounter--;
		
	}
	else if(bimodalCounter == 4){
		
		bimodalCounter = 2;
		
	}
	
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

