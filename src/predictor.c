//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "SANANYA MAJUMDER & DEBADITYA BASU ";
const char *studentID   = "PID";
const char *email       = "EMAIL";


//------------------------------------//
//      Hybrid Perceptron Configuration       //
//------------------------------------//
#define HYBRID_PERCEPTRON 1 

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//             Debug                  //
//------------------------------------//
static int count_local = 0;
static int count_global = 0;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

uint32_t *gsPht;
uint32_t gsMask;

uint32_t *gPht;
uint32_t *lPht;
uint32_t *lht;
uint32_t *cPht;

uint32_t ghistory;
uint32_t pcMask;
uint32_t gMask;
uint32_t lMask;

//-------------------------------------//
//      Custom Bits Declaration        //
//-------------------------------------//

short int **weights;
int custom_lhistoryBits;

int custom_pcIndexBits;

uint8_t *custom_history;
uint32_t custom_pcMask;
uint32_t custom_lMask;
uint32_t custom_gMask;
uint32_t custom_ghistoryBits;
#ifdef HYBRID_PERCEPTRON
uint32_t custom_ghistory;
#endif


//-------------------------------------//
//      Custom Global Variables        //
//-------------------------------------//

uint32_t custom_btableSize;
uint32_t custom_pcbits;
uint32_t custom_pcindex;
uint32_t custom_ghistbits;



//------------------------------------//
//        Custom M Share Variables    //
//------------------------------------//
uint32_t custom_ghistbits_indexing ;
uint32_t custom_lhistbits_indexing ;
uint32_t custom_pcbits_indexing ;

uint32_t custom_gMask_index;
uint32_t custom_lMask_index;
uint32_t custom_pcMask_index;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

uint32_t get_required_bits(int length)                                      
{
    uint32_t r_bits=0;
    for(int i=0;i<length;i++)
    {
        r_bits = r_bits | (1<<i);
    }
    
return r_bits;
}

void init_predictor()
{
    
    ghistory = 0;
    custom_ghistory = 0;

    switch(bpType) {
      case STATIC:
            break;
      case GSHARE:
        gsMask = get_required_bits(ghistoryBits);
        uint32_t gstableSize;
        gstableSize = 1<<ghistoryBits;                                  //Get The  Size of Gshare Prediction History Table
        gsPht = (uint32_t*) malloc(sizeof(uint32_t)*gstableSize);
        for(int i=0;i<gstableSize;i++)
        {
          gsPht[i] = 1;                                                 //Initializing Gshare Prediction History Table Entries to WEAKLY_NOT_TAKEN
        }
        break;
      case TOURNAMENT:
        gMask = get_required_bits(ghistoryBits);
        lMask = get_required_bits(lhistoryBits);
        pcMask = get_required_bits(pcIndexBits);
        uint32_t btableSize;
        uint32_t gtableSize;
        uint32_t ltableSize;
        uint32_t ctableSize;

        btableSize = 1<<pcIndexBits;
        lht = (uint32_t*) malloc(sizeof(uint32_t)*btableSize);          //Local History Table Size Allocation
        for(int i=0;i<btableSize;i++)
        {
          lht[i] = 0;                                                    //Initializing History of Each Entries to Not Taken
        }
       
        ltableSize = 1<<lhistoryBits;
        lPht = (uint32_t*) malloc(sizeof(uint32_t)*ltableSize);           //Local Prediction History Table Size Allocation
        for(int i=0;i<ltableSize;i++)
        {
          lPht[i] = 1;                                                    //Initializing Local Prediction History Table Entries to WEAKLY_NOT_TAKEN
        }
       
        gtableSize = 1<<ghistoryBits;
        gPht = (uint32_t*) malloc(sizeof(uint32_t)*gtableSize);           //Global PRediction History Size Allocation
        for(int i=0;i<gtableSize;i++)
        {
          gPht[i] = 1;                                                    //Initializing Global Prediction History Table Entries to WEAKLY_NOT_TAKEN
        }

        ctableSize = 1<<ghistoryBits;                                     
        cPht = (uint32_t*) malloc(sizeof(uint32_t)*ctableSize);           //Choice Prediction Table Size Allocation
        for(int i=0;i<ctableSize;i++)
        {
          cPht[i] = 2;                                                     //Initializing Choice Prediction Table Entries to WEAKLY Select Global Predictor
        }
        break;
      case PERCEPTRON:
        ;
        custom_pcIndexBits = 9;                                          
        custom_lhistoryBits = 15;                                           //number of bits in the history register(SHOULD BE GLOBAL HISTORY)
        custom_pcMask = get_required_bits(custom_pcIndexBits);            //Creating Mask for PC (TO select the entries in perceptron table)
                                           
            
        custom_history = (uint8_t*) malloc(sizeof(uint8_t)*custom_lhistoryBits);         //Input to perceptron Model (Global History Register)

        for(int i=0; i<custom_lhistoryBits;i++)
            custom_history[i]=0;
            
        custom_btableSize = 1<<(custom_pcIndexBits);                                                //change table size based on budget
        weights = (short int**)malloc(custom_btableSize * sizeof(short int*));            
        
        for (int i = 0; i < custom_btableSize; i++)
            weights[i] = (short int*)malloc((custom_lhistoryBits+1) * sizeof(short int));


        for(int i=0; i<custom_btableSize; i++)
        {
            for(int j=0; j<custom_lhistoryBits; j++)
                weights[i][j]=0;
            
            weights[i][custom_lhistoryBits]=1;
        }
            
            break;


        case CUSTOM:

         ;
        ghistoryBits = 13;
        pcIndexBits = 10;
        lhistoryBits = 11;
        custom_gMask = get_required_bits(ghistoryBits);
        custom_lMask = get_required_bits(lhistoryBits);
        custom_pcMask = get_required_bits(pcIndexBits);
       
        uint32_t custom_gtableSize;
        uint32_t custom_ltableSize;
        uint32_t custom_ctableSize;
       
        custom_btableSize = 1<<pcIndexBits;
        lht = (uint32_t*) malloc(sizeof(uint32_t)*custom_btableSize);          //Local History Table Size Allocation
        for(int i=0;i<custom_btableSize;i++)
        {
          lht[i] = 0;                                                    //Initializing History of Each Entries to Not Taken
        }
        
        custom_ltableSize = 1<<lhistoryBits;
        lPht = (uint32_t*) malloc(sizeof(uint32_t)*custom_ltableSize);           //Local Prediction History Table Size Allocation
        for(int i=0;i<custom_ltableSize;i++)
        {
          lPht[i] = 1;                                                    //Initializing Local Prediction History Table Entries to WEAKLY_NOT_TAKEN
        }
        
        custom_gtableSize = 1<<ghistoryBits;
        gPht = (uint32_t*) malloc(sizeof(uint32_t)*custom_gtableSize);           //Global PRediction History Size Allocation
        for(int i=0;i<custom_gtableSize;i++)
        {
          gPht[i] = 1;                                                    //Initializing Global Prediction History Table Entries to WEAKLY_NOT_TAKEN
        }

        custom_ctableSize = 1<<ghistoryBits;                                     
        cPht = (uint32_t*) malloc(sizeof(uint32_t)*custom_ctableSize);           //Choice Prediction Table Size Allocation
        for(int i=0;i<custom_ctableSize;i++)
        {
          cPht[i] = 3;                                                     //Initializing Choice Prediction Table Entries to WEAKLY Select Localclea Predictor
        }
        
            break;

        
        
      default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
    uint32_t prediction;

  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
        ;
        uint32_t pcbits;
        uint32_t gshistbits;
        uint32_t index;
          
        pcbits = pc & gsMask;
        gshistbits = ghistory & gsMask;
        index = gshistbits ^ pcbits;
        prediction = gsPht[index];
        if(prediction>1)
            return TAKEN;
        else
            return NOTTAKEN;
        break;
    case TOURNAMENT:
        ;
        uint32_t choice;
        uint32_t pcindex;
        uint32_t lhistbits;
        uint32_t ghistbits;
          
        ghistbits = ghistory & gMask;
        choice = cPht[ghistbits];
        
        if(choice<2)
        {
        pcindex = pcMask & pc;
        lhistbits = lMask & lht[pcindex];
        prediction = lPht[lhistbits];
        }
        else
        {
        prediction = gPht[ghistbits];
        }
        
        
        if(prediction>1)
            return TAKEN;
        else
            return NOTTAKEN;
          break;
    case PERCEPTRON: 
        ;
        
        int32_t custom_prediction = 0;
        
        
        

        #ifdef HYBRID_PERCEPTRON
        //***********************************************//
        //         Variables for Hybrid Selection        //
        custom_ghistbits =  custom_ghistory & custom_pcMask;
        custom_pcbits = pc & custom_pcMask;
        custom_pcindex = custom_pcbits ^ custom_ghistbits;
        
        //***********************************************//
        #endif
        
        #ifndef HYBRID_PERCEPTRON
        custom_pcindex = custom_pcMask & pc;                      //Actual working one without hybrid
        #endif

        for(int i=0; i<custom_lhistoryBits; i++)
        {
            if(custom_history[i]>0)
                custom_prediction = custom_prediction + weights[custom_pcindex][i];
            else
                custom_prediction = custom_prediction - weights[custom_pcindex][i];
        }                                                       //check where bias needs to be added
        custom_prediction = custom_prediction + weights[custom_pcindex][custom_lhistoryBits];
        
        if(custom_prediction>1)
            return TAKEN;
        else
            return NOTTAKEN;
        
        break;

        case CUSTOM:
        ;
        
        uint32_t custom_choice;
        uint32_t custom_lhistbits;
         
        uint32_t custom_index;
        

        custom_ghistbits = ghistory & custom_gMask;
        custom_pcbits = pc & custom_gMask;
        custom_index = custom_pcbits ^ custom_ghistbits;
        custom_choice = cPht[custom_index];
        
        if(custom_choice<4)
        {
        custom_pcindex = custom_pcMask & pc;
        custom_lhistbits = custom_lMask & lht[custom_pcindex];
        prediction = lPht[custom_lhistbits];
        count_local++;
        }
        else
        {
        prediction = gPht[custom_index];
        count_global++;
        }
        
        //printf("Count of choice taken local branch=%d global_branch=%d \n",count_local,count_global);
        if(prediction>1)
            return TAKEN;
        else
            return NOTTAKEN;
        break;  

        

          default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  
    uint8_t prediction = make_prediction(pc);

    switch(bpType) {
      case STATIC:
            break;
      case GSHARE:
        ;
        uint32_t pcbits;
        uint32_t gshistbits;
        uint32_t index;

            
        pcbits = pc & gsMask;
        gshistbits = ghistory & gsMask;
        index = gshistbits ^ pcbits;
            
        if(outcome==TAKEN)
        {
          if(gsPht[index]<3)
            gsPht[index]++;
        }
        else
        {
          if(gsPht[index]>0)
            gsPht[index]--;
        }
        ghistory = (ghistory<<1) | outcome;
        ghistory = ghistory & gsMask;
        return;
        break;
      case TOURNAMENT:
        ;
        uint32_t choice;
        uint32_t pcindex;
        uint32_t lhistbits;
        uint32_t ghistbits;
        uint32_t lprediction;
        uint32_t gprediction;
            
              
        ghistbits = ghistory & gMask;
        choice = cPht[ghistbits];
        pcindex = pcMask & pc;
        lhistbits = lMask & lht[pcindex];
            
        lprediction = lPht[lhistbits];
        if(lprediction>1)
          lprediction = TAKEN;
        else
          lprediction = NOTTAKEN;

        gprediction = gPht[ghistbits];
        if(gprediction>1)
          gprediction = TAKEN;
        else
          gprediction = NOTTAKEN;
    
        if((gprediction == outcome) && (lprediction != outcome))
        {
            if(cPht[ghistbits]<3)
                cPht[ghistbits]++;
        }
        else if((lprediction == outcome) && (gprediction !=outcome))
        {
            if(cPht[ghistbits]>0)
                cPht[ghistbits]--;
        }
        if(outcome==TAKEN)
        {
          if(gPht[ghistbits]<3)
            gPht[ghistbits]++;
            
          if(lPht[lhistbits]<3)
            lPht[lhistbits]++;
        }
        else
        {
          if(gPht[ghistbits]>0)
            gPht[ghistbits]--;
            
          if(lPht[lhistbits]>0)
            lPht[lhistbits]--;
        }
       
        ghistory = ((ghistory<<1) | outcome);
        ghistory = ghistory & gMask;
    
        lht[pcindex] = ((lht[pcindex]<<1) | outcome);
        lht[pcindex] = lht[pcindex] & lMask;
        break;
    case PERCEPTRON:
        ;
        
        int32_t custom_prediction = 0;
        uint32_t theta;
        uint32_t custom_outcome;
        uint8_t sign_outcome;    
        uint8_t sign_custom_prediction;

        #ifdef HYBRID_PERCEPTRON
        //********************************
        //  Adding for hybrid indexing of perceptront table
        
        

        custom_ghistbits =  custom_ghistory & custom_pcMask;
        custom_pcbits = pc & custom_pcMask;
        custom_pcindex = custom_pcbits ^ custom_ghistbits;
        //*********************************
        #endif

        #ifndef HYBRID_PERCEPTRON
        custom_pcindex = custom_pcMask & pc;                      //Working code without Hybrid 
        #endif

        theta = 1.93 * lhistoryBits + 14;
            
        for(int i=0; i<custom_lhistoryBits; i++)
        {
            if(custom_history[i]>0)
                custom_prediction = custom_prediction + weights[custom_pcindex][i];
            else
                custom_prediction = custom_prediction - weights[custom_pcindex][i];
        }
            //check where bias needs to be added
          custom_prediction = custom_prediction + weights[custom_pcindex][custom_lhistoryBits];
         
          
          
          if (outcome > 0)
            sign_outcome = 1;
          else
             sign_outcome = -1;

          if(custom_prediction>0)
            sign_custom_prediction = 1;
          else
            sign_custom_prediction = -1;


        if((sign_custom_prediction != sign_outcome) || (abs(custom_prediction)<=theta))
        {
            for(int i=0; i<custom_lhistoryBits; i++)
            {
                
                if(custom_history[i]>0)
                {
                    
                    if(outcome>0)
                      {
                        if(weights[custom_pcindex][i]<127)
                          weights[custom_pcindex][i] = weights[custom_pcindex][i] + 1;
                        else
                          weights[custom_pcindex][i] = 127;
                      }
                    else
                      if(weights[custom_pcindex][i]>-128)  
                        weights[custom_pcindex][i] = weights[custom_pcindex][i] - 1;
                      else
                        weights[custom_pcindex][i] = -128;
                }
                else
                {
                    if(outcome>0)
                      {
                      if(weights[custom_pcindex][i]>-128)  
                        weights[custom_pcindex][i] = weights[custom_pcindex][i] - 1;
                      else
                        weights[custom_pcindex][i] = -128;
                      }
                    else
                        {
                        if(weights[custom_pcindex][i]<127)  
                          weights[custom_pcindex][i] = weights[custom_pcindex][i] + 1;
                        else
                          weights[custom_pcindex][i] = 127;
                        }
                }
            }
        }
            
            for(int i=0; i<custom_lhistoryBits-1; i++)
               {
                custom_history[i]=custom_history[i+1];
              }
               
               custom_history[custom_lhistoryBits-1]=outcome;
              
              #ifdef HYBRID_PERCEPTRON
               custom_ghistory  = (custom_ghistory<<1) | outcome;
               custom_ghistory = custom_ghistory & custom_pcMask;
              #endif
          
          break;
        case CUSTOM:
        ;
        
        uint32_t custom_choice;
        
        uint32_t custom_lhistbits;
       
        uint32_t custom_lprediction;
        uint32_t custom_gprediction;
        uint32_t custom_index;
        
        
        custom_ghistbits = ghistory & custom_gMask;
        custom_pcbits = pc & custom_gMask;
        custom_index = custom_pcbits ^ custom_ghistbits;
        custom_choice = cPht[custom_index];
        
        custom_pcindex = custom_pcMask & pc;
        custom_lhistbits = custom_lMask & lht[custom_pcindex];
            
        custom_lprediction = lPht[custom_lhistbits];
        if(custom_lprediction>1)
          custom_lprediction = TAKEN;
        else
          custom_lprediction = NOTTAKEN;

        custom_gprediction = gPht[custom_index];
        if(custom_gprediction>1)
          custom_gprediction = TAKEN;
        else
          custom_gprediction = NOTTAKEN;
    
        if((custom_gprediction == outcome) && (custom_lprediction != outcome))
        {
            if(cPht[custom_index]<7)
                cPht[custom_index]++;
        }
        else if((custom_lprediction == outcome) && (custom_gprediction !=outcome))
        {
            if(cPht[custom_index]>0)
                cPht[custom_index]--;
        }
        if(outcome==TAKEN)
        {
          if(gPht[custom_index]<3)
            gPht[custom_index]++;
            
          if(lPht[custom_lhistbits]<3)
            lPht[custom_lhistbits]++;
        }
        else
        {
          if(gPht[custom_index]>0)
            gPht[custom_index]--;
            
          if(lPht[custom_lhistbits]>0)
            lPht[custom_lhistbits]--;
        }
       
        ghistory = ((ghistory<<1) | outcome);
        ghistory = ghistory & custom_gMask;
    
        lht[custom_pcindex] = ((lht[custom_pcindex]<<1) | outcome);
        lht[custom_pcindex] = lht[custom_pcindex] & custom_lMask;
        
          break;       

        
    default:
          break;
    }
return;

}
