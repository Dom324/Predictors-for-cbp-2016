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

#define VECTOR_LEN  16
#define VECTOR_LEN_TAKEN  16
#define VECTOR_LEN_NOT_TAKEN  16

#define BIMODAL_LEN (VECTOR_LEN - 1)

//#define HL_BIM 5
#define HL_TAKEN_1 18
#define HL_TAKEN_2 23

#define HL_NOTTAKEN_1 17
#define HL_NOTTAKEN_2 24

#define SHFT_AMOUNT 2
#define HystShft 2

#define WARM_UP_TIME 16
#define REFRESH_TIME 20

//stats
  UINT64  NumMispred = 0, NumBranches = 0, phtT1mispred = 0, phtT2mispred = 0, phtNT1mispred = 0, phtNT2mispred = 0, bimodalMispred = 0,
		  MajorityVote3Mispred = 0, MajorityVote5Mispred = 0, MajorityVote5Used = 0, MajorityVote3Used = 0, TakenPhtUsed = 0, NotTakenPhtUsed = 0, TakenPhtMispred = 0, NotTakenPhtMispred = 0;
//stats

class PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT64  ghr = 0;           // global history register
  UINT32  *phtTaken;          // pattern history table
  UINT32  *phtNotTaken;          // pattern history table
  UINT32  *bimodal;		  // bimodal table
  UINT32  *HYSTTaken;
  UINT32  *HYSTNotTaken;
  UINT32  numPhtEntries; // entries in pht 
  UINT32  numPhtEntriesTaken; // entries in pht 
  UINT32  numPhtEntriesNotTaken; // entries in pht 
  UINT32  numBimodalEntries;
  UINT32  numHystEntries;
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
  numPhtEntriesTaken    = (1 << VECTOR_LEN_TAKEN);
  numPhtEntriesNotTaken    = (1 << VECTOR_LEN_NOT_TAKEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  numHystEntries = (1 << (VECTOR_LEN - HystShft));
  

  phtTaken = new UINT32[numPhtEntriesTaken];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    phtTaken[ii]=1; 
  }
  
  phtNotTaken = new UINT32[numPhtEntriesNotTaken];

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    phtNotTaken[ii]=0; 
  }
  
  bimodal = new UINT32[numBimodalEntries];

  for(UINT32 ii=0; ii< numBimodalEntries; ii++){
    bimodal[ii]=PHT_CTR_INIT;
  }
  
  
  HYSTTaken = new UINT32[numHystEntries];

  for(UINT32 ii=0; ii< numHystEntries; ii++){
    HYSTTaken[ii]= 0;
  }
  
  HYSTNotTaken = new UINT32[numHystEntries];

  for(UINT32 ii=0; ii< numHystEntries; ii++){
    HYSTNotTaken[ii]= 0;
  }
  
}


bool   PREDICTOR::GetPrediction(UINT64 PC){
	
  numPhtEntriesTaken    = (1 << VECTOR_LEN_TAKEN);
  numPhtEntriesNotTaken    = (1 << VECTOR_LEN_NOT_TAKEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  numHystEntries = (1 << (VECTOR_LEN - HystShft));
  
  UINT32 majorityVote = 0;
  UINT64 f0T, f1T, f0NT, f1NT, v, v1, v2;
  
  
	//Bimodal index
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  UINT32 bimodalCounter = bimodal[bimodal_index];
	
	//Index PHT1 TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_TAKEN_1) + (ghr % ((UINT64)1 << HL_TAKEN_1));
  v1 = v % numPhtEntriesTaken;
  v2 = (v >> VECTOR_LEN_TAKEN) % (numPhtEntriesTaken);
  
	f0T = H (v1, VECTOR_LEN_TAKEN) ^ H_inversed (v2, VECTOR_LEN_TAKEN) ^ v2;
	
	
	//Index PHT2 TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_TAKEN_2) + (ghr % ((UINT64)1 << HL_TAKEN_2));
  v1 = v % numPhtEntriesTaken;
  v2 = (v >> VECTOR_LEN_TAKEN) % (numPhtEntriesTaken);
  
	f1T = H_inversed (v1, VECTOR_LEN_TAKEN) ^ H (v2, VECTOR_LEN_TAKEN) ^ v1;
	
		UINT32 phtCounterT1 = (phtTaken[f0T] << 1) + HYSTTaken[f0T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)];
		UINT32 phtCounterT2 = (phtTaken[f1T] << 1) + HYSTTaken[f1T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)];
	
	
	//Index PHT1 NOT TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_NOTTAKEN_1) + (ghr % ((UINT64)1 << HL_NOTTAKEN_1));
  v1 = v % numPhtEntriesNotTaken;
  v2 = (v >> VECTOR_LEN_NOT_TAKEN) % (numPhtEntriesNotTaken);
  
	f0NT = H (v1, VECTOR_LEN_NOT_TAKEN) ^ H_inversed (v2, VECTOR_LEN_NOT_TAKEN) ^ v2;
	
	
	//Index PHT2 NOT TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_NOTTAKEN_2) + (ghr % ((UINT64)1 << HL_NOTTAKEN_2));
  v1 = v % numPhtEntriesNotTaken;
  v2 = (v >> VECTOR_LEN_NOT_TAKEN) % (numPhtEntriesNotTaken);
  
	f1NT = H_inversed (v1, VECTOR_LEN_NOT_TAKEN) ^ H (v2, VECTOR_LEN_NOT_TAKEN) ^ v1;
 
		UINT32 phtCounterNT1 = (phtNotTaken[f0NT] << 1) + HYSTNotTaken[f0NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)];
		UINT32 phtCounterNT2 = (phtNotTaken[f1NT] << 1) + HYSTNotTaken[f1NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)];

	
	if((bimodalCounter & 1) == 1){
		
		if(bimodalCounter >> 1){
		
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
		
		if(bimodalCounter >> 1)
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
	
  numPhtEntriesTaken    = (1 << VECTOR_LEN_TAKEN);
  numPhtEntriesNotTaken    = (1 << VECTOR_LEN_NOT_TAKEN);
  numBimodalEntries = (1 << BIMODAL_LEN);
  numHystEntries = (1 << (VECTOR_LEN - HystShft));
  
  UINT32 majorityVote = 0;
  UINT64 f0T, f1T, f0NT, f1NT, v, v1, v2;
  UINT32 egskewOutcome, newPrediction, predMethod;
  
  
	//Bimodal index
  UINT32 bimodal_index = (PC >> SHFT_AMOUNT) % numBimodalEntries;
  UINT32 bimodalCounter = bimodal[bimodal_index];
	
	//Index PHT1 TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_TAKEN_1) + (ghr % ((UINT64)1 << HL_TAKEN_1));
  v1 = v % numPhtEntriesTaken;
  v2 = (v >> VECTOR_LEN_TAKEN) % (numPhtEntriesTaken);
  
	f0T = H (v1, VECTOR_LEN_TAKEN) ^ H_inversed (v2, VECTOR_LEN_TAKEN) ^ v2;
	
	
	//Index PHT2 TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_TAKEN_2) + (ghr % ((UINT64)1 << HL_TAKEN_2));
  v1 = v % numPhtEntriesTaken;
  v2 = (v >> VECTOR_LEN_TAKEN) % (numPhtEntriesTaken);
  
	f1T = H_inversed (v1, VECTOR_LEN_TAKEN) ^ H (v2, VECTOR_LEN_TAKEN) ^ v1;
	
		UINT32 phtCounterT1 = (phtTaken[f0T] << 1) + HYSTTaken[f0T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)];
		UINT32 phtCounterT2 = (phtTaken[f1T] << 1) + HYSTTaken[f1T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)];
	
	
	//Index PHT1 NOT TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_NOTTAKEN_1) + (ghr % ((UINT64)1 << HL_NOTTAKEN_1));
  v1 = v % numPhtEntriesNotTaken;
  v2 = (v >> VECTOR_LEN_NOT_TAKEN) % (numPhtEntriesNotTaken);
  
	f0NT = H (v1, VECTOR_LEN_NOT_TAKEN) ^ H_inversed (v2, VECTOR_LEN_NOT_TAKEN) ^ v2;
	
	
	//Index PHT2 NOT TAKEN
  v = ((PC >> SHFT_AMOUNT) << HL_NOTTAKEN_2) + (ghr % ((UINT64)1 << HL_NOTTAKEN_2));
  v1 = v % numPhtEntriesNotTaken;
  v2 = (v >> VECTOR_LEN_NOT_TAKEN) % (numPhtEntriesNotTaken);
  
	f1NT = H_inversed (v1, VECTOR_LEN_NOT_TAKEN) ^ H (v2, VECTOR_LEN_NOT_TAKEN) ^ v1;
 
		UINT32 phtCounterNT1 = (phtNotTaken[f0NT] << 1) + HYSTNotTaken[f0NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)];
		UINT32 phtCounterNT2 = (phtNotTaken[f1NT] << 1) + HYSTNotTaken[f1NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)];
  
  
	if((bimodalCounter & 1) == 1){
		
		predMethod = 1;
		
		if(bimodalCounter >> 1){
		
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
			
		if(bimodalCounter >> 1)
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
  
//My stats
  NumBranches++;

  if(resolveDir != predDir){NumMispred++;};

  if((bimodalCounter >> 1) != resolveDir){
	bimodalMispred++;
  }
	
if(predMethod == 1){
	
	MajorityVote3Used++;
	
	if(predDir != resolveDir){
		MajorityVote3Mispred++;
	}
	
	if((bimodalCounter >> 1) == 1){
		
		TakenPhtUsed++;
		
		if(predDir != resolveDir){
			TakenPhtMispred++;
		
		if((phtCounterT1 >> 1) != resolveDir){
			phtT1mispred++;
		}
		if((phtCounterT2 >> 1)!= resolveDir){
			phtT2mispred++;
		}
		}
	
	}
	else{
		
		NotTakenPhtUsed++;
		
		if(predDir != resolveDir){
			NotTakenPhtMispred++;
		
		if((phtCounterNT1 >> 1) != resolveDir){
			phtNT1mispred++;
		}
		if((phtCounterNT2 >> 1) != resolveDir){
			phtNT2mispred++;
		}
		}
		
	}
}
else{
	
	MajorityVote5Used++;
	
	if(predDir != resolveDir){
		MajorityVote5Mispred++;

	if((phtCounterT1 >> 1) != resolveDir){
		phtT1mispred++;
	}
	if((phtCounterT2 >> 1) != resolveDir){
		phtT2mispred++;
	}
	if((phtCounterNT1 >> 1) != resolveDir){
		phtNT1mispred++;
	}
	if((phtCounterNT2 >> 1) != resolveDir){
		phtNT2mispred++;
	}
	
	}
	
}


//Update
  
	
	//UpdateALL:
	if(predMethod == 1){
	
		//if( ! (predDir == resolveDir && (bimodalCounter >> 1) != resolveDir))
			bimodal[bimodal_index] = updateCounter(bimodalCounter, resolveDir, PHT_CTR_MAX);
		
		
		if(bimodalCounter >> 1 == 1){
		
			phtCounterT1 = updateCounter(phtCounterT1, resolveDir, PHT_CTR_MAX);
			phtCounterT2 = updateCounter(phtCounterT2, resolveDir, PHT_CTR_MAX);
			
			phtTaken[f0T] = phtCounterT1 >> 1;
			phtTaken[f1T] = phtCounterT2 >> 1;
			HYSTTaken[f0T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)] = phtCounterT1 & 1;
			HYSTTaken[f1T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)] = phtCounterT2 & 1;
		}
		else{
			
			phtCounterNT1 = updateCounter(phtCounterNT1, resolveDir, PHT_CTR_MAX);
			phtCounterNT2 = updateCounter(phtCounterNT2, resolveDir, PHT_CTR_MAX);

			phtNotTaken[f0NT] = phtCounterNT1 >> 1;
			phtNotTaken[f1NT] = phtCounterNT2 >> 1;
			HYSTNotTaken[f0NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)] = phtCounterNT1 & 1;
			HYSTNotTaken[f1NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)] = phtCounterNT2 & 1;			
		}
	}
	else{
		
			bimodal[bimodal_index] = updateCounter(bimodalCounter, resolveDir, PHT_CTR_MAX);	
			phtCounterT1 = updateCounter(phtCounterT1, resolveDir, PHT_CTR_MAX);
			phtCounterT2 = updateCounter(phtCounterT2, resolveDir, PHT_CTR_MAX);
			phtCounterNT1 = updateCounter(phtCounterNT1, resolveDir, PHT_CTR_MAX);
			phtCounterNT2 = updateCounter(phtCounterNT2, resolveDir, PHT_CTR_MAX);
		  

	  phtTaken[f0T] = phtCounterT1 >> 1;
	  phtTaken[f1T] = phtCounterT2 >> 1;
	  phtNotTaken[f0NT] = phtCounterNT1 >> 1;
	  phtNotTaken[f1NT] = phtCounterNT2 >> 1;
	  HYSTTaken[f0T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)] = phtCounterT1 & 1;
	  HYSTTaken[f1T >> (VECTOR_LEN_TAKEN - VECTOR_LEN + HystShft)] = phtCounterT2 & 1;
	  HYSTNotTaken[f0NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)] = phtCounterNT1 & 1;
	  HYSTNotTaken[f1NT >> (VECTOR_LEN_NOT_TAKEN - VECTOR_LEN + HystShft)] = phtCounterNT2 & 1;
		
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
//void PrintStat(){}
void PrintStat(){
	
	
//Absolute mispred = actual mispred of a bank, purely theoretical
//Relative mispred = mispred by which the bank contributed to bad prediction	 	 

printf("\n");
printf("\nPredictor_accurancy:%9.5f %%",  100 - (100.0*(double)(NumMispred)/NumBranches));
printf("\n");

printf("\nMode");
printf("\n5 Component majorityVote was used %9.5f%%",  (100.0*(double)(MajorityVote5Used)/NumBranches));
printf("\n5 Component majorityVote accurancy %9.5f%%",  (100 - 100.0*(double)(MajorityVote5Mispred)/MajorityVote5Used));
printf("\n3 Component majorityVote was used %9.5f%%",  (100.0*(double)(MajorityVote3Used)/NumBranches));
printf("\n3 Component majorityVote accurancy %9.5f%%",  (100 - 100.0*(double)(MajorityVote3Mispred)/MajorityVote3Used));
printf("\n");
printf("\nTaken PHT was used %9.5f%%",  (100.0*(double)(TakenPhtUsed)/NumBranches));
printf("\nNot Taken PHT was used %9.5f%%",  (100.0*(double)(NotTakenPhtUsed)/NumBranches));
printf("\nTaken PHT accurancy %9.5f%%",  (100 - 100.0*(double)(TakenPhtMispred)/TakenPhtUsed));
printf("\nNot Taken PHT accurancy %9.5f%%",  (100 - 100.0*(double)(NotTakenPhtMispred)/NotTakenPhtUsed));
printf("\n");

printf("\nBimodal accurancy %9.5f%%", 100 - 100.0*(double)(bimodalMispred)/NumBranches);

printf("\nPercentage of mispredictions caused by Taken PHT 1 %9.5f%%",   (100 - 100.0*(double)(phtT1mispred)/(NumMispred)));
printf("\nPercentage of mispredictions caused by Taken PHT 2 %9.5f%%",   (100 - 100.0*(double)(phtT2mispred)/(NumMispred)));
printf("\nPercentage of mispredictions caused by Not Taken PHT 1 %9.5f%%",   (100 - 100.0*(double)(phtNT1mispred)/(NumMispred)));
printf("\nPercentage of mispredictions caused by Not Taken PHT 2 %9.5f%%",   (100 - 100.0*(double)(phtNT2mispred)/(NumMispred)));
printf("\nAccurancy %9.5f%%",   100.0*(double)(bimodalMispred)/(NumBranches));
printf("\n");


	return;
}

/***********************************************************/
#endif