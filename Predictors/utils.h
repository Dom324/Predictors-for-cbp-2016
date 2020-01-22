///////////////////////////////////////////////////////////////////////
//  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
///////////////////////////////////////////////////////////////////////


#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;

#define UINT32      unsigned int
#define INT32       int
#define UINT64      unsigned long long
#define COUNTER     unsigned long long


#define NOT_TAKEN 0
#define TAKEN 1

#define FAILURE 0
#define SUCCESS 1

//JD2_2_2016
//typedef enum {
//  OPTYPE_OP               =2,
//  OPTYPE_BRANCH_COND      =3,
//  OPTYPE_RET              =4,
//  OPTYPE_BRANCH           =6,
//  OPTYPE_INDIRECT         =7,
//  OPTYPE_MAX              =8
//}OpType;

//JD2_17_2016 break down types into COND/UNCOND
typedef enum {
  OPTYPE_OP               =2,

  OPTYPE_RET_UNCOND,
  OPTYPE_JMP_DIRECT_UNCOND,
  OPTYPE_JMP_INDIRECT_UNCOND,
  OPTYPE_CALL_DIRECT_UNCOND,
  OPTYPE_CALL_INDIRECT_UNCOND,

  OPTYPE_RET_COND,
  OPTYPE_JMP_DIRECT_COND,
  OPTYPE_JMP_INDIRECT_COND,
  OPTYPE_CALL_DIRECT_COND,
  OPTYPE_CALL_INDIRECT_COND,

  OPTYPE_ERROR,

  OPTYPE_MAX
}OpType;



static inline UINT32 SatIncrement(UINT32 x, UINT32 max)
{
  if(x<max) return x+1;
  return x;
}

static inline UINT32 SatDecrement(UINT32 x)
{
  if(x>0) return x-1;
  return x;
}

static inline UINT32 H(UINT32 v1, UINT32 VECTOR_LEN){
	
	int W;
	W = (v1 >> (VECTOR_LEN - 1));
	W = (W ^ (v1 % 2)) << (VECTOR_LEN - 1);
	W = W + (v1 % (1 << (VECTOR_LEN - 1)));
	
	W = W % (1 << VECTOR_LEN);
	
	return W;
}

static inline UINT32 H_inversed(UINT32 v1, UINT32 VECTOR_LEN){
	
	int W, k;
	
	if(((v1 >> (VECTOR_LEN - 1)) & 1) == ((v1 >> (VECTOR_LEN - 2)) & 1)){
		
		k = 0;
		
	}
	else{
		
		k = 1;
		
	}
	
	W = (v1 << 1) + k;
	
	W = W % (1 << VECTOR_LEN);
	
	return W;
}



void print_byte(uint8_t byte)
{
	const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
    };
    printf("%s %s \n", bit_rep[byte], bit_rep[byte & 0x0F]);
}

int updateCounter(int x, int updateDir, int CNTR_MAX){
	
	int SHFT_AMOUNT = 0, DirBit, Strenght;
	while((CNTR_MAX >> SHFT_AMOUNT) != 1){
		SHFT_AMOUNT++;
	}

    DirBit = x >> SHFT_AMOUNT;
    Strenght = x - ((x >> SHFT_AMOUNT) << SHFT_AMOUNT);
		
		if(updateDir != DirBit){
			
			if(Strenght == 0){
			    DirBit = !DirBit;
			}
			else{
			    Strenght--;
			}
			
		}
        else{
		
		    if(Strenght != ((1 << SHFT_AMOUNT) - 1)){
			    Strenght++;
		    }
        }
	
	return (DirBit << SHFT_AMOUNT) + Strenght;
}

int updateCounterDealised_1a(int bimodalCounter, int resolveDir, int CNTR_MAX){
	
	int DirBit = bimodalCounter >> 2;
	
	if(resolveDir == DirBit){
		
		if((bimodalCounter & 3) != 3) bimodalCounter++;
		
	}
	else{

	    if((bimodalCounter & 3) == 1){
		
		    bimodalCounter = (!DirBit) << 2;
		
	    }
	    else{
	        if((bimodalCounter & 3) == 0) bimodalCounter++;
	        else bimodalCounter--;
	    }
  
    }
	
	return bimodalCounter;
	
}

int updateCounterDealised_1b(int bimodalCounter, int resolveDir, int CNTR_MAX){
	
	int DirBit = bimodalCounter >> 2;
	
	if(resolveDir == DirBit){
		
		if((bimodalCounter & 3) != 3) bimodalCounter++;
		
	}
	else{

	    if((bimodalCounter & 3) == 0){
		
		    bimodalCounter = ((!DirBit) << 2) + 1;
		
	    }
	    else{
	        bimodalCounter--;
	    }
  
    }
	return bimodalCounter;
	
}

#endif