CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O
OBJ = yess.o Simulate.o Loader.o Memory.o Tools.o RegisterFile.o ConditionCodes.o F.o D.o M.o W.o E.o FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o PipeRegField.o PipeReg.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Loader.h Memory.h RegisterFile.h ConditionCodes.h Debug.h PipeReg.h Stage.h Simulate.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h

ConditionCodes.o: ConditionCodes.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h

Tools.o: Tools.h

Loader.o: Loader.h

F.o: PipeRegField.h PipeReg.h F.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h E.h Status.h

M.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h M.h Status.h

W.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h W.h Status.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h Instructions.h F.h D.h M.h W.h FetchStage.h DecodeStage.h ExecuteStage.h Memory.h Tools.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h Instructions.h D.h E.h M.h W.h DecodeStage.h ExecuteStage.h MemoryStage.h

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h Instructions.h E.h M.h W.h ExecuteStage.h MemoryStage.h 

MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h Instructions.h M.h W.h MemoryStage.h Memory.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h Stage.h Status.h Debug.h Instructions.h W.h WritebackStage.h

PipeRegField.o: PipeRegField.h

PipeReg.o: PipeReg.h

clean:
	rm $(OBJ) yess
	rm -f *.o

run:
	make clean
	make yess
	./run.sh
