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

#define VECTOR_LEN  15
#define BIMODAL_LEN (VECTOR_LEN - 2)

#define HIST_LEN  15
#define SHFT_AMOUNT 2

#define WARM_UP_TIME 16
#define REFRESH_TIME 20

//stats
 // UINT64  NumMispred = 0, NumBranches = 0, NumphtTaken = 0, NumphtNotTaken = 0, Numbimodal = 0, phtTakenakenRelativeMispred = 0, pht1NotTakenRelativeMispred = 0,
 //         pht2NotTakenRelativeMispred = 0, bimodalRelativeMispred = 0, bimodalEgskewRelativeMispred = 0, metaChoosedBimodal = 0, metaChoosedWrong = 0, metaDontMatter = 0;
//stats

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT64  ghr = 0;           // global history register
  UINT32  *phtTaken;          // pattern history table
  UINT32  *phtNotTaken;          // pattern history table
  UINT32  *bimodal;		  // bimodal table
  UINT32  *HYST;
  UINT32  numPhtEntries; // entries in pht 
  UINT32  numBimodalEntries;
  UINT32  WarmUpTime = 0;
  UINT32  Refresh = 0;

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
  

  phtTaken = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    phtTaken[ii]=PHT_CTR_INIT; 
  }
  
  phtNotTaken = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    phtNotTaken[ii]=0; 
  }
  
  bimodal = new UINT32[numBimodalEntries];

  for(UINT32 ii=0; ii< numBimodalEntries; ii++){
    bimodal[ii]=PHT_CTR_INIT;
  }
  
  
  HYST = new UINT32[numPhtEntries];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    HYST[ii]= 0;
  }
  
}


