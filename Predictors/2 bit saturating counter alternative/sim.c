/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>

int main()
{
    int PHT_CTR_MAX = 7, PHT_CTR_INIT = 5;
    int bimodalCounter = PHT_CTR_INIT;
    int resolveDir;
    
   
  while(1){
    
  if(bimodalCounter > PHT_CTR_MAX/2){
	  
	printf("1   %d  ", bimodalCounter);
	
  }
  else{
	  
	printf("0   %d  ", bimodalCounter);
	
  }
  
  
  
  scanf("%d", &resolveDir);
    
    
    
    
    
    if(resolveDir == 1){

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
  

    return 0;
}
