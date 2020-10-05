#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "MemoryStage.h"


/*
 * doClockLow:
 * Performs the Decode stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    
    uint64_t valA = 0, valB = 0;
    uint64_t dstE = RNONE, dstM = RNONE, srcA = RNONE, srcB = RNONE;
   
    uint64_t stat = dreg->getstat()->getOutput();
    uint64_t icode = dreg->geticode()->getOutput();
    uint64_t ifun = dreg->getifun()->getOutput();
    uint64_t valC = dreg->getvalC()->getOutput();

    bool oops = false;
//src a
	
	srcA = getsrcA(icode, dreg->getrA()->getOutput());
	
//src b
	
	srcB = getsrcB(icode, dreg->getrB()->getOutput());

//dstE
	//word d_dstE = [
	//D_icode in {IRRMOVQ, IIRMOVQ, IOPQ} : D_rB;
	//D_icode in {IPUSHQ, IPOPQ, ICALL, IRET} : RSP;
	//1: RNONE;
	//];
	dstE = getdstE(icode, dreg->getrB()->getOutput());

	
//dstM
	//word d_dstM = [
	//D_icode in {IMRMOVQ, IPOPQ} : D_rA;
	//1: RNONE;
	//];
	dstM = getdstM(icode, dreg->getrA()->getOutput());

//Sel+FwsA
	//word d_valA = [
	//1: d_rvalA;   //value from register file
	//];	
    ExecuteStage * ex = (ExecuteStage *) stages[ESTAGE];
    MemoryStage * ms = (MemoryStage *) stages[MSTAGE];
    RegisterFile * rf = RegisterFile::getInstance();
    valA = getvalA(srcA, dreg, mreg, wreg, ex->getedstE(), 
    		ex->getevalE(), rf->readRegister(srcA, oops), ms->getvalM());
	
	 
//FwdB
	//word d_valB = [
	//1: d_rvalB;  //value from register file
	//];
    valB = getvalB(srcB, dreg, mreg, wreg, ex->getedstE(), ex->getevalE(), 
    		rf->readRegister(srcB, oops), ms->getvalM());
	d_srcA = srcA;
	d_srcB = srcB;
	
	E_bubble = setEBubble(ereg->geticode()->getOutput(), ereg->getdstM()->getOutput(), ex->geteCndE(), srcA, srcB);
	
    setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
    return false;
}

/* doClockHigh
 * 
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
    E * ereg = (E *) pregs[EREG];
	if(E_bubble){
		ereg->getstat()->bubble(SAOK);
    	ereg->geticode()->bubble(INOP);
    	ereg->getifun()->bubble(FNONE);
    	ereg->getvalC()->bubble();
    	ereg->getvalA()->bubble();
    	ereg->getvalB()->bubble();
    	ereg->getdstE()->bubble(RNONE);
    	ereg->getdstM()->bubble(RNONE);
    	ereg->getsrcA()->bubble(RNONE);
    	ereg->getsrcB()->bubble(RNONE);
	}else{
		ereg->getstat()->normal();
    	ereg->geticode()->normal();
    	ereg->getifun()->normal();
    	ereg->getvalC()->normal();
    	ereg->getvalA()->normal();
    	ereg->getvalB()->normal();
    	ereg->getdstE()->normal();
    	ereg->getdstM()->normal();
    	ereg->getsrcA()->normal();
    	ereg->getsrcB()->normal();
	}
    
}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
                            uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, 
                            uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
    ereg->getstat()->setInput(stat);
    ereg->geticode()->setInput(icode);
    ereg->getifun()->setInput(ifun);
    ereg->getvalA()->setInput(valA);
    ereg->getvalC()->setInput(valC);
    ereg->getvalB()->setInput(valB);
    ereg->getdstE()->setInput(dstE);
    ereg->getdstM()->setInput(dstM);
    ereg->getsrcA()->setInput(srcA);
    ereg->getsrcB()->setInput(srcB);
}

uint64_t DecodeStage::getsrcA(uint64_t d_icode, uint64_t d_rA)
{
    if (d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ || 
        d_icode == IPUSHQ)
    {
        return d_rA;
    }
    else if (d_icode == IPOPQ || d_icode == IRET)
    {
        return RSP;
    }
    else 
    {
        return RNONE;
    }
}

uint64_t DecodeStage::getsrcB(uint64_t d_icode, uint64_t d_rB)
{
    if (d_icode == IOPQ || d_icode == IRMMOVQ || d_icode == IMRMOVQ)
    {
        return d_rB;
    }
    else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}

uint64_t DecodeStage::getdstE(uint64_t d_icode, uint64_t d_rB)
{
    if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ)
    {
        return d_rB;
    }
    else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET)
    {
        return RSP;
    }
    else
    {
        return RNONE;
    }
}

uint64_t DecodeStage::getdstM(uint64_t d_icode, uint64_t d_rA)
{
    if (d_icode == IMRMOVQ || d_icode == IPOPQ)
    {
        return d_rA;
    }
    else 
    {
        return RNONE;
    }
}

uint64_t DecodeStage::getvalA(uint64_t d_srcA, D * dreg, M * mreg, W * wreg, uint64_t e_dstE, uint64_t e_valE, uint64_t d_rvalA, uint64_t M_valM){
    
    uint64_t d_icode = dreg->geticode()->getOutput();
    
    if(d_icode == ICALL || d_icode == IJXX){
    	return dreg->getvalP()->getOutput();
    }
    
    if(d_srcA == RNONE){
    	return 0;
    }
    if(d_srcA == e_dstE){
        return e_valE;
    }
    
    uint64_t M_dstM = mreg->getdstM()->getOutput();
    if(d_srcA == M_dstM){
        return M_valM;
	}
	
    uint64_t M_dstE = mreg->getdstE()->getOutput();
    uint64_t M_valE = mreg->getvalE()->getOutput();
    if(d_srcA == M_dstE){
        return M_valE;
    }
    
    uint64_t W_dstM = wreg->getdstM()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();
    if(d_srcA == W_dstM){
    	return W_valM;
    }
    
    uint64_t W_dstE = wreg->getdstE()->getOutput();
    uint64_t W_valE = wreg->getvalE()->getOutput();
    if(d_srcA == W_dstE){
        return W_valE;
    }
    
    return d_rvalA;
}

uint64_t DecodeStage::getvalB(uint64_t d_srcB,D * dreg, M * mreg, 
			W * wreg, uint64_t e_dstE, uint64_t e_valE, uint64_t d_rvalB, uint64_t M_valM){
    
   	if(d_srcB == RNONE){
    	return 0;
    }
    if(d_srcB == e_dstE){
        return e_valE;
    }
    
    uint64_t M_dstM = mreg->getdstM()->getOutput();
    if(d_srcB == M_dstM){
        return M_valM;
    }
    
    uint64_t M_dstE = mreg->getdstE()->getOutput();
    uint64_t M_valE = mreg->getvalE()->getOutput();
    if(d_srcB == M_dstE){
        return M_valE;
    }
    
    uint64_t W_dstM = wreg->getdstM()->getOutput();
    uint64_t W_valM = wreg->getvalM()->getOutput();
    if(d_srcB == W_dstM){
    	return W_valM;
    }
    
    
    uint64_t W_dstE = wreg->getdstE()->getOutput();
    uint64_t W_valE = wreg->getvalE()->getOutput();
    if(d_srcB == W_dstE){
        return W_valE;
    }
        
    return d_rvalB;
}

uint64_t DecodeStage::getd_srcA(){
	return d_srcA;
}

uint64_t DecodeStage::getd_srcB(){
	return d_srcB;
}

bool DecodeStage::setEBubble(uint64_t E_icode, uint64_t E_dstM, uint64_t e_Cnd, uint64_t srcA, uint64_t srcB){
	if((E_icode == IJXX && !e_Cnd) || ((E_icode == IMRMOVQ 
				|| E_icode == IPOPQ) && (E_dstM == srcA || E_dstM == srcB))){
		return true;
	}
	return false;
}
	
	
	
	