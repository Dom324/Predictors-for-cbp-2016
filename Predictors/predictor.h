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

#define LOGPRED 16
#ifndef NR
#define NR 31
#endif
#define PREDICTOR_H_SEEN
#define PHT_CTR_INIT 2
#define PC_SHFT	2

#include <cstddef>
#include <inttypes.h>
#include <vector>

static char GOG1[1 << (LOGPRED - 1)];
/* GOG1: shared prediction tables for G0 and G1*/
static char BIMMETA[1 << (LOGPRED - 2)];
/*BIMMETA: shared prediction tables for  BIM and META*/
static char HYST[1 << (LOGPRED - 2)];
/* HYST: shared hysteresis tables */


static const int L_BIM = 2;
static const int L_G0 = 17;
static const int L_G1 = 43;
static const int L_META = 6;
static int LOGSIZE;


class PREDICTOR{


 private:
 
  UINT64  ghr;           // global history register

 public:


  PREDICTOR(void);
  
  bool    GetPrediction(UINT64 PC);  
  void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

};
int INDEX (long long Add,  long long histo, int m, int funct);
int F1 (long long a);
int F2 (long long a);
int F3 (long long a);
int F4 (long long a);
int Hi (long long a);
int Hi (long long a);
void PrintStat();




PREDICTOR::PREDICTOR(void){

  for(UINT32 ii=0; ii< (1 << (LOGPRED - 1)); ii++){
    GOG1[ii]=PHT_CTR_INIT; 
  }
  
  for(UINT32 ii=0; ii< (1 << (LOGPRED - 2)); ii++){
    BIMMETA[ii]=PHT_CTR_INIT; 
  }
  
  for(UINT32 ii=0; ii< (1 << (LOGPRED - 2)); ii++){
    HYST[ii]=PHT_CTR_INIT; 
  }

}

bool   PREDICTOR::GetPrediction(UINT64 PC){

    int add,indexbim, indexg1, indexg0, indexmeta,  NUMHYST;
    char pg0, pg1, pbim, pmeta;
    bool prediction = false;
    long long Add;
    

 long long GHIST;
	add = PC >> PC_SHFT;
	add = add;
        NUMHYST = ((add ^ (int (ghr))) & 3);
        Add = (long long) add;
        GHIST =  ghr  ^ ((ghr & 3) << 5);

	LOGSIZE = LOGPRED - 3;
	indexg0 = (INDEX (Add, GHIST, L_G0, 1) << 2) + (NUMHYST);
	indexg1 = (INDEX (Add, GHIST, L_G1, 2) << 2) + (NUMHYST ^ 1);

	LOGSIZE = LOGPRED - 4;
	indexbim = (INDEX (Add, GHIST, L_BIM, 3) << 2) + (NUMHYST ^ 2);
	indexmeta = (INDEX (Add, GHIST, L_META, 4) << 2) + (NUMHYST ^ 3);

	pg0 = GOG1[indexg0];
	pg1 = GOG1[indexg1];
	pbim = BIMMETA[indexbim];
	pmeta = BIMMETA[indexmeta];
	if (pmeta)
	  prediction = ((pbim + pg0 + pg1) > 1);
	else
	  prediction = (pbim > 0);
      
    return prediction;		// true for taken, false for not taken
}