bool   PREDICTOR::GetPrediction(UINT64 PC){
	
  numPhtEntries    = (1 << VECTOR_LEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  
  UINT32 majorityVote = 0;
  UINT64 f0, f1;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + (ghr % (1 << HIST_LEN));
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  
  
	f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
	f1 = H_inversed (v1, VECTOR_LEN) ^ H (v2, VECTOR_LEN) ^ v1;
  
 
  UINT32 phtCounterT1 = (phtTaken[f0] << 1) + HYST[f0];
  UINT32 phtCounterNT1 = (phtNotTaken[f0] << 1) + HYST[f0];
  UINT32 phtCounterT2= (phtTaken[f1] << 1) + HYST[f1];
  UINT32 phtCounterNT2 = (phtNotTaken[f1] << 1) + HYST[f1];
  UINT32 bimodalCounter = bimodal[bimodal_index];
		
	if(bimodalCounter == PHT_CTR_MAX  ||  bimodalCounter == 0){
		
		if(bimodalCounter > PHT_CTR_MAX/2){
		
			majorityVote++;
			
			if(phtCounterT1 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
	
			if(phtCounterT2 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
			
		}
		else{

			if(phtCounterNT1 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
	
			if(phtCounterNT2 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
			
		}
	
		if(majorityVote == 3 || majorityVote == 2){
		
			return TAKEN;
		}
		else{
			
			return NOT_TAKEN;
		}
		
	}
	else{
			
		if(bimodalCounter > PHT_CTR_MAX/2)
			majorityVote++;
		
		if(phtCounterT1 > PHT_CTR_MAX/2)
			majorityVote++;
	
		if(phtCounterT2 > PHT_CTR_MAX/2)
			majorityVote++;
			
		if(phtCounterNT1 > PHT_CTR_MAX/2)
			majorityVote++;
	
		if(phtCounterNT2 > PHT_CTR_MAX/2)
			majorityVote++;

		if(majorityVote == 5 || majorityVote == 4 || majorityVote == 3){
		
			return TAKEN;
		}
		else{
			
			return NOT_TAKEN;
		}

	}

}


void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){
	
  numPhtEntries    = (1 << VECTOR_LEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  
  UINT32 majorityVote = 0;
  UINT64 f0, f1;
  UINT32 egskewOutcome, newPrediction, predMethod;
	
  UINT64 v = ((PC >> SHFT_AMOUNT) << HIST_LEN) + (ghr % (1 << HIST_LEN));
  
  UINT64 v1 = v % numPhtEntries;
  UINT64 v2 = (v >> VECTOR_LEN) % (numPhtEntries);
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  
  
	f0 = H (v1, VECTOR_LEN) ^ H_inversed (v2, VECTOR_LEN) ^ v2;
	f1 = H_inversed (v1, VECTOR_LEN) ^ H (v2, VECTOR_LEN) ^ v1;
  
 
  UINT32 phtCounterT1 = (phtTaken[f0] << 1) + HYST[f0];
  UINT32 phtCounterNT1 = (phtNotTaken[f0] << 1) + HYST[f0];
  UINT32 phtCounterT2= (phtTaken[f1] << 1) + HYST[f1];
  UINT32 phtCounterNT2 = (phtNotTaken[f1] << 1) + HYST[f1];
  UINT32 bimodalCounter = bimodal[bimodal_index];
  
  
	if(bimodalCounter == PHT_CTR_MAX  ||  bimodalCounter == 0){
		
		predMethod = 1;
		
		if(bimodalCounter > PHT_CTR_MAX/2){
		
			majorityVote++;
			
			if(phtCounterT1 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
	
			if(phtCounterT2 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
			
		}
		else{

			if(phtCounterNT1 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
	
			if(phtCounterNT2 > PHT_CTR_MAX/2){
		
				majorityVote++;
			}
			
		}
	
		if(majorityVote == 3 || majorityVote == 2){
		
			egskewOutcome = 1;
		}
		else{
			
			egskewOutcome = 0;
		}
		
	}
	else{
		
		predMethod = 0;
			
		if(bimodalCounter > PHT_CTR_MAX/2)
			majorityVote++;
		
		if(phtCounterT1 > PHT_CTR_MAX/2)
			majorityVote++;
	
		if(phtCounterT2 > PHT_CTR_MAX/2)
			majorityVote++;
			
		if(phtCounterNT1 > PHT_CTR_MAX/2)
			majorityVote++;
	
		if(phtCounterNT2 > PHT_CTR_MAX/2)
			majorityVote++;

		if(majorityVote == 5 || majorityVote == 4 || majorityVote == 3){
		
			egskewOutcome = 1;
		}
		else{
			
			egskewOutcome = 0;
		}

	}
  
/*//My stats
NumBranches++;

if(resolveDir != predDir){NumMispred++;}

if(		((phtCounterT1 > PHT_CTR_MAX/2) != resolveDir)	&&	((phtCounterT1 > PHT_CTR_MAX/2) == predDir)   &&   (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 1))
	phtTakenakenRelativeMispred++;

if(		((phtCounterT1 > PHT_CTR_MAX/2) != resolveDir)	&&	((phtCounterT1 > PHT_CTR_MAX/2) == predDir)   &&   (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 0))
	pht1NotTakenRelativeMispred++;

if(		((phtCounterNT2 > PHT_CTR_MAX/2) != resolveDir)	&&	((phtCounterNT2 > PHT_CTR_MAX/2) == predDir)		&&	(metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 1))
	phtTakenakenRelativeMispred++;

if(		((phtCounterNT2 > PHT_CTR_MAX/2) != resolveDir)	&&	((phtCounterNT2 > PHT_CTR_MAX/2) == predDir)		&&	(metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 0))
	pht2NotTakenRelativeMispred++;

if(		((bimodalCounter > PHT_CTR_MAX/2) != resolveDir)	&&	((bimodalCounter > PHT_CTR_MAX/2) == predDir)		&&	(metaCounter > PHT_CTR_MAX/2))
	bimodalEgskewRelativeMispred++;

if(		((bimodalCounter > PHT_CTR_MAX/2) != resolveDir)	&&	(metaCounter < PHT_CTR_MAX/2))
	bimodalRelativeMispred++;

if(  ((phtCounterT1 > PHT_CTR_MAX/2) == predDir)  &&  (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 1))
	NumphtTaken++;

if(  ((phtCounterNT1 > PHT_CTR_MAX/2) == predDir)  &&  (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 0))
	NumphtNotTaken++;

if(  ((phtCounterT2 > PHT_CTR_MAX/2) == predDir)  &&  (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 1))
	NumphtTaken++;

if(  ((phtCounterNT2 > PHT_CTR_MAX/2) == predDir)  &&  (metaCounter > PHT_CTR_MAX/2)  &&  ((bimodalCounter >> 1) == 0))
	NumphtNotTaken++;

if(((bimodalCounter > PHT_CTR_MAX/2) == predDir) && (metaCounter > PHT_CTR_MAX/2))
	Numbimodal++;


if(metaCounter < PHT_CTR_MAX/2)
	metaChoosedBimodal++;

if((metaCounter < PHT_CTR_MAX/2)	&&	((bimodalCounter > PHT_CTR_MAX/2) != resolveDir)	 &&	(egskewOutcome == resolveDir))
	metaChoosedWrong++;

if((metaCounter > PHT_CTR_MAX/2)	&&	(egskewOutcome != resolveDir) &&	((bimodalCounter > PHT_CTR_MAX/2) == resolveDir))
	metaChoosedWrong++;

if(		(((bimodalCounter > PHT_CTR_MAX/2) != resolveDir)		&&		(egskewOutcome != resolveDir))		||		(((bimodalCounter > PHT_CTR_MAX/2) == resolveDir)		&&		(egskewOutcome == resolveDir))		)
	metaDontMatter++;

*/


//Update 
  
	
	//UpdateALL:
	if(predMethod == 1){
	  
	if(resolveDir == TAKEN){
		
		bimodal[bimodal_index] = SatIncrement(bimodalCounter, PHT_CTR_MAX);	
		
		if(bimodalCounter >> 1 == 1){
		
			phtCounterT1 = SatIncrement(phtCounterT1, PHT_CTR_MAX);
			phtCounterT2 = SatIncrement(phtCounterT2, PHT_CTR_MAX);
		}
		else{
			
			phtCounterNT1 = SatIncrement(phtCounterNT1, PHT_CTR_MAX);
			phtCounterNT2 = SatIncrement(phtCounterNT2, PHT_CTR_MAX);	
		}
	  
	  	phtTaken[f0] = phtCounterT1 >> 1;
		phtTaken[f1] = phtCounterT2 >> 1;
		HYST[f0] = phtCounterT1 & 1;
	    HYST[f1] = phtCounterT2 & 1;
	  
	}
	else{
		
		bimodal[bimodal_index] = SatDecrement(bimodalCounter);
		
		if(bimodalCounter >> 1 == 1){
		
			phtCounterT1 = SatDecrement(phtCounterT1);
			phtCounterT2 = SatDecrement(phtCounterT2);
		}
		else{
			
			phtCounterNT1 = SatDecrement(phtCounterNT1);
			phtCounterNT2 = SatDecrement(phtCounterNT2);
		}
		
		phtNotTaken[f0] = phtCounterNT1 >> 1;
		phtNotTaken[f1] = phtCounterNT2 >> 1;
		HYST[f0] = phtCounterNT1 & 1;
		HYST[f1] = phtCounterNT2 & 1;
		
	  }
	  
	}
	else{
		
		if(resolveDir == TAKEN){
		
			bimodal[bimodal_index] = SatIncrement(bimodalCounter, PHT_CTR_MAX);		
			phtCounterT1 = SatIncrement(phtCounterT1, PHT_CTR_MAX);
			phtCounterT2 = SatIncrement(phtCounterT2, PHT_CTR_MAX);	
			phtCounterNT1 = SatIncrement(phtCounterNT1, PHT_CTR_MAX);
			phtCounterNT2 = SatIncrement(phtCounterNT2, PHT_CTR_MAX);	
		}
	    else{
		  

			bimodal[bimodal_index] = SatDecrement(bimodalCounter);
			phtCounterT1 = SatDecrement(phtCounterT1);
			phtCounterT2 = SatDecrement(phtCounterT2);
			phtCounterNT1 = SatDecrement(phtCounterNT1);
			phtCounterNT2 = SatDecrement(phtCounterNT2);
			
		}
		
	  phtTaken[f0] = phtCounterT1 >> 1;
	  phtTaken[f1] = phtCounterT2 >> 1;
	  phtNotTaken[f0] = phtCounterNT1 >> 1;
	  phtNotTaken[f1] = phtCounterNT2 >> 1;
	  HYST[f0] = phtCounterT1 & 1;
	  HYST[f1] = phtCounterT2 & 1;
	  HYST[f0] = phtCounterNT1 & 1;
	  HYST[f1] = phtCounterNT2 & 1;
		
	}
	
	
    // update the GHR
    ghr = (ghr << 1);

    if(resolveDir == TAKEN){
      ghr++;
    }
  
}


void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;

}
void PrintStat(){}
/*void PrintStat(){
	
	
//Absolute mispred = actual mispred of a bank, purely theoretical
//Relative mispred = mispred by which the bank contributed to bad prediction	 	 

printf("\n");
printf("\nPredictor_accurancy:%9.5f %%",  100 - (100.0*(double)(NumMispred)/NumBranches));
printf("\n");

printf("\nMETA");
printf("\nMeta choosed bimodal:%9.5f %%",  (100.0*(double)(metaChoosedBimodal)/NumBranches));
printf("\nMeta choosed egskew:%9.5f %%",  100 - (100.0*(double)(metaChoosedBimodal)/NumBranches));
printf("\nMeta choosed right:%9.5f %%",  100 - (100.0*(double)(metaChoosedWrong)/NumBranches) - (100.0*(double)(metaDontMatter)/NumBranches));
printf("\nMeta choosed wrong:%9.5f %%",  (100.0*(double)(metaChoosedWrong)/NumBranches));
printf("\nMeta doesnt matter:%9.5f %%",  (100.0*(double)(metaDontMatter)/NumBranches));
printf("\nTaken PHTs used:%9.5f %%",  (100.0*(double)(NumphtTaken + NumphtTaken)/NumBranches));
printf("\nNot Taken PHTs used:%9.5f %%",  (100.0*(double)(NumphtNotTaken + NumphtNotTaken)/NumBranches));
printf("\n");


printf("\npht1_relative_mispred:%9.5f %%",   100.0*(double)( (phtTakenakenRelativeMispred + pht1NotTakenRelativeMispred) / (NumphtTaken + NumphtNotTaken) ) );
printf("\npht1_how often used:%9.5f %%",   100.0*(double)(NumphtTaken + NumphtNotTaken)/NumBranches);
printf("\npht1 Taken misprediction:%9.5f %%",   100.0*(double) ( phtTakenakenRelativeMispred / NumphtTaken ) );
printf("\npht1 Not Taken misprediction:%9.5f %%",   100.0*(double) ( pht1NotTakenRelativeMispred / NumphtNotTaken ) );
printf("\n");

printf("\npht2_relative_mispred:%9.5f %%", 100.0*(double) ( (phtTakenakenRelativeMispred + pht2NotTakenRelativeMispred)/(NumphtTaken + NumphtNotTaken) ) );
printf("\npht2_how often used:%9.5f %%", 100.0*(double)(NumphtTaken + NumphtNotTaken)/NumBranches);
printf("\npht2 Taken misprediction:%9.5f %%",   100.0*(double) ( phtTakenakenRelativeMispred / NumphtTaken ) );
printf("\npht2 Not Taken misprediction:%9.5f %%",   100.0*(double) ( pht2NotTakenRelativeMispred / NumphtNotTaken ) );
printf("\n");

printf("\nbimodal_relative_mispred in egskew:%9.5f %%", 100.0*(double)(bimodalEgskewRelativeMispred)/Numbimodal);
printf("\nbimodal_how often used in egskew:%9.5f %%", 100.0*(double)(Numbimodal)/NumBranches);
printf("\n");
printf("\nbimodal_relative_mispred in BIM:%9.5f %%", 100.0*(double)(bimodalRelativeMispred)/metaChoosedBimodal);
printf("\nMeta choosed bimodal:%9.5f %%",  (100.0*(double)(metaChoosedBimodal)/NumBranches));
printf("\n");

	return;
}*/

/***********************************************************/
#endif