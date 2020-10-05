#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "Memory.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
/*
 * doClockLow:
 * performs the Memory Stage logic that is performed when the clock edge is low
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage, Memory Stage, WritebackStagfe instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages){
	M * mreg = (M *) pregs[MREG];
   	W * wreg = (W *) pregs[WREG];
	Memory * mem = Memory::getInstance();
	bool error = false;
	uint64_t stat = mreg->getstat()->getOutput();
   	uint64_t icode = mreg->geticode()->getOutput();
	uint64_t valE = mreg->getvalE()->getOutput();
	uint64_t dstE = mreg->getdstE()->getOutput();
	uint64_t dstM = mreg->getdstM()->getOutput();
	uint64_t valM = 0;
	uint64_t mem_addr = getmemAddr(icode, mreg->getvalA()->getOutput(), valE);
	
	bool mem_read = (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
	if(mem_read){
		valM = mem->getLong(mem_addr, error);
	}
	
	bool mem_write = (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
	if(mem_write){
		mem->putLong(mreg->getvalA()->getOutput(), mem_addr, error);
	}
	stat = error ? SADR : stat;
	mvalM = valM;
	m_stat = stat;
	setWinput(wreg, stat, icode, valE, dstE, dstM, valM);
	
	return false;
}

void MemoryStage:: doClockHigh(PipeReg ** pregs){
	W * wreg = (W *) pregs[WREG];
	
	wreg->getstat()->normal();
	wreg->geticode()->normal();
	wreg->getvalE()->normal();
	wreg->getdstE()->normal();
	wreg->getdstM()->normal();
	wreg->getvalM()->normal();
}

void MemoryStage::setWinput(W * wreg, uint64_t m_stat, 
			uint64_t icode, uint64_t valE, uint64_t dstE, uint64_t dstM, uint64_t valM){
	
	wreg->getstat()->setInput(m_stat);
	wreg->geticode()->setInput(icode);
	wreg->getvalE()->setInput(valE);
	wreg->getdstE()->setInput(dstE);
	wreg->getdstM()->setInput(dstM);
	wreg->getvalM()->setInput(valM);
}

uint64_t MemoryStage::getmemAddr(uint64_t icode, uint64_t valA, uint64_t valE){
/*
//HCL for Addr component
word mem_addr = [
M_icode in { IRMMOVQ, IPUSHQ, ICALL, IMRMOVQ } : M_valE;
M_icode in { IPOPQ, IRET } : M_valA;
1: 0;
];
*/
	if( icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL || icode == IMRMOVQ){
		return valE;
	}else if( icode == IPOPQ || icode == IRET){
		return valA;
	}else{
		return 0;
	}
}

uint64_t MemoryStage::getvalM(){
	return mvalM;
}


uint64_t MemoryStage::getm_stat(){
	return m_stat;
}
