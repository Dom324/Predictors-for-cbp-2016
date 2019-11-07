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

#define PHT_CTR_MAX  8
#define PHT_CTR_INIT 4
#define NUM_BTB_ENTRIES 8192


class PREDICTOR{


 private:
  UINT32  *btb;
 public:


  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
};




PREDICTOR::PREDICTOR(void){
	
	btb = new UINT32[NUM_BTB_ENTRIES];
	
	for(UINT32 ii=0; ii< NUM_BTB_ENTRIES; ii++){
    btb[ii]=PHT_CTR_INIT; 
    }
	
}





bool   PREDICTOR::GetPrediction(UINT64 PC){
  
  UINT32 btbIndex   = (PC) % (NUM_BTB_ENTRIES);
  UINT32 btbCounter = btb[btbIndex];

  if(btbCounter  > (PHT_CTR_MAX/2)){return TAKEN;}
  else{return NOT_TAKEN;}
  
  }


  
  
void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){

  UINT32 btbIndex   = (PC) % (NUM_BTB_ENTRIES);
  UINT32 btbCounter = btb[btbIndex];
  
  if(resolveDir == 1){
      btb[btbIndex] = SatIncrement(btbCounter, PHT_CTR_MAX);}
  else{
      btb[btbIndex] = SatDecrement(btbCounter);}

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
#endif

