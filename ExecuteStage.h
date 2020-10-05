//class to perform the Decode Stage
class ExecuteStage: public Stage
{
	private:
        uint64_t edstE;
        uint64_t evalE;    
        uint64_t e_Cnd;    
        void setMinput(M * mreg, uint64_t Cnd, 
				uint64_t valE, uint64_t stat, uint64_t icode, 
				uint64_t valA, uint64_t dstE, uint64_t dstM );
		uint64_t getalufun(uint64_t icode, E * ereg);
		int64_t getalu2(uint64_t icode, E * ereg);
		int64_t getalu1(uint64_t icode, E * ereg);
        bool set_cc(uint64_t icode, uint64_t m_stat, uint64_t W_stat);
        uint64_t getdstE(uint64_t icode, uint64_t Cnd, uint64_t dstE);
        int64_t aluMethod(uint64_t alu, int64_t alu1, int64_t alu2, bool get_cc);
        void ccMethod(int64_t alu1, int64_t alu2, int64_t aluVal, uint64_t y86stuff);
        uint64_t getCnd(uint64_t icode,uint64_t ifun);
		bool M_bubble;
		bool CalculateControlSignals(uint64_t m_stat, uint64_t W_stat);
	    void normalM(PipeReg ** pregs);
		void bubbleM(PipeReg ** pregs);

	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
        uint64_t getedstE();
        uint64_t getevalE();
        uint64_t geteCndE();
};
