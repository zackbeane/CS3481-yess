//class to perform the Writeback Stage
class WritebackStage: public Stage
{
	private:
	
	public:
		bool doClockLow(PipeReg ** pregs, Stage ** stages);
		void doClockHigh(PipeReg ** pregs);
};