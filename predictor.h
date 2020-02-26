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

#include <cstddef>
#include <inttypes.h>
#include <vector>

#define LOGPRED 18
#ifndef NR
#define NR 31
#endif

#define L_BIM 2
#define L_G0 17
#define L_G1 43
#define L_META 6
static int LOGSIZE;


//stats

  UINT64  NumMispred = 0, NumBranches = 0;
  
//stats

class PREDICTOR{


 private:
  UINT64  ghr = 0;           // global history register
  UINT32 static const numPhtEntries = 1 << (LOGPRED - 1);
  UINT32 static const numBimMetaEntries = 1 << (LOGPRED - 2);
  UINT32 static const numHystEntries = 1 << (LOGPRED - 2);

  UINT32 pht[numPhtEntries];
  /* GOG1: shared prediction tables for G0 and G1*/
  //UINT32 phtNotTaken[numPhtEntriesNotTaken];
  /* GOG1: shared prediction tables for G0 and G1*/
  UINT32 BIMMETA[numBimMetaEntries];
  /*BIMMETA: shared prediction tables for  BIM and META*/
  UINT32 HYST[numHystEntries];
  /* HYST: shared hysteresis tables */


 public:
 

  PREDICTOR(void);

  // The interface to the functions below CAN NOT be changed
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  

  // Contestants can define their own functions below
};

  void    PrintStat();
  int F1 (long long a);
  int F2 (long long a);
  int F3 (long long a);
  int F4 (long long a);
  int INDEX (long long Add,  long long histo, int m, int funct);


PREDICTOR::PREDICTOR(void){

  ghr              = 0;

  for(UINT32 ii=0; ii< numPhtEntries; ii++){
    pht[ii]=0;
  }

  /*for(UINT32 ii=0; ii< numPhtEntriesNotTaken; ii++){
    phtNotTaken[ii]=0;
  }*/

  for(UINT32 ii=0; ii< numBimMetaEntries; ii++){
    BIMMETA[ii]=0;
  }

  for(UINT32 ii=0; ii< numHystEntries; ii++){
    HYST[ii]=0;
  }
  
  
}


bool   PREDICTOR::GetPrediction(UINT64 PC){
  
    UINT64 add,indexbim, indexg1, indexg0, indexg2, indexg3, indexmeta,  NUMHYST;
    UINT32 pg0, pg1, pbim, pmeta;
    UINT32 prediction;
    UINT64 Add;
    

    UINT64 GHIST;
	add = PC;
	add = (add >> 4) ^ add;
        NUMHYST = ((add ^ (ghr)) & 3);
        Add = add;
        GHIST =  ghr  ^ ((ghr & 3) << 5);
         Add = Add ^ (Add >> 5);

	LOGSIZE = LOGPRED - 3;
	indexg0 = (INDEX (Add, GHIST, L_G0, 1) << 2) + (NUMHYST);
	indexg1 = (INDEX (Add, GHIST, L_G1, 2) << 2) + (NUMHYST ^ 1);
	
	//indexg2 = (INDEX (Add, GHIST, L_G0, 3) << 2) + (NUMHYST ^ 2);
	//indexg3 = (INDEX (Add, GHIST, L_G1, 4) << 2) + (NUMHYST ^ 3);

	LOGSIZE = LOGPRED - 4;
	indexmeta = (INDEX (Add, GHIST, L_META, 4) << 2) + (NUMHYST ^ 3);
	indexbim = (INDEX (Add, GHIST, L_BIM, 3) << 2) + (NUMHYST ^ 2);


	pbim = BIMMETA[indexbim];
	pmeta = BIMMETA[indexmeta];
	
	//if(pbim == 1){
		pg0 = pht[indexg0];
		pg1 = pht[indexg1];
	/*}
	else{
		pg0 = pht[indexg2];
		pg1 = pht[indexg3];
	}*/
	
	if (pmeta)
	  prediction = ((pbim + pg0 + pg1) > 1);
	else
	  prediction = (pbim > 0);



    return prediction;		// true for taken, false for not taken

}


void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){
  
   UINT64 add, indexbim, indexg1, indexg0, indexg2, indexg3, indexmeta,  NUMHYST;
   UINT32 pg0, pg1, pbim, pmeta, PESKEW;
   UINT32 peskew, prediction, psmall;
   UINT64 Add;
   UINT64 GHIST;


   	/* first recompute the prediction */
	add = PC;
	add = (add >> 4) ^ add;
        NUMHYST = ((add ^ (ghr)) & 3);
        Add = add;
        GHIST =  ghr  ^ ((ghr & 3) << 5);
         Add = Add ^ (Add >> 5);

	LOGSIZE = LOGPRED - 3;
	indexg0 = (INDEX (Add, GHIST, L_G0, 1) << 2) + (NUMHYST);
	indexg1 = (INDEX (Add, GHIST, L_G1, 2) << 2) + (NUMHYST ^ 1);
	
	//indexg2 = (INDEX (Add, GHIST, L_G0, 3) << 2) + (NUMHYST ^ 2);
	//indexg3 = (INDEX (Add, GHIST, L_G1, 4) << 2) + (NUMHYST ^ 3);

	LOGSIZE = LOGPRED - 4;
	indexmeta = (INDEX (Add, GHIST, L_META, 4) << 2) + (NUMHYST ^ 3);
	indexbim = (INDEX (Add, GHIST, L_BIM, 3) << 2) + (NUMHYST ^ 2);


	pbim = BIMMETA[indexbim];
	pmeta = BIMMETA[indexmeta];
	
	//if(pbim == 1){
		pg0 = pht[indexg0];
		pg1 = pht[indexg1];
	/*}
	else{
		pg0 = pht[indexg2];
		pg1 = pht[indexg3];
	}*/
	PESKEW = pbim + pg0 + pg1;
	peskew = ((pbim + pg0 + pg1) > 1);
	psmall = (pbim > 0);

	if (pmeta)
             prediction = peskew;
        	else
             prediction = psmall;
        
/*recompute the complete counter values*/

	//if(pbim == 1){
		pg0 = (pg0 << 1) + HYST[indexg0 & ((1 << (LOGPRED - 3)) - 1)];
		pg1 = (pg1 << 1) + HYST[indexg1 & ((1 << (LOGPRED - 3)) - 1)];
	/*}
	else{
		pg0 = (pg0 << 1) + HYST[indexg2 & ((1 << (LOGPRED - 2)) - 1)];
		pg1 = (pg1 << 1) + HYST[indexg3 & ((1 << (LOGPRED - 2)) - 1)];
	}*/
	pbim = (pbim << 1) + HYST[indexbim % (1 << (LOGPRED - 3))];
	pmeta = (pmeta << 1) + HYST[indexmeta % (1 << (LOGPRED - 3))];
/*      pg0 ^= 1;
        pg1 ^= 1;
        pbim ^= 1;
        pmeta ^=1;*/
        


	if ((prediction != resolveDir) & ((random () & NR) == 0))
	  {
/* to break ping-pong phenomena*/
	    if (peskew == psmall)
	      {
		if (resolveDir)
		  {
		    pbim = 2;
		    pg0 = 2;
		    pg1 = 2;
		  }
		else
		  {
		    pbim = 1;
		    pg0 = 1;
		    pg1 = 1;
		  }
	      }

	    else
	      {
		pmeta = (pmeta & 2) ^ 2;
	      }
	  }
	else if (PESKEW != 3 * resolveDir){										// 
		
	    if ((pbim & 2) == 2 * resolveDir)
	      {
		pbim = updateCounter(pbim, resolveDir, 3);
	      }
	    else if (prediction != resolveDir)
	      pbim = updateCounter(pbim, resolveDir, 3);
	  
	  
	  



	    if (peskew != psmall)
	      {
			
			pmeta = updateCounter(pmeta, peskew == resolveDir, 3);
			
		/*if (peskew == resolveDir)
		  {
		    pmeta++;
		    if (pmeta > 3)
		      pmeta = 3;
		  }
		else{
			  
		    if (pmeta != 0)
		      pmeta--;
		    }*/
	      }



	    if ((pmeta > 1) | (prediction != resolveDir))
	      {
			
		if ((pg1 & 2) == 2 * resolveDir)
		  {
		    pg1 = updateCounter(pg1, resolveDir, 3);
		  }
		else if (prediction != resolveDir)
		  pg1 = updateCounter(pg1, resolveDir, 3);
	      }

	    if ((pmeta > 1) | (prediction != resolveDir))
	      {
			
		if ((pg0 & 2) == 2 * resolveDir)
		  {

		    pg0 = updateCounter(pg0, resolveDir, 3);
		  }
		else if (prediction != resolveDir)
		  pg0 = updateCounter(pg0, resolveDir, 3);
	      }
	
	}


/*	pg0 ^= 1;
        pg1 ^= 1;
        pbim ^= 1;
        pmeta ^=1;*/
		
	HYST[indexbim % (1 << (LOGPRED - 3))] = (pbim & 1);
	HYST[indexmeta % (1 << (LOGPRED - 3))] = (pmeta & 1);
	
	//if(pbim >> 1){
		pht[indexg0] = (pg0 >> 1) & 1;
		pht[indexg1] = (pg1 >> 1) & 1;
		
		HYST[indexg0 & ((1 << (LOGPRED - 3)) - 1)] = (pg0 & 1);
		HYST[indexg1 & ((1 << (LOGPRED - 3)) - 1)] = (pg1 & 1);
		

	/*}
	else{
		pht[indexg2] = (pg0 >> 1) & 1;
		pht[indexg3] = (pg1 >> 1) & 1;
		
		HYST[indexg2 & ((1 << (LOGPRED - 2)) - 1)] = (pg0 & 1);
		HYST[indexg3 & ((1 << (LOGPRED - 2)) - 1)] = (pg1 & 1);
	}*/
	
	BIMMETA[indexbim] = (pbim >> 1) & 1;
	BIMMETA[indexmeta] = (pmeta >> 1) & 1;
	
	





    // update the GHR
    ghr = (ghr << 1);

    if(resolveDir == TAKEN){
      ghr++;
    }
	
//My stats
  NumBranches++;

  if(resolveDir != predDir){NumMispred++;};

//My stats
  
}