void  PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){

    int add, indexbim, indexg1, indexg0, indexmeta,  NUMHYST;
    char pg0, pg1, pbim, pmeta, PESKEW,outcome;
    bool peskew, prediction, psmall;
 long long Add;
 
 
	/* first recompute the prediction */
 long long GHIST;
	add = PC >> PC_SHFT;
	add = add;
        NUMHYST = ((add ^ (int (ghr))) & 3);
        Add = (long long) add;
        GHIST =  ghr  ^ ((ghr & 3) << 5);

	LOGSIZE = LOGPRED - 3;
	indexg0 = (INDEX (Add, GHIST, L_G0, 1) << 2) + (NUMHYST);
	indexg1 = (INDEX (Add, GHIST, L_G1, 2) << 2) + (NUMHYST ^ 1);
/*indexg0, indexg1 are smaller than 2**(LOGPRED-1)*/

	LOGSIZE = LOGPRED - 4;
	indexbim = (INDEX (Add, GHIST, L_BIM, 3) << 2) + (NUMHYST ^ 2);
	indexmeta = (INDEX (Add, GHIST, L_META, 4) << 2) + (NUMHYST ^ 3);
/*indexg0, indexg1 are smaller than 2**(LOGPRED-2)*/
	pg0 = GOG1[indexg0];
	pg1 = GOG1[indexg1];
	pbim = BIMMETA[indexbim];
	pmeta = BIMMETA[indexmeta];
	PESKEW = pbim + pg0 + pg1;
	peskew = ((pbim + pg0 + pg1) > 1);
	psmall = (pbim > 0);

	if (pmeta)
             prediction = peskew;
        	else
             prediction = psmall;
        
/*recompute the complete counter values*/
	pg0 = (pg0 << 1) + HYST[indexg0 & ((1 << (LOGPRED - 2)) - 1)];
	pg1 = (pg1 << 1) + HYST[indexg1 & ((1 << (LOGPRED - 2)) - 1)];
	pbim = (pbim << 1) + HYST[indexbim];
	pmeta = (pmeta << 1) + HYST[indexmeta];
/*      pg0 ^= 1;
        pg1 ^= 1;
        pbim ^= 1;
        pmeta ^=1;*/
        
	if (resolveDir)
	  outcome = 1;
	else
	  outcome = 0;
	/*always easier to manipulate integers than booleans */
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
	else if (PESKEW != 3 * outcome)
	  {
	    if ((pbim & 2) == 2 * outcome)
	      {
		pbim = 3 * outcome;
	      }
	    else if (prediction != resolveDir)
	      pbim = (pbim & 1) + 1;

	    if (peskew != psmall)
	      {
		if (peskew == outcome)
		  {
		    pmeta++;
		    if (pmeta > 3)
		      pmeta = 3;
		  }
		else
		  {
		    pmeta--;
		    if (pmeta < 0)
		      pmeta = 0;
		  }
	      }

	    if ((pmeta > 1) | (prediction != resolveDir))
	      {
		if ((pg1 & 2) == 2 * outcome)
		  {
		    pg1 = 3 * outcome;
		  }
		else if (prediction != resolveDir)
		  pg1 = (pg1 & 1) + 1;
	      }

	    if ((pmeta > 1) | (prediction != resolveDir))
	      {
		if ((pg0 & 2) == 2 * outcome)
		  {

		    pg0 = 3 * outcome;
		  }
		else if (prediction != resolveDir)
		  pg0 = (pg0 & 1) + 1;
	      }
	  }

/*	pg0 ^= 1;
        pg1 ^= 1;
        pbim ^= 1;
        pmeta ^=1;*/
        HYST[indexg0 & ((1 << (LOGPRED - 2)) - 1)] = (pg0 & 1);
	HYST[indexg1 & ((1 << (LOGPRED - 2)) - 1)] = (pg1 & 1);
	HYST[indexbim] = (pbim & 1);
	HYST[indexmeta] = (pmeta & 1);
	GOG1[indexg0] = (pg0 >> 1) & 1;
	GOG1[indexg1] = (pg1 >> 1) & 1;
	BIMMETA[indexbim] = (pbim >> 1) & 1;
	BIMMETA[indexmeta] = (pmeta >> 1) & 1;

	ghr = ghr << 1;
	if (resolveDir)
	  ghr++;


}


void    PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

  return;
}

int
H (long long a)

{
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

 int
Hi (long long a)

{
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

 int
F1 (long long a)
{
 long long res ;
  res = (H (a) ^ Hi (a >> LOGSIZE) ^ (a >> LOGSIZE)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

 int
F2 (long long a)

{
 long long res ;


  res = (H (a) ^ Hi (a >> LOGSIZE) ^ (a)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

 int
F3 (long long a)

{
 long long res ;

  res = (Hi (a) ^ H (a >> LOGSIZE) ^ (a >> LOGSIZE)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}
 int
F4 (long long a)

{
 long long res ;

  res = (Hi (a) ^ H (a >> LOGSIZE) ^ (a)) & ((1 << LOGSIZE) - 1);
  return (int (res));
}

 int
INDEX (long long Add,  long long histo, int m, int funct)
{
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

void PrintStat(){
	
	return;
}
/***********************************************************/
#endif

