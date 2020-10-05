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
#include "WritebackStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Status.h"

/*
 * doClockLow:
 * performs the Writeback Stage logic that is performed when the clock edge is low
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage, Memory Stage, WritebackStagfe instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages){
	W * wreg = (W *) pregs[WREG];
	
	if(wreg->getstat()->getOutput() != SAOK){
    	return true;
    }
	return false;
	
	
}

void WritebackStage::doClockHigh(PipeReg ** pregs){
	bool error = false;
    
    RegisterFile * rf = RegisterFile::getInstance();
    W * wreg = (W *) pregs[WREG];
    
    uint64_t valE = wreg->getvalE()->getOutput();
    uint64_t dstE = wreg->getdstE()->getOutput();
    rf->writeRegister(valE, dstE, error);
    
    uint64_t valM = wreg->getvalM()->getOutput();
	uint64_t dstM = wreg->getdstM()->getOutput();
    rf->writeRegister(valM, dstM, error);

}
