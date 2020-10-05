#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "FetchStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Memory.h"
#include "Tools.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages){
   	F * freg = (F *) pregs[FREG];
   	D * dreg = (D *) pregs[DREG];   	
   	M * mreg = (M *) pregs[MREG];
   	W * wreg = (W *) pregs[WREG];
   	E * ereg = (E *) pregs[EREG];
   	DecodeStage * ds = (DecodeStage *) stages[DSTAGE];
	ExecuteStage * ex = (ExecuteStage *) stages[ESTAGE];

   	uint64_t f_pc = 0, icode = INOP, ifun = FNONE, valC = 0, valP = 0;
   	uint64_t rA = RNONE, rB = RNONE, f_stat = SAOK;
   

   	f_pc = selectPC(mreg->getCnd()->getOutput(), mreg->getvalA()->getOutput(), wreg->getvalM()->getOutput(), 
                   freg->getpredPC()->getOutput(), mreg->geticode()->getOutput(), wreg->geticode()->getOutput());
   
   	bool wrong = false;
   	Memory * mem = Memory::getInstance();
   	uint64_t rec = mem->getByte(f_pc, wrong);
   	icode = Tools::getBits(rec, 4, 7);
   	ifun = Tools::getBits(rec, 0, 3);

   	bool requiresC = need_valC(icode);
   	bool requiresreg = need_regids(icode); // if this is true then 
	if(requiresreg){
		//call get byte with fpc + 1
		uint64_t regs = mem->getByte(f_pc + 1, wrong);
		//pass in ra, rb as reference (&), so that if you modify them in the method it modifies them in doClockLow
		getRegIds(regs, rA, rB);
	}
	if(requiresC){
		buildValC(valC, requiresreg, wrong, f_pc);
	}
	icode = wrong? INOP: icode;
	ifun = wrong? FNONE: ifun;
	bool instr_valid = isinstr_valid(icode);
	f_stat = setf_stat(wrong, instr_valid, icode);
	
	F_stall = setF_stall(ereg->geticode()->getOutput(), ereg->getdstM()->getOutput(), ds->getd_srcA(), ds->getd_srcB(), 
			dreg->geticode()->getOutput(), mreg->geticode()->getOutput());
	D_stall = setD_stall(ereg->geticode()->getOutput(), ereg->getdstM()->getOutput(), ds->getd_srcA(), ds->getd_srcB());
	D_bubble = setD_bubble(ereg->geticode()->getOutput(), ex->geteCndE(), ereg->getdstM()->getOutput(), 
			dreg->geticode()->getOutput(), mreg->geticode()->getOutput(), ds->getd_srcA(), ds->getd_srcB());
   	valP = PCincrement(f_pc, requiresC, requiresreg);

   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.
   
   //The value passed to setInput below will need to be changed
   	freg->getpredPC()->setInput(predictPC(valC, valP, icode));

   //provide the input values for the D register
   	setDInput(dreg, f_stat, icode, ifun, rA, rB, valC, valP);
   	return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
	F * freg = (F *) pregs[FREG];
   	D * dreg = (D *) pregs[DREG];

   	if(!F_stall){
   		freg->getpredPC()->normal();
   	}
   	if(D_bubble){
		dreg->getstat()->bubble(SAOK);
   		dreg->geticode()->bubble(INOP);
   		dreg->getifun()->bubble();
   		dreg->getrA()->bubble(RNONE);
   		dreg->getrB()->bubble(RNONE);
   		dreg->getvalC()->bubble();
   		dreg->getvalP()->bubble();
	}else if(!D_stall){
   		dreg->getstat()->normal();
   		dreg->geticode()->normal();
   		dreg->getifun()->normal();
   		dreg->getrA()->normal();
   		dreg->getrB()->normal();
   		dreg->getvalC()->normal();
   		dreg->getvalP()->normal();
	}
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
	dreg->getstat()->setInput(stat);
   	dreg->geticode()->setInput(icode);
   	dreg->getifun()->setInput(ifun);
   	dreg->getrA()->setInput(rA);
   	dreg->getrB()->setInput(rB);
   	dreg->getvalC()->setInput(valC);
   	dreg->getvalP()->setInput(valP);
}

uint64_t FetchStage::selectPC(uint64_t M_Cnd, uint64_t M_valA, uint64_t W_valM, uint64_t F_predPC,
                              uint64_t M_icode, uint64_t W_icode)
{
    if (M_icode == IJXX && !M_Cnd)
    {
        return M_valA;
    }
    else if (W_icode == IRET)
    {
        return W_valM;
    }
    return F_predPC;
}

bool FetchStage::need_regids(uint64_t f_icode){
    if (f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ || f_icode == IPOPQ || 
        f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ)
    {
    	//if icode is IRMOVQ get regids and build initialize them to input to the d register
    	//if(f_icode == IRMOVQ){
    	//	getRegIds(f_icode);
    	//}
        return true;
    }
    return false;
}

// if true read the register byte and initialize rA and rB to the appropriate bits in the register byte, then use them as an input to the D register
void FetchStage::getRegIds(uint64_t regs, uint64_t &rA, uint64_t &rB){

	//this will set rA to the correct register, or NOREG
	rA = Tools::getBits(regs, 4, 7); // since its pulling the byte will it know thats its a NOREG
	// this will set rB to the correct register
	rB = Tools::getBits(regs, 0, 3);
	//these will be used as input to the d register.
}
bool FetchStage::need_valC(uint64_t f_icode){
    if (f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || 
        f_icode == IJXX || f_icode == ICALL){
        return true;
    }
    return false;
}
void FetchStage::buildValC(uint64_t &valC, bool requiresreg, bool &wrong, uint64_t f_pc){
	//this is where i do the building of valc :)
	uint8_t V[8];
    Memory * mem = Memory::getInstance();
	//call getByte 8 times
	if(requiresreg){
		for(int i = 0; i < 8; i++){
			V[i] = mem->getByte(f_pc + i + 2, wrong);
		}
	}else{
		for(int i = 0; i < 8; i++){
			V[i] = mem->getByte(f_pc + i + 1, wrong);
		}
	}
	//use build Long to build the long out of the bytes
	valC = Tools::buildLong(V);
		// it starts a fpc + 1 or fpc + 2
		
		//write a function that returns uint64_t	
}

uint64_t FetchStage::predictPC(uint64_t f_valC, uint64_t f_valP, uint64_t f_icode){
    if (f_icode == IJXX || f_icode == ICALL){
        return f_valC;
    }
    return f_valP;
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool uno, bool dos){
    if (uno){
        f_pc = f_pc + 8;
    }
    if (dos){
        f_pc = f_pc + 1;
    }
    f_pc = f_pc + 1;
    return f_pc;
}

bool FetchStage::isinstr_valid(uint64_t f_icode){
    if(f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ || f_icode == IIRMOVQ || 
    	f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IOPQ || f_icode == IJXX ||
    	f_icode == ICALL || f_icode == IRET || f_icode == IPUSHQ || f_icode == IPOPQ){
    	return true;
    }
    return false;
}


uint64_t FetchStage::setf_stat(bool &mem_error, bool instr_valid, uint64_t f_icode){
    if(mem_error){
        return SADR;
    }else if(f_icode == IHALT){
        return SHLT;
    }else if(!instr_valid){
        return SINS;
    }else{
        return SAOK;
    }
}


bool FetchStage::setF_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB, uint64_t d_icode, uint64_t m_icode){
    if( (d_icode == IRET || m_icode == IRET || E_icode == IRET) || ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB))){
    	return true;
    }
    return false;
    
}

bool FetchStage::setD_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB){
    if( (E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)){
    	return true;
    }
    return false;
}


bool FetchStage::setD_bubble(uint64_t e_icode, uint64_t e_Cnd, uint64_t E_dstM, uint64_t d_icode, uint64_t m_icode, uint64_t d_srcA, uint64_t d_srcB){
    //E_icode == IJXX && !e_Cnd;
    if((e_icode == IJXX && !e_Cnd) || 
    (!((e_icode == IMRMOVQ || e_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) 
    && (e_icode == IRET || d_icode == IRET || m_icode == IRET))){
    	return true;
    }
    return false;
}