void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;

}




int H (long long a){
	
 long long res  = 0;
  res = a ^ (a << (LOGSIZE - 1));
  res = res & (1 << (LOGSIZE - 1));
  a = a & ((1 << LOGSIZE) - 1);
  a = a >> 1;
  res = res + a;
  return (int (res));
}

/*********************************************
 inverse fonction Hi
 *********************************************/

int Hi (long long a){
	
 long long res  = 0;
  res = a >> (LOGSIZE - 1);
  res = (res ^ (a >> (LOGSIZE - 2))) & 1;
  a = a & ((1 << (LOGSIZE - 1)) - 1);
  a = a << 1;
  res = res + a;
  return (int (res));
}

/********************************************
skewing functions from PARLE 93 paper
 ********************************************/

int F1 (long long a){
	
 long long res ;
  res = (H (a) ^ Hi (a >> LOGSIZE) ^ (a >> LOGSIZE)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

int F2 (long long a){
	
 long long res ;
  res = (H (a) ^ Hi (a >> LOGSIZE) ^ (a)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

int F3 (long long a){
	
 long long res ;
  res = (Hi (a) ^ H (a >> LOGSIZE) ^ (a >> LOGSIZE)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

int F4 (long long a){
	
 long long res ;
  res = (Hi (a) ^ H (a >> LOGSIZE) ^ (a)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

int INDEX (long long Add,  long long histo, int m, int funct){
	
  long long hm, inter;
  int i;
  int RES;
  
  if (m < 32)
    hm = (histo & ((1 << m) - 1)) + (Add << m);
  else
    {
      if (m != 32)
	{
	  hm = (histo << (64 - m)) ^ (Add);
	}
      else
	hm = ((histo & (0xFFFFFFFF)) << 18) ^ (Add);
    }
  hm = hm ^ (Add << funct) ^ (Add << (10 + funct));
/* incorporate address bits*/
  inter = hm;
  for (i = 0; i < 64; i += (2 * (LOGSIZE - funct) + 1))
    {
      inter = inter >> LOGSIZE;
      inter = inter >> (LOGSIZE - (funct + 1));
      hm = (hm ^ inter);
    }
  switch (funct)
    {
    case 4:
RES= (F4 (hm));
      break;
    case 1:
   RES = (F1 (hm));
      break;

    case 2:
      RES = (F2 (hm));
      break;
    case 3:
      RES = (F3 (hm));
      break;
    default:
      printf (" Problem, inumplemented index function F%d\n", funct);
      exit (1);
    }
  
  return ( RES);
}
//void PrintStat(){}
void PrintStat(){
	
	
//Absolute mispred = actual mispred of a bank, purely theoretical
//Relative mispred = mispred by which the bank contributed to bad prediction	 	 

printf("\n");
printf("\nPredictor_accurancy:%9.5f %%",  100 - (100.0*(double)(NumMispred)/NumBranches));
printf("\n");

printf("\nNumber of bits %d", 1 << LOGPRED);


	return;
}

/***********************************************************/
#endif