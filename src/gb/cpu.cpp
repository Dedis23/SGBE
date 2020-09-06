#include "cpu.h"

CPU::CPU(GBInternals& i_GBInternals, MMU& i_MMU) :
AF(A, F), BC(B, C), DE(D, E), HL(H, L),
m_IME(false), 
m_HALT(false), 
m_IsConditionalJumpTaken(false), 
m_MMU(i_MMU),
m_GBInternals(i_GBInternals)
{
	initOPCodes();
	initExtendedOPCodes();
}

string CPU::Step(uint32_t& o_Cycles)
{
	static bool isOkToPrint = false;
	//static bool lastPCCorrect = false;
	////cout << "PC: 0x" << std::hex << PC.GetValue() << endl;
	//if (PC.GetValue() == 0xC4DD)
	//{
	//	lastPCCorrect = true;
	//}
	//if (false /*PC.GetValue() == 0xC503 && A.GetValue() == 0x01*/) // done till C501
	//{
	//	isOkToPrint = true;
	//	cout << "STOP" << endl;
	//	dumpRegisters();
	//}
	if (m_HALT)
	{
		o_Cycles += 4;
		return m_OPCodesNames[0x76];
	}
	// read next instruction opcode
	byte OPCode = readNextByte();

	//if (m_MMU.Read(BOOTSTRAP_DONE_ADDR) != 0x01)
	//{
	//	// set memory
	//
	//	// gpu init
	//	m_MMU.Write(GPU_LCD_CONTROL_ADDR, 0x91);
	//	m_MMU.Write(GPU_LCDC_STATUS_ADDR, 0x81);
	//	m_MMU.Write(GPU_SCROLL_Y_ADDR, 0x00);
	//	m_MMU.Write(GPU_SCROLL_X_ADDR, 0x00);
	//	m_MMU.Write(GPU_LCDC_Y_COORDINATE_ADDR, 0x90);
	//	m_MMU.Write(GPU_LY_COMPARE_ADDR, 0x00);
	//	m_MMU.Write(GPU_DMA_TRANSFER_AND_START_ADDR, 0x00);
	//	m_MMU.Write(GPU_BG_PALETTE_DATA_ADDR, 0xFC);
	//	m_MMU.Write(GPU_OBJECT_PALETTE_0_DATA_ADDR, 0x00);
	//	m_MMU.Write(GPU_OBJECT_PALETTE_1_DATA_ADDR, 0x00);
	//	m_MMU.Write(GPU_WINDOW_Y_POSITION_ADDR, 0x00);
	//	m_MMU.Write(GPU_WINDOW_X_POSITION_MINUS_7_ADDR, 0x00);
	//
	//	// various stuff
	//	m_MMU.Write(0xFF70, 0xF8);
	//	m_MMU.Write(0xFF4F, 0xFE);
	//	m_MMU.Write(0xFF4D, 0x7E);
	//	m_MMU.Write(0xFF00, 0xCF);
	//	m_MMU.Write(0xFF01, 0x00);
	//	m_MMU.Write(0xFF02, 0x7C);
	//	m_MMU.Write(0xFF04, 0x1E);
	//	m_MMU.Write(0xFF05, 0x00);
	//	m_MMU.Write(0xFF06, 0x00);
	//	m_MMU.Write(0xFF07, 0xF8);
	//	m_MMU.Write(0xFF0F, 0xE1);
	//	m_MMU.Write(0xFFFF, 0x00);
	//
	//	m_MMU.Write(BOOTSTRAP_DONE_ADDR, 0x01);
	//
	//	// set registers
	//	PC.SetValue(0x100);
	//	SP.SetValue(0xFFFE);
	//	//if (m_bCGB)
	//	//	AF.SetValue(0x11B0);
	//	//else
	//		AF.SetValue(0x1180);
	//	BC.SetValue(0x0000);
	//	DE.SetValue(0xFF56);
	//	HL.SetValue(0x000D);
	//	return;
	//}

	//cout << "PC is: " << PC.GetValue() << endl;
	//cout << "SP is: " << SP.GetValue() << endl;
	//AF.SetValue(4096);
	//cout << "AF is: " << AF.GetValue() << endl;
	////PUSH(AF.GetValue());
	//OPCode_F5();
	//AF.SetValue(9);
	//cout << "AF is: " << AF.GetValue() << endl;
	//OPCode_F1();
	//cout << "AF is: " << AF.GetValue() << endl;
	//cout << "PC is: " << PC.GetValue() << endl;
	//cout << "SP is: " << SP.GetValue() << endl;

	// get opcode data from the opcode data map or the CB opcode data map
	//OPCodeData OPCodeData;
	if (OPCode == 0xCB)
	{
		byte cbOPCode = readNextByte();
		//OPCodeData = m_CBTestArr[OPCode];
		//OPCodeData = m_CB_OPCodeDataMap[cbOPCode];
		LOG_INFO(isOkToPrint, NOP, "Executing OPCode: " << std::hex << (int)OPCode << " " << m_ExtendedOPCodesNames[OPCode] << " in address 0x" << std::hex << PC.GetValue() - 1);
		(this->*m_ExtendedOPCodes[cbOPCode])();
		// calculate cycles
		o_Cycles += m_ExtendedOPCodesCycles[cbOPCode];
		return m_ExtendedOPCodesNames[cbOPCode];
	}
	else
	{
		LOG_INFO(isOkToPrint, NOP, "Executing OPCode: " << std::hex << (int)OPCode << " " << m_OPCodesNames[OPCode] << " in address 0x" << std::hex << PC.GetValue() - 1);
		(this->*m_OPCodes[OPCode])();
		//if (isOkToPrint) dumpRegisters();
		//OPCodeData = m_TestArr[OPCode];
		//OPCodeData = m_OPCodeDataMap[OPCode];
		//(this->*m_OPCodes[OPCode])();
		// calculate cycles
		if (m_IsConditionalJumpTaken)
		{
			o_Cycles += m_OPCodesConditionalCycles[OPCode];
		}
		else
		{
			o_Cycles += m_OPCodesCycles[OPCode];
		}
		return m_OPCodesNames[OPCode];
	}

	// execute
	//LOG_INFO(true, NOP, "Executing OPCode: " << std::hex << (int)OPCode << " " << OPCodeData.Name << " in address 0x" << std::hex << PC.GetValue() - 1);

	//(this->*OPCodeData.Operation)();
	// this is for debug only, to be removed
	//if (PC.GetValue() - 1 >= 0x95 && PC.GetValue() - 1 <= 0xA7)
	//{
	//	//LOG_INFO(true, NOP, "Executing " << OPCodeData.Name << " in address 0x" << std::hex << PC.GetValue() - 1);
	//	(this->*OPCodeData.Operation)();
	//	//dumpRegisters();
	//	//cout << endl;
	//}
	//else
	//{
	//	(this->*OPCodeData.Operation)();
	//	//(true, NOP, "Executing " << OPCodeData.Name << " in address 0x" << std::hex << PC.GetValue() - 1);
	//}



}

void CPU::Reset()
{
	A.SetValue(0);
	B.SetValue(0);
	C.SetValue(0);
	D.SetValue(0);
	E.SetValue(0);
	F.SetValue(0);
	H.SetValue(0);
	L.SetValue(0);
	SP.SetValue(0);
	PC.SetValue(0);
	F.SetValue(0);
	m_IME = false;
	m_HALT = false;
	m_IsConditionalJumpTaken = false;
}

void CPU::RequestInterrupt(InterruptType i_InterruptType)
{
	// get out of halt
	m_HALT = false;
	// set in memory the requested interrupt
	byte interruptRequest = m_MMU.Read(INTERRUPT_REQUREST_ADDR);
	switch (i_InterruptType)
	{
	case CPU::InterruptType::VBlank:
		bitwise::SetBit(0, interruptRequest);
		break;
	case CPU::InterruptType::LCD:
		bitwise::SetBit(1, interruptRequest);
		break;
	case CPU::InterruptType::Timer:
		bitwise::SetBit(2, interruptRequest);
		break;
	case CPU::InterruptType::Serial:
		bitwise::SetBit(3, interruptRequest);
		break;
	case CPU::InterruptType::Joypad:
		bitwise::SetBit(4, interruptRequest);
		break;
	}
	m_MMU.Write(INTERRUPT_REQUREST_ADDR, interruptRequest);
}

void CPU::HandleInterrupts(uint32_t& o_Cycles)
{

	// every bit represent a different interrupt, lowest bits = highest priority
	// need to check whether a bit was requested from the Interrup Request byte
	// and whether the or not this interrupt type is currently enabled
	byte interruptRequest = m_MMU.Read(INTERRUPT_REQUREST_ADDR);
	byte interruptEnabled = m_MMU.Read(INTERRUPT_ENABLED_ADDR);
	byte activeInterrupts = interruptRequest & interruptEnabled;
	if (activeInterrupts > 0)
	{
		if (m_IME)
		{
			// disable IME during the time we serivce the interrupt
			m_IME = false;

			// cpu is not halt during the interrupt
			m_HALT = false;

			// save current PC into stack
			PUSH(PC.GetValue());
			// set PC to the interrupt routine based on priority and clear the request bit from request flag
			if (bitwise::IsBitSet(0, activeInterrupts))
			{
				PC.SetValue(VBLANK_INTERRUPT_ROUTINE_ADDR);
				bitwise::ClearBit(0, interruptRequest);
			}
			else if (bitwise::IsBitSet(1, activeInterrupts))
			{
				PC.SetValue(LCD_INTERRUPT_ROUTINE_ADDR);
				bitwise::ClearBit(1, interruptRequest);
			}
			else if (bitwise::IsBitSet(2, activeInterrupts))
			{
				PC.SetValue(TIMER_INTERRUPT_ROUTINE_ADDR);
				bitwise::ClearBit(2, interruptRequest);
			}
			else if (bitwise::IsBitSet(3, activeInterrupts))
			{
				PC.SetValue(SERIAL_INTERRUPT_ROUTINE_ADDR);
				bitwise::ClearBit(3, interruptRequest);
			}
			else if (bitwise::IsBitSet(4, activeInterrupts))
			{
				PC.SetValue(JOYPAD_INTERRUPT_ROUTINE_ADDR);
				bitwise::ClearBit(4, interruptRequest);
			}

			m_MMU.Write(INTERRUPT_REQUREST_ADDR, interruptRequest);
			o_Cycles += 20;
		}
	}
}

void CPU::initOPCodes()
{
	m_OPCodes[0x00] = &CPU::OPCode_00;
	m_OPCodes[0x01] = &CPU::OPCode_01;
	m_OPCodes[0x02] = &CPU::OPCode_02;
	m_OPCodes[0x03] = &CPU::OPCode_03;
	m_OPCodes[0x04] = &CPU::OPCode_04;
	m_OPCodes[0x05] = &CPU::OPCode_05;
	m_OPCodes[0x06] = &CPU::OPCode_06;
	m_OPCodes[0x07] = &CPU::OPCode_07;
	m_OPCodes[0x08] = &CPU::OPCode_08;
	m_OPCodes[0x09] = &CPU::OPCode_09;
	m_OPCodes[0x0A] = &CPU::OPCode_0A;
	m_OPCodes[0x0B] = &CPU::OPCode_0B;
	m_OPCodes[0x0C] = &CPU::OPCode_0C;
	m_OPCodes[0x0D] = &CPU::OPCode_0D;
	m_OPCodes[0x0E] = &CPU::OPCode_0E;
	m_OPCodes[0x0F] = &CPU::OPCode_0F;
	m_OPCodes[0x10] = &CPU::OPCode_10;
	m_OPCodes[0x11] = &CPU::OPCode_11;
	m_OPCodes[0x12] = &CPU::OPCode_12;
	m_OPCodes[0x13] = &CPU::OPCode_13;
	m_OPCodes[0x14] = &CPU::OPCode_14;
	m_OPCodes[0x15] = &CPU::OPCode_15;
	m_OPCodes[0x16] = &CPU::OPCode_16;
	m_OPCodes[0x17] = &CPU::OPCode_17;
	m_OPCodes[0x18] = &CPU::OPCode_18;
	m_OPCodes[0x19] = &CPU::OPCode_19;
	m_OPCodes[0x1A] = &CPU::OPCode_1A;
	m_OPCodes[0x1B] = &CPU::OPCode_1B;
	m_OPCodes[0x1C] = &CPU::OPCode_1C;
	m_OPCodes[0x1D] = &CPU::OPCode_1D;
	m_OPCodes[0x1E] = &CPU::OPCode_1E;
	m_OPCodes[0x1F] = &CPU::OPCode_1F;
	m_OPCodes[0x20] = &CPU::OPCode_20;
	m_OPCodes[0x21] = &CPU::OPCode_21;
	m_OPCodes[0x22] = &CPU::OPCode_22;
	m_OPCodes[0x23] = &CPU::OPCode_23;
	m_OPCodes[0x24] = &CPU::OPCode_24;
	m_OPCodes[0x25] = &CPU::OPCode_25;
	m_OPCodes[0x26] = &CPU::OPCode_26;
	m_OPCodes[0x27] = &CPU::OPCode_27;
	m_OPCodes[0x28] = &CPU::OPCode_28;
	m_OPCodes[0x29] = &CPU::OPCode_29;
	m_OPCodes[0x2A] = &CPU::OPCode_2A;
	m_OPCodes[0x2B] = &CPU::OPCode_2B;
	m_OPCodes[0x2C] = &CPU::OPCode_2C;
	m_OPCodes[0x2D] = &CPU::OPCode_2D;
	m_OPCodes[0x2E] = &CPU::OPCode_2E;
	m_OPCodes[0x2F] = &CPU::OPCode_2F;
	m_OPCodes[0x30] = &CPU::OPCode_30;
	m_OPCodes[0x31] = &CPU::OPCode_31;
	m_OPCodes[0x32] = &CPU::OPCode_32;
	m_OPCodes[0x33] = &CPU::OPCode_33;
	m_OPCodes[0x34] = &CPU::OPCode_34;
	m_OPCodes[0x35] = &CPU::OPCode_35;
	m_OPCodes[0x36] = &CPU::OPCode_36;
	m_OPCodes[0x37] = &CPU::OPCode_37;
	m_OPCodes[0x38] = &CPU::OPCode_38;
	m_OPCodes[0x39] = &CPU::OPCode_39;
	m_OPCodes[0x3A] = &CPU::OPCode_3A;
	m_OPCodes[0x3B] = &CPU::OPCode_3B;
	m_OPCodes[0x3C] = &CPU::OPCode_3C;
	m_OPCodes[0x3D] = &CPU::OPCode_3D;
	m_OPCodes[0x3E] = &CPU::OPCode_3E;
	m_OPCodes[0x3F] = &CPU::OPCode_3F;
	m_OPCodes[0x40] = &CPU::OPCode_40;
	m_OPCodes[0x41] = &CPU::OPCode_41;
	m_OPCodes[0x42] = &CPU::OPCode_42;
	m_OPCodes[0x43] = &CPU::OPCode_43;
	m_OPCodes[0x44] = &CPU::OPCode_44;
	m_OPCodes[0x45] = &CPU::OPCode_45;
	m_OPCodes[0x46] = &CPU::OPCode_46;
	m_OPCodes[0x47] = &CPU::OPCode_47;
	m_OPCodes[0x48] = &CPU::OPCode_48;
	m_OPCodes[0x49] = &CPU::OPCode_49;
	m_OPCodes[0x4A] = &CPU::OPCode_4A;
	m_OPCodes[0x4B] = &CPU::OPCode_4B;
	m_OPCodes[0x4C] = &CPU::OPCode_4C;
	m_OPCodes[0x4D] = &CPU::OPCode_4D;
	m_OPCodes[0x4E] = &CPU::OPCode_4E;
	m_OPCodes[0x4F] = &CPU::OPCode_4F;
	m_OPCodes[0x50] = &CPU::OPCode_50;
	m_OPCodes[0x51] = &CPU::OPCode_51;
	m_OPCodes[0x52] = &CPU::OPCode_52;
	m_OPCodes[0x53] = &CPU::OPCode_53;
	m_OPCodes[0x54] = &CPU::OPCode_54;
	m_OPCodes[0x55] = &CPU::OPCode_55;
	m_OPCodes[0x56] = &CPU::OPCode_56;
	m_OPCodes[0x57] = &CPU::OPCode_57;
	m_OPCodes[0x58] = &CPU::OPCode_58;
	m_OPCodes[0x59] = &CPU::OPCode_59;
	m_OPCodes[0x5A] = &CPU::OPCode_5A;
	m_OPCodes[0x5B] = &CPU::OPCode_5B;
	m_OPCodes[0x5C] = &CPU::OPCode_5C;
	m_OPCodes[0x5D] = &CPU::OPCode_5D;
	m_OPCodes[0x5E] = &CPU::OPCode_5E;
	m_OPCodes[0x5F] = &CPU::OPCode_5F;
	m_OPCodes[0x60] = &CPU::OPCode_60;
	m_OPCodes[0x61] = &CPU::OPCode_61;
	m_OPCodes[0x62] = &CPU::OPCode_62;
	m_OPCodes[0x63] = &CPU::OPCode_63;
	m_OPCodes[0x64] = &CPU::OPCode_64;
	m_OPCodes[0x65] = &CPU::OPCode_65;
	m_OPCodes[0x66] = &CPU::OPCode_66;
	m_OPCodes[0x67] = &CPU::OPCode_67;
	m_OPCodes[0x68] = &CPU::OPCode_68;
	m_OPCodes[0x69] = &CPU::OPCode_69;
	m_OPCodes[0x6A] = &CPU::OPCode_6A;
	m_OPCodes[0x6B] = &CPU::OPCode_6B;
	m_OPCodes[0x6C] = &CPU::OPCode_6C;
	m_OPCodes[0x6D] = &CPU::OPCode_6D;
	m_OPCodes[0x6E] = &CPU::OPCode_6E;
	m_OPCodes[0x6F] = &CPU::OPCode_6F;
	m_OPCodes[0x70] = &CPU::OPCode_70;
	m_OPCodes[0x71] = &CPU::OPCode_71;
	m_OPCodes[0x72] = &CPU::OPCode_72;
	m_OPCodes[0x73] = &CPU::OPCode_73;
	m_OPCodes[0x74] = &CPU::OPCode_74;
	m_OPCodes[0x75] = &CPU::OPCode_75;
	m_OPCodes[0x76] = &CPU::OPCode_76;
	m_OPCodes[0x77] = &CPU::OPCode_77;
	m_OPCodes[0x78] = &CPU::OPCode_78;
	m_OPCodes[0x79] = &CPU::OPCode_79;
	m_OPCodes[0x7A] = &CPU::OPCode_7A;
	m_OPCodes[0x7B] = &CPU::OPCode_7B;
	m_OPCodes[0x7C] = &CPU::OPCode_7C;
	m_OPCodes[0x7D] = &CPU::OPCode_7D;
	m_OPCodes[0x7E] = &CPU::OPCode_7E;
	m_OPCodes[0x7F] = &CPU::OPCode_7F;
	m_OPCodes[0x80] = &CPU::OPCode_80;
	m_OPCodes[0x81] = &CPU::OPCode_81;
	m_OPCodes[0x82] = &CPU::OPCode_82;
	m_OPCodes[0x83] = &CPU::OPCode_83;
	m_OPCodes[0x84] = &CPU::OPCode_84;
	m_OPCodes[0x85] = &CPU::OPCode_85;
	m_OPCodes[0x86] = &CPU::OPCode_86;
	m_OPCodes[0x87] = &CPU::OPCode_87;
	m_OPCodes[0x88] = &CPU::OPCode_88;
	m_OPCodes[0x89] = &CPU::OPCode_89;
	m_OPCodes[0x8A] = &CPU::OPCode_8A;
	m_OPCodes[0x8B] = &CPU::OPCode_8B;
	m_OPCodes[0x8C] = &CPU::OPCode_8C;
	m_OPCodes[0x8D] = &CPU::OPCode_8D;
	m_OPCodes[0x8E] = &CPU::OPCode_8E;
	m_OPCodes[0x8F] = &CPU::OPCode_8F;
	m_OPCodes[0x90] = &CPU::OPCode_90;
	m_OPCodes[0x91] = &CPU::OPCode_91;
	m_OPCodes[0x92] = &CPU::OPCode_92;
	m_OPCodes[0x93] = &CPU::OPCode_93;
	m_OPCodes[0x94] = &CPU::OPCode_94;
	m_OPCodes[0x95] = &CPU::OPCode_95;
	m_OPCodes[0x96] = &CPU::OPCode_96;
	m_OPCodes[0x97] = &CPU::OPCode_97;
	m_OPCodes[0x98] = &CPU::OPCode_98;
	m_OPCodes[0x99] = &CPU::OPCode_99;
	m_OPCodes[0x9A] = &CPU::OPCode_9A;
	m_OPCodes[0x9B] = &CPU::OPCode_9B;
	m_OPCodes[0x9C] = &CPU::OPCode_9C;
	m_OPCodes[0x9D] = &CPU::OPCode_9D;
	m_OPCodes[0x9E] = &CPU::OPCode_9E;
	m_OPCodes[0x9F] = &CPU::OPCode_9F;
	m_OPCodes[0xA0] = &CPU::OPCode_A0;
	m_OPCodes[0xA1] = &CPU::OPCode_A1;
	m_OPCodes[0xA2] = &CPU::OPCode_A2;
	m_OPCodes[0xA3] = &CPU::OPCode_A3;
	m_OPCodes[0xA4] = &CPU::OPCode_A4;
	m_OPCodes[0xA5] = &CPU::OPCode_A5;
	m_OPCodes[0xA6] = &CPU::OPCode_A6;
	m_OPCodes[0xA7] = &CPU::OPCode_A7;
	m_OPCodes[0xA8] = &CPU::OPCode_A8;
	m_OPCodes[0xA9] = &CPU::OPCode_A9;
	m_OPCodes[0xAA] = &CPU::OPCode_AA;
	m_OPCodes[0xAB] = &CPU::OPCode_AB;
	m_OPCodes[0xAC] = &CPU::OPCode_AC;
	m_OPCodes[0xAD] = &CPU::OPCode_AD;
	m_OPCodes[0xAE] = &CPU::OPCode_AE;
	m_OPCodes[0xAF] = &CPU::OPCode_AF;
	m_OPCodes[0xB0] = &CPU::OPCode_B0;
	m_OPCodes[0xB1] = &CPU::OPCode_B1;
	m_OPCodes[0xB2] = &CPU::OPCode_B2;
	m_OPCodes[0xB3] = &CPU::OPCode_B3;
	m_OPCodes[0xB4] = &CPU::OPCode_B4;
	m_OPCodes[0xB5] = &CPU::OPCode_B5;
	m_OPCodes[0xB6] = &CPU::OPCode_B6;
	m_OPCodes[0xB7] = &CPU::OPCode_B7;
	m_OPCodes[0xB8] = &CPU::OPCode_B8;
	m_OPCodes[0xB9] = &CPU::OPCode_B9;
	m_OPCodes[0xBA] = &CPU::OPCode_BA;
	m_OPCodes[0xBB] = &CPU::OPCode_BB;
	m_OPCodes[0xBC] = &CPU::OPCode_BC;
	m_OPCodes[0xBD] = &CPU::OPCode_BD;
	m_OPCodes[0xBE] = &CPU::OPCode_BE;
	m_OPCodes[0xBF] = &CPU::OPCode_BF;
	m_OPCodes[0xC0] = &CPU::OPCode_C0;
	m_OPCodes[0xC1] = &CPU::OPCode_C1;
	m_OPCodes[0xC2] = &CPU::OPCode_C2;
	m_OPCodes[0xC3] = &CPU::OPCode_C3;
	m_OPCodes[0xC4] = &CPU::OPCode_C4;
	m_OPCodes[0xC5] = &CPU::OPCode_C5;
	m_OPCodes[0xC6] = &CPU::OPCode_C6;
	m_OPCodes[0xC7] = &CPU::OPCode_C7;
	m_OPCodes[0xC8] = &CPU::OPCode_C8;
	m_OPCodes[0xC9] = &CPU::OPCode_C9;
	m_OPCodes[0xCA] = &CPU::OPCode_CA;
	m_OPCodes[0xCB] = &CPU::OPCode_CB; // gate to the extended opcodes
	m_OPCodes[0xCC] = &CPU::OPCode_CC;
	m_OPCodes[0xCD] = &CPU::OPCode_CD;
	m_OPCodes[0xCE] = &CPU::OPCode_CE;
	m_OPCodes[0xCF] = &CPU::OPCode_CF;
	m_OPCodes[0xD0] = &CPU::OPCode_D0;
	m_OPCodes[0xD1] = &CPU::OPCode_D1;
	m_OPCodes[0xD2] = &CPU::OPCode_D2;
	m_OPCodes[0xD3] = &CPU::OPCode_D3; // undefined
	m_OPCodes[0xD4] = &CPU::OPCode_D4;
	m_OPCodes[0xD5] = &CPU::OPCode_D5;
	m_OPCodes[0xD6] = &CPU::OPCode_D6;
	m_OPCodes[0xD7] = &CPU::OPCode_D7;
	m_OPCodes[0xD8] = &CPU::OPCode_D8;
	m_OPCodes[0xD9] = &CPU::OPCode_D9;
	m_OPCodes[0xDA] = &CPU::OPCode_DA;
	m_OPCodes[0xDB] = &CPU::OPCode_DB; // undefined
	m_OPCodes[0xDC] = &CPU::OPCode_DC;
	m_OPCodes[0xDD] = &CPU::OPCode_DD; // undefined
	m_OPCodes[0xDE] = &CPU::OPCode_DE;
	m_OPCodes[0xDF] = &CPU::OPCode_DF;
	m_OPCodes[0xE0] = &CPU::OPCode_E0;
	m_OPCodes[0xE1] = &CPU::OPCode_E1;
	m_OPCodes[0xE2] = &CPU::OPCode_E2;
	m_OPCodes[0xE3] = &CPU::OPCode_E3; // undefined
	m_OPCodes[0xE4] = &CPU::OPCode_E4; // undefined
	m_OPCodes[0xE5] = &CPU::OPCode_E5;
	m_OPCodes[0xE6] = &CPU::OPCode_E6;
	m_OPCodes[0xE7] = &CPU::OPCode_E7;
	m_OPCodes[0xE8] = &CPU::OPCode_E8;
	m_OPCodes[0xE9] = &CPU::OPCode_E9;
	m_OPCodes[0xEA] = &CPU::OPCode_EA;
	m_OPCodes[0xEB] = &CPU::OPCode_EB; // undefined
	m_OPCodes[0xEC] = &CPU::OPCode_EC; // undefined
	m_OPCodes[0xED] = &CPU::OPCode_ED; // undefined
	m_OPCodes[0xEE] = &CPU::OPCode_EE;
	m_OPCodes[0xEF] = &CPU::OPCode_EF;
	m_OPCodes[0xF0] = &CPU::OPCode_F0;
	m_OPCodes[0xF1] = &CPU::OPCode_F1;
	m_OPCodes[0xF2] = &CPU::OPCode_F2;
	m_OPCodes[0xF3] = &CPU::OPCode_F3;
	m_OPCodes[0xF4] = &CPU::OPCode_F4; // undefined
	m_OPCodes[0xF5] = &CPU::OPCode_F5;
	m_OPCodes[0xF6] = &CPU::OPCode_F6;
	m_OPCodes[0xF7] = &CPU::OPCode_F7;
	m_OPCodes[0xF8] = &CPU::OPCode_F8;
	m_OPCodes[0xF9] = &CPU::OPCode_F9;
	m_OPCodes[0xFA] = &CPU::OPCode_FA;
	m_OPCodes[0xFB] = &CPU::OPCode_FB;
	m_OPCodes[0xFC] = &CPU::OPCode_FC; // undefined
	m_OPCodes[0xFD] = &CPU::OPCode_FD; // undefined
	m_OPCodes[0xFE] = &CPU::OPCode_FE;
	m_OPCodes[0xFF] = &CPU::OPCode_FF;

	m_OPCodesNames[0x00] = "NOP";
	m_OPCodesNames[0x01] = "LD BC, nn";
	m_OPCodesNames[0x02] = "LD (BC), A";
	m_OPCodesNames[0x03] = "INC BC";
	m_OPCodesNames[0x04] = "INC B";
	m_OPCodesNames[0x05] = "DEC B";
	m_OPCodesNames[0x06] = "LD B, n";
	m_OPCodesNames[0x07] = "RLCA";
	m_OPCodesNames[0x08] = "LD (nn), SP";
	m_OPCodesNames[0x09] = "ADD HL, BC";
	m_OPCodesNames[0x0A] = "LD A, (BC)";
	m_OPCodesNames[0x0B] = "DEC BC";
	m_OPCodesNames[0x0C] = "INC C";
	m_OPCodesNames[0x0D] = "DEC C";
	m_OPCodesNames[0x0E] = "LD C, n";
	m_OPCodesNames[0x0F] = "RRCA";
	m_OPCodesNames[0x10] = "STOP";
	m_OPCodesNames[0x11] = "LD DE, nn";
	m_OPCodesNames[0x12] = "LD (DE), A";
	m_OPCodesNames[0x13] = "INC DE";
	m_OPCodesNames[0x14] = "INC D";
	m_OPCodesNames[0x15] = "DEC D";
	m_OPCodesNames[0x16] = "LD D, n";
	m_OPCodesNames[0x17] = "RLA";
	m_OPCodesNames[0x18] = "JR n";
	m_OPCodesNames[0x19] = "ADD HL, DE";
	m_OPCodesNames[0x1A] = "LD A, (DE)";
	m_OPCodesNames[0x1B] = "DEC DE";
	m_OPCodesNames[0x1C] = "INC E";
	m_OPCodesNames[0x1D] = "DEC E";
	m_OPCodesNames[0x1E] = "LD E, n";
	m_OPCodesNames[0x1F] = "RRA";
	m_OPCodesNames[0x20] = "JR NZ, n";
	m_OPCodesNames[0x21] = "LD HL, nn";
	m_OPCodesNames[0x22] = "LDI (HL), A";
	m_OPCodesNames[0x23] = "INC HL";
	m_OPCodesNames[0x24] = "INC H";
	m_OPCodesNames[0x25] = "DEC H";
	m_OPCodesNames[0x26] = "LD H, n";
	m_OPCodesNames[0x27] = "DAA";
	m_OPCodesNames[0x28] = "JR Z, n";
	m_OPCodesNames[0x29] = "ADD HL, HL";
	m_OPCodesNames[0x2A] = "LDI A, (HL)";
	m_OPCodesNames[0x2B] = "DEC HL";
	m_OPCodesNames[0x2C] = "INC L";
	m_OPCodesNames[0x2D] = "DEC L";
	m_OPCodesNames[0x2E] = "LD L, n";
	m_OPCodesNames[0x2F] = "CPL";
	m_OPCodesNames[0x30] = "JR NC, n";
	m_OPCodesNames[0x31] = "LD SP, nn";
	m_OPCodesNames[0x32] = "LDD (HL), A";
	m_OPCodesNames[0x33] = "INC SP";
	m_OPCodesNames[0x34] = "INC (HL)";
	m_OPCodesNames[0x35] = "DEC (HL)";
	m_OPCodesNames[0x36] = "LD (HL), n";
	m_OPCodesNames[0x37] = "SCF";
	m_OPCodesNames[0x38] = "JR C, n";
	m_OPCodesNames[0x39] = "ADD HL, SP";
	m_OPCodesNames[0x3A] = "LDD A, (HL)";
	m_OPCodesNames[0x3B] = "DEC SP";
	m_OPCodesNames[0x3C] = "INC A";
	m_OPCodesNames[0x3D] = "DEC A";
	m_OPCodesNames[0x3E] = "LD A, n";
	m_OPCodesNames[0x3F] = "CCF";
	m_OPCodesNames[0x40] = "LD B, B";
	m_OPCodesNames[0x41] = "LD B, C";
	m_OPCodesNames[0x42] = "LD B, D";
	m_OPCodesNames[0x43] = "LD B, E";
	m_OPCodesNames[0x44] = "LD B, H";
	m_OPCodesNames[0x45] = "LD B, L";
	m_OPCodesNames[0x46] = "LD B, (HL)";
	m_OPCodesNames[0x47] = "LD B, A";
	m_OPCodesNames[0x48] = "LD C, B";
	m_OPCodesNames[0x49] = "LD C, C";
	m_OPCodesNames[0x4A] = "LD C, D";
	m_OPCodesNames[0x4B] = "LD C, E";
	m_OPCodesNames[0x4C] = "LD C, H";
	m_OPCodesNames[0x4D] = "LD C, L";
	m_OPCodesNames[0x4E] = "LD C, (HL)";
	m_OPCodesNames[0x4F] = "LD C, A";
	m_OPCodesNames[0x50] = "LD D, B";
	m_OPCodesNames[0x51] = "LD D, C";
	m_OPCodesNames[0x52] = "LD D, D";
	m_OPCodesNames[0x53] = "LD D, E";
	m_OPCodesNames[0x54] = "LD D, H";
	m_OPCodesNames[0x55] = "LD D, L";
	m_OPCodesNames[0x56] = "LD D, (HL)";
	m_OPCodesNames[0x57] = "LD D, A";
	m_OPCodesNames[0x58] = "LD E, B";
	m_OPCodesNames[0x59] = "LD E, C";
	m_OPCodesNames[0x5A] = "LD E, D";
	m_OPCodesNames[0x5B] = "LD E, E";
	m_OPCodesNames[0x5C] = "LD E, H";
	m_OPCodesNames[0x5D] = "LD E, L";
	m_OPCodesNames[0x5E] = "LD E, (HL)";
	m_OPCodesNames[0x5F] = "LD E, A";
	m_OPCodesNames[0x60] = "LD H, B";
	m_OPCodesNames[0x61] = "LD H, C";
	m_OPCodesNames[0x62] = "LD H, D";
	m_OPCodesNames[0x63] = "LD H, E";
	m_OPCodesNames[0x64] = "LD H, H";
	m_OPCodesNames[0x65] = "LD H, L";
	m_OPCodesNames[0x66] = "LD H, (HL)";
	m_OPCodesNames[0x67] = "LD H, A";
	m_OPCodesNames[0x68] = "LD L, B";
	m_OPCodesNames[0x69] = "LD L, C";
	m_OPCodesNames[0x6A] = "LD L, D";
	m_OPCodesNames[0x6B] = "LD L, E";
	m_OPCodesNames[0x6C] = "LD L, H";
	m_OPCodesNames[0x6D] = "LD L, L";
	m_OPCodesNames[0x6E] = "LD L, (HL)";
	m_OPCodesNames[0x6F] = "LD L, A";
	m_OPCodesNames[0x70] = "LD (HL), B";
	m_OPCodesNames[0x71] = "LD (HL), C";
	m_OPCodesNames[0x72] = "LD (HL), D";
	m_OPCodesNames[0x73] = "LD (HL), E";
	m_OPCodesNames[0x74] = "LD (HL), H";
	m_OPCodesNames[0x75] = "LD (HL), L";
	m_OPCodesNames[0x76] = "HALT";
	m_OPCodesNames[0x77] = "LD (HL), A";
	m_OPCodesNames[0x78] = "LD A, B";
	m_OPCodesNames[0x79] = "LD A, C";
	m_OPCodesNames[0x7A] = "LD A, D";
	m_OPCodesNames[0x7B] = "LD A, E";
	m_OPCodesNames[0x7C] = "LD A, H";
	m_OPCodesNames[0x7D] = "LD A, L";
	m_OPCodesNames[0x7E] = "LD A, (HL)";
	m_OPCodesNames[0x7F] = "LD A, A";
	m_OPCodesNames[0x80] = "ADD A, B";
	m_OPCodesNames[0x81] = "ADD A, C";
	m_OPCodesNames[0x82] = "ADD A, D";
	m_OPCodesNames[0x83] = "ADD A, E";
	m_OPCodesNames[0x84] = "ADD A, H";
	m_OPCodesNames[0x85] = "ADD A, L";
	m_OPCodesNames[0x86] = "ADD A, (HL)";
	m_OPCodesNames[0x87] = "ADD A, A";
	m_OPCodesNames[0x88] = "ADC A, B";
	m_OPCodesNames[0x89] = "ADC A, C";
	m_OPCodesNames[0x8A] = "ADC A, D";
	m_OPCodesNames[0x8B] = "ADC A, E";
	m_OPCodesNames[0x8C] = "ADC A, H";
	m_OPCodesNames[0x8D] = "ADC A, L";
	m_OPCodesNames[0x8E] = "ADC A, (HL)";
	m_OPCodesNames[0x8F] = "ADC A, A";
	m_OPCodesNames[0x90] = "SUB B";
	m_OPCodesNames[0x91] = "SUB C";
	m_OPCodesNames[0x92] = "SUB D";
	m_OPCodesNames[0x93] = "SUB E";
	m_OPCodesNames[0x94] = "SUB H";
	m_OPCodesNames[0x95] = "SUB L";
	m_OPCodesNames[0x96] = "SUB (HL)";
	m_OPCodesNames[0x97] = "SUB A";
	m_OPCodesNames[0x98] = "SBC A, B";
	m_OPCodesNames[0x99] = "SBC A, C";
	m_OPCodesNames[0x9A] = "SBC A, D";
	m_OPCodesNames[0x9B] = "SBC A, E";
	m_OPCodesNames[0x9C] = "SBC A, H";
	m_OPCodesNames[0x9D] = "SBC A, L";
	m_OPCodesNames[0x9E] = "SBC A, (HL)";
	m_OPCodesNames[0x9F] = "SBC A, A";
	m_OPCodesNames[0xA0] = "AND B";
	m_OPCodesNames[0xA1] = "AND C";
	m_OPCodesNames[0xA2] = "AND D";
	m_OPCodesNames[0xA3] = "AND E";
	m_OPCodesNames[0xA4] = "AND H";
	m_OPCodesNames[0xA5] = "AND L";
	m_OPCodesNames[0xA6] = "AND (HL)";
	m_OPCodesNames[0xA7] = "AND A";
	m_OPCodesNames[0xA8] = "XOR B";
	m_OPCodesNames[0xA9] = "XOR C";
	m_OPCodesNames[0xAA] = "XOR D";
	m_OPCodesNames[0xAB] = "XOR E";
	m_OPCodesNames[0xAC] = "XOR H";
	m_OPCodesNames[0xAD] = "XOR L";
	m_OPCodesNames[0xAE] = "XOR (HL)";
	m_OPCodesNames[0xAF] = "XOR A";
	m_OPCodesNames[0xB0] = "OR B";
	m_OPCodesNames[0xB1] = "OR C";
	m_OPCodesNames[0xB2] = "OR D";
	m_OPCodesNames[0xB3] = "OR E";
	m_OPCodesNames[0xB4] = "OR H";
	m_OPCodesNames[0xB5] = "OR L";
	m_OPCodesNames[0xB6] = "OR (HL)";
	m_OPCodesNames[0xB7] = "OR A";
	m_OPCodesNames[0xB8] = "CP B";
	m_OPCodesNames[0xB9] = "CP C";
	m_OPCodesNames[0xBA] = "CP D";
	m_OPCodesNames[0xBB] = "CP E";
	m_OPCodesNames[0xBC] = "CP H";
	m_OPCodesNames[0xBD] = "CP L";
	m_OPCodesNames[0xBE] = "CP (HL)";
	m_OPCodesNames[0xBF] = "CP A";
	m_OPCodesNames[0xC0] = "RET NZ";
	m_OPCodesNames[0xC1] = "POP BC";
	m_OPCodesNames[0xC2] = "JP NZ, nn";
	m_OPCodesNames[0xC3] = "JP nn";
	m_OPCodesNames[0xC4] = "CALL NZ, nn";
	m_OPCodesNames[0xC5] = "PUSH BC";
	m_OPCodesNames[0xC6] = "ADD A, n";
	m_OPCodesNames[0xC7] = "RST 00H";
	m_OPCodesNames[0xC8] = "RET Z";
	m_OPCodesNames[0xC9] = "RET";
	m_OPCodesNames[0xCA] = "JP Z, nn";
	m_OPCodesNames[0xCB] = ""; // gate to the extended opcodes
	m_OPCodesNames[0xCC] = "CALL Z, nn";
	m_OPCodesNames[0xCD] = "CALL nn";
	m_OPCodesNames[0xCE] = "ADC A, n";
	m_OPCodesNames[0xCF] = "RST 08H";
	m_OPCodesNames[0xD0] = "RET NC";
	m_OPCodesNames[0xD1] = "POP DE";
	m_OPCodesNames[0xD2] = "JP NC, nn";
	m_OPCodesNames[0xD3] = ""; // undefined
	m_OPCodesNames[0xD4] = "CALL NC, nn";
	m_OPCodesNames[0xD5] = "PUSH DE";
	m_OPCodesNames[0xD6] = "SUB n";
	m_OPCodesNames[0xD7] = "RST 10H";
	m_OPCodesNames[0xD8] = "RET C";
	m_OPCodesNames[0xD9] = "RETI";
	m_OPCodesNames[0xDA] = "JP C, nn";
	m_OPCodesNames[0xDB] = ""; // undefined
	m_OPCodesNames[0xDC] = "CALL C, nn";
	m_OPCodesNames[0xDD] = ""; // undefined
	m_OPCodesNames[0xDE] = "SBC A, n";
	m_OPCodesNames[0xDF] = "RST 18H";
	m_OPCodesNames[0xE0] = "LDH (n), A";
	m_OPCodesNames[0xE1] = "POP HL";
	m_OPCodesNames[0xE2] = "LD (C), A";
	m_OPCodesNames[0xE3] = ""; // undefined
	m_OPCodesNames[0xE4] = ""; // undefined
	m_OPCodesNames[0xE5] = "PUSH HL";
	m_OPCodesNames[0xE6] = "AND n";
	m_OPCodesNames[0xE7] = "RST 20H";
	m_OPCodesNames[0xE8] = "ADD SP, #";
	m_OPCodesNames[0xE9] = "JP (HL)";
	m_OPCodesNames[0xEA] = "LD (nn), A";
	m_OPCodesNames[0xEB] = ""; // undefined
	m_OPCodesNames[0xEC] = ""; // undefined
	m_OPCodesNames[0xED] = ""; // undefined
	m_OPCodesNames[0xEE] = "XOR n";
	m_OPCodesNames[0xEF] = "RST 28H";
	m_OPCodesNames[0xF0] = "LDH A, (n)";
	m_OPCodesNames[0xF1] = "POP AF";
	m_OPCodesNames[0xF2] = "LD A, (C)";
	m_OPCodesNames[0xF3] = "DI";
	m_OPCodesNames[0xF4] = ""; // undefined
	m_OPCodesNames[0xF5] = "PUSH AF";
	m_OPCodesNames[0xF6] = "OR n";
	m_OPCodesNames[0xF7] = "RST 30H";
	m_OPCodesNames[0xF8] = "LDHL SP, n";
	m_OPCodesNames[0xF9] = "LD SP, HL";
	m_OPCodesNames[0xFA] = "LD A, (nn)";
	m_OPCodesNames[0xFB] = "EI";
	m_OPCodesNames[0xFC] = ""; // undefined
	m_OPCodesNames[0xFD] = ""; // undefined
	m_OPCodesNames[0xFE] = "CP n";
	m_OPCodesNames[0xFF] = "RST 38H";

	m_OPCodesCycles[0x00] = 4;
	m_OPCodesCycles[0x01] = 12;
	m_OPCodesCycles[0x02] = 8;
	m_OPCodesCycles[0x03] = 8;
	m_OPCodesCycles[0x04] = 4;
	m_OPCodesCycles[0x05] = 4;
	m_OPCodesCycles[0x06] = 8;
	m_OPCodesCycles[0x07] = 4;
	m_OPCodesCycles[0x08] = 20;
	m_OPCodesCycles[0x09] = 8;
	m_OPCodesCycles[0x0A] = 8;
	m_OPCodesCycles[0x0B] = 8;
	m_OPCodesCycles[0x0C] = 4;
	m_OPCodesCycles[0x0D] = 4;
	m_OPCodesCycles[0x0E] = 8;
	m_OPCodesCycles[0x0F] = 4;
	m_OPCodesCycles[0x10] = 4;
	m_OPCodesCycles[0x11] = 12;
	m_OPCodesCycles[0x12] = 8;
	m_OPCodesCycles[0x13] = 8;
	m_OPCodesCycles[0x14] = 4;
	m_OPCodesCycles[0x15] = 4;
	m_OPCodesCycles[0x16] = 8;
	m_OPCodesCycles[0x17] = 4;
	m_OPCodesCycles[0x18] = 12;
	m_OPCodesCycles[0x19] = 8;
	m_OPCodesCycles[0x1A] = 8;
	m_OPCodesCycles[0x1B] = 8;
	m_OPCodesCycles[0x1C] = 4;
	m_OPCodesCycles[0x1D] = 4;
	m_OPCodesCycles[0x1E] = 8;
	m_OPCodesCycles[0x1F] = 4;
	m_OPCodesCycles[0x20] = 8;
	m_OPCodesCycles[0x21] = 12;
	m_OPCodesCycles[0x22] = 8;
	m_OPCodesCycles[0x23] = 8;
	m_OPCodesCycles[0x24] = 4;
	m_OPCodesCycles[0x25] = 4;
	m_OPCodesCycles[0x26] = 8;
	m_OPCodesCycles[0x27] = 4;
	m_OPCodesCycles[0x28] = 8;
	m_OPCodesCycles[0x29] = 8;
	m_OPCodesCycles[0x2A] = 8;
	m_OPCodesCycles[0x2B] = 8;
	m_OPCodesCycles[0x2C] = 4;
	m_OPCodesCycles[0x2D] = 4;
	m_OPCodesCycles[0x2E] = 8;
	m_OPCodesCycles[0x2F] = 4;
	m_OPCodesCycles[0x30] = 8;
	m_OPCodesCycles[0x31] = 12;
	m_OPCodesCycles[0x32] = 8;
	m_OPCodesCycles[0x33] = 8;
	m_OPCodesCycles[0x34] = 12;
	m_OPCodesCycles[0x35] = 12;
	m_OPCodesCycles[0x36] = 12;
	m_OPCodesCycles[0x37] = 4;
	m_OPCodesCycles[0x38] = 8;
	m_OPCodesCycles[0x39] = 8;
	m_OPCodesCycles[0x3A] = 8;
	m_OPCodesCycles[0x3B] = 8;
	m_OPCodesCycles[0x3C] = 4;
	m_OPCodesCycles[0x3D] = 4;
	m_OPCodesCycles[0x3E] = 8;
	m_OPCodesCycles[0x3F] = 4;
	m_OPCodesCycles[0x40] = 4;
	m_OPCodesCycles[0x41] = 4;
	m_OPCodesCycles[0x42] = 4;
	m_OPCodesCycles[0x43] = 4;
	m_OPCodesCycles[0x44] = 4;
	m_OPCodesCycles[0x45] = 4;
	m_OPCodesCycles[0x46] = 8;
	m_OPCodesCycles[0x47] = 4;
	m_OPCodesCycles[0x48] = 4;
	m_OPCodesCycles[0x49] = 4;
	m_OPCodesCycles[0x4A] = 4;
	m_OPCodesCycles[0x4B] = 4;
	m_OPCodesCycles[0x4C] = 4;
	m_OPCodesCycles[0x4D] = 4;
	m_OPCodesCycles[0x4E] = 8;
	m_OPCodesCycles[0x4F] = 4;
	m_OPCodesCycles[0x50] = 4;
	m_OPCodesCycles[0x51] = 4;
	m_OPCodesCycles[0x52] = 4;
	m_OPCodesCycles[0x53] = 4;
	m_OPCodesCycles[0x54] = 4;
	m_OPCodesCycles[0x55] = 4;
	m_OPCodesCycles[0x56] = 8;
	m_OPCodesCycles[0x57] = 4;
	m_OPCodesCycles[0x58] = 4;
	m_OPCodesCycles[0x59] = 4;
	m_OPCodesCycles[0x5A] = 4;
	m_OPCodesCycles[0x5B] = 4;
	m_OPCodesCycles[0x5C] = 4;
	m_OPCodesCycles[0x5D] = 4;
	m_OPCodesCycles[0x5E] = 8;
	m_OPCodesCycles[0x5F] = 4;
	m_OPCodesCycles[0x60] = 4;
	m_OPCodesCycles[0x61] = 4;
	m_OPCodesCycles[0x62] = 4;
	m_OPCodesCycles[0x63] = 4;
	m_OPCodesCycles[0x64] = 4;
	m_OPCodesCycles[0x65] = 4;
	m_OPCodesCycles[0x66] = 8;
	m_OPCodesCycles[0x67] = 4;
	m_OPCodesCycles[0x68] = 4;
	m_OPCodesCycles[0x69] = 4;
	m_OPCodesCycles[0x6A] = 4;
	m_OPCodesCycles[0x6B] = 4;
	m_OPCodesCycles[0x6C] = 4;
	m_OPCodesCycles[0x6D] = 4;
	m_OPCodesCycles[0x6E] = 8;
	m_OPCodesCycles[0x6F] = 4;
	m_OPCodesCycles[0x70] = 8;
	m_OPCodesCycles[0x71] = 8;
	m_OPCodesCycles[0x72] = 8;
	m_OPCodesCycles[0x73] = 8;
	m_OPCodesCycles[0x74] = 8;
	m_OPCodesCycles[0x75] = 8;
	m_OPCodesCycles[0x76] = 4;
	m_OPCodesCycles[0x77] = 8;
	m_OPCodesCycles[0x78] = 4;
	m_OPCodesCycles[0x79] = 4;
	m_OPCodesCycles[0x7A] = 4;
	m_OPCodesCycles[0x7B] = 4;
	m_OPCodesCycles[0x7C] = 4;
	m_OPCodesCycles[0x7D] = 4;
	m_OPCodesCycles[0x7E] = 8;
	m_OPCodesCycles[0x7F] = 4;
	m_OPCodesCycles[0x80] = 4;
	m_OPCodesCycles[0x81] = 4;
	m_OPCodesCycles[0x82] = 4;
	m_OPCodesCycles[0x83] = 4;
	m_OPCodesCycles[0x84] = 4;
	m_OPCodesCycles[0x85] = 4;
	m_OPCodesCycles[0x86] = 8;
	m_OPCodesCycles[0x87] = 4;
	m_OPCodesCycles[0x88] = 4;
	m_OPCodesCycles[0x89] = 4;
	m_OPCodesCycles[0x8A] = 4;
	m_OPCodesCycles[0x8B] = 4;
	m_OPCodesCycles[0x8C] = 4;
	m_OPCodesCycles[0x8D] = 4;
	m_OPCodesCycles[0x8E] = 8;
	m_OPCodesCycles[0x8F] = 4;
	m_OPCodesCycles[0x90] = 4;
	m_OPCodesCycles[0x91] = 4;
	m_OPCodesCycles[0x92] = 4;
	m_OPCodesCycles[0x93] = 4;
	m_OPCodesCycles[0x94] = 4;
	m_OPCodesCycles[0x95] = 4;
	m_OPCodesCycles[0x96] = 8;
	m_OPCodesCycles[0x97] = 4;
	m_OPCodesCycles[0x98] = 4;
	m_OPCodesCycles[0x99] = 4;
	m_OPCodesCycles[0x9A] = 4;
	m_OPCodesCycles[0x9B] = 4;
	m_OPCodesCycles[0x9C] = 4;
	m_OPCodesCycles[0x9D] = 4;
	m_OPCodesCycles[0x9E] = 8;
	m_OPCodesCycles[0x9F] = 4;
	m_OPCodesCycles[0xA0] = 4;
	m_OPCodesCycles[0xA1] = 4;
	m_OPCodesCycles[0xA2] = 4;
	m_OPCodesCycles[0xA3] = 4;
	m_OPCodesCycles[0xA4] = 4;
	m_OPCodesCycles[0xA5] = 4;
	m_OPCodesCycles[0xA6] = 8;
	m_OPCodesCycles[0xA7] = 4;
	m_OPCodesCycles[0xA8] = 4;
	m_OPCodesCycles[0xA9] = 4;
	m_OPCodesCycles[0xAA] = 4;
	m_OPCodesCycles[0xAB] = 4;
	m_OPCodesCycles[0xAC] = 4;
	m_OPCodesCycles[0xAD] = 4;
	m_OPCodesCycles[0xAE] = 8;
	m_OPCodesCycles[0xAF] = 4;
	m_OPCodesCycles[0xB0] = 4;
	m_OPCodesCycles[0xB1] = 4;
	m_OPCodesCycles[0xB2] = 4;
	m_OPCodesCycles[0xB3] = 4;
	m_OPCodesCycles[0xB4] = 4;
	m_OPCodesCycles[0xB5] = 4;
	m_OPCodesCycles[0xB6] = 8;
	m_OPCodesCycles[0xB7] = 4;
	m_OPCodesCycles[0xB8] = 4;
	m_OPCodesCycles[0xB9] = 4;
	m_OPCodesCycles[0xBA] = 4;
	m_OPCodesCycles[0xBB] = 4;
	m_OPCodesCycles[0xBC] = 4;
	m_OPCodesCycles[0xBD] = 4;
	m_OPCodesCycles[0xBE] = 8;
	m_OPCodesCycles[0xBF] = 4;
	m_OPCodesCycles[0xC0] = 8;
	m_OPCodesCycles[0xC1] = 12;
	m_OPCodesCycles[0xC2] = 12;
	m_OPCodesCycles[0xC3] = 16;
	m_OPCodesCycles[0xC4] = 12;
	m_OPCodesCycles[0xC5] = 16;
	m_OPCodesCycles[0xC6] = 8;
	m_OPCodesCycles[0xC7] = 16;
	m_OPCodesCycles[0xC8] = 8;
	m_OPCodesCycles[0xC9] = 16;
	m_OPCodesCycles[0xCA] = 12;
	m_OPCodesCycles[0xCB] = 0; // gate to the extended opcodes
	m_OPCodesCycles[0xCC] = 12;
	m_OPCodesCycles[0xCD] = 24;
	m_OPCodesCycles[0xCE] = 8;
	m_OPCodesCycles[0xCF] = 16;
	m_OPCodesCycles[0xD0] = 8;
	m_OPCodesCycles[0xD1] = 12;
	m_OPCodesCycles[0xD2] = 12;
	m_OPCodesCycles[0xD3] = 0; // undefined
	m_OPCodesCycles[0xD4] = 12;
	m_OPCodesCycles[0xD5] = 16;
	m_OPCodesCycles[0xD6] = 8;
	m_OPCodesCycles[0xD7] = 16;
	m_OPCodesCycles[0xD8] = 8;
	m_OPCodesCycles[0xD9] = 16;
	m_OPCodesCycles[0xDA] = 12;
	m_OPCodesCycles[0xDB] = 0; // undefined
	m_OPCodesCycles[0xDC] = 12;
	m_OPCodesCycles[0xDD] = 0; // undefined
	m_OPCodesCycles[0xDE] = 8;
	m_OPCodesCycles[0xDF] = 16;
	m_OPCodesCycles[0xE0] = 12;
	m_OPCodesCycles[0xE1] = 12;
	m_OPCodesCycles[0xE2] = 8;
	m_OPCodesCycles[0xE3] = 0; // undefined
	m_OPCodesCycles[0xE4] = 0; // undefined
	m_OPCodesCycles[0xE5] = 16;
	m_OPCodesCycles[0xE6] = 8;
	m_OPCodesCycles[0xE7] = 16;
	m_OPCodesCycles[0xE8] = 16;
	m_OPCodesCycles[0xE9] = 4;
	m_OPCodesCycles[0xEA] = 16;
	m_OPCodesCycles[0xEB] = 0; // undefined
	m_OPCodesCycles[0xEC] = 0; // undefined
	m_OPCodesCycles[0xED] = 0; // undefined
	m_OPCodesCycles[0xEE] = 8;
	m_OPCodesCycles[0xEF] = 16;
	m_OPCodesCycles[0xF0] = 12;
	m_OPCodesCycles[0xF1] = 12;
	m_OPCodesCycles[0xF2] = 8;
	m_OPCodesCycles[0xF3] = 4;
	m_OPCodesCycles[0xF4] = 0; // undefined
	m_OPCodesCycles[0xF5] = 16;
	m_OPCodesCycles[0xF6] = 8;
	m_OPCodesCycles[0xF7] = 16;
	m_OPCodesCycles[0xF8] = 12;
	m_OPCodesCycles[0xF9] = 8;
	m_OPCodesCycles[0xFA] = 16;
	m_OPCodesCycles[0xFB] = 4;
	m_OPCodesCycles[0xFC] = 0; // undefined
	m_OPCodesCycles[0xFD] = 0; // undefined
	m_OPCodesCycles[0xFE] = 8;
	m_OPCodesCycles[0xFF] = 16;

	m_OPCodesConditionalCycles[0x00] = 4;
	m_OPCodesConditionalCycles[0x01] = 12;
	m_OPCodesConditionalCycles[0x02] = 8;
	m_OPCodesConditionalCycles[0x03] = 8;
	m_OPCodesConditionalCycles[0x04] = 4;
	m_OPCodesConditionalCycles[0x05] = 4;
	m_OPCodesConditionalCycles[0x06] = 8;
	m_OPCodesConditionalCycles[0x07] = 4;
	m_OPCodesConditionalCycles[0x08] = 20;
	m_OPCodesConditionalCycles[0x09] = 8;
	m_OPCodesConditionalCycles[0x0A] = 8;
	m_OPCodesConditionalCycles[0x0B] = 8;
	m_OPCodesConditionalCycles[0x0C] = 4;
	m_OPCodesConditionalCycles[0x0D] = 4;
	m_OPCodesConditionalCycles[0x0E] = 8;
	m_OPCodesConditionalCycles[0x0F] = 4;
	m_OPCodesConditionalCycles[0x10] = 4;
	m_OPCodesConditionalCycles[0x11] = 12;
	m_OPCodesConditionalCycles[0x12] = 8;
	m_OPCodesConditionalCycles[0x13] = 8;
	m_OPCodesConditionalCycles[0x14] = 4;
	m_OPCodesConditionalCycles[0x15] = 4;
	m_OPCodesConditionalCycles[0x16] = 8;
	m_OPCodesConditionalCycles[0x17] = 4;
	m_OPCodesConditionalCycles[0x18] = 12;
	m_OPCodesConditionalCycles[0x19] = 8;
	m_OPCodesConditionalCycles[0x1A] = 8;
	m_OPCodesConditionalCycles[0x1B] = 8;
	m_OPCodesConditionalCycles[0x1C] = 4;
	m_OPCodesConditionalCycles[0x1D] = 4;
	m_OPCodesConditionalCycles[0x1E] = 8;
	m_OPCodesConditionalCycles[0x1F] = 4;
	m_OPCodesConditionalCycles[0x20] = 12;
	m_OPCodesConditionalCycles[0x21] = 12;
	m_OPCodesConditionalCycles[0x22] = 8;
	m_OPCodesConditionalCycles[0x23] = 8;
	m_OPCodesConditionalCycles[0x24] = 4;
	m_OPCodesConditionalCycles[0x25] = 4;
	m_OPCodesConditionalCycles[0x26] = 8;
	m_OPCodesConditionalCycles[0x27] = 4;
	m_OPCodesConditionalCycles[0x28] = 12;
	m_OPCodesConditionalCycles[0x29] = 8;
	m_OPCodesConditionalCycles[0x2A] = 8;
	m_OPCodesConditionalCycles[0x2B] = 8;
	m_OPCodesConditionalCycles[0x2C] = 4;
	m_OPCodesConditionalCycles[0x2D] = 4;
	m_OPCodesConditionalCycles[0x2E] = 8;
	m_OPCodesConditionalCycles[0x2F] = 4;
	m_OPCodesConditionalCycles[0x30] = 12;
	m_OPCodesConditionalCycles[0x31] = 12;
	m_OPCodesConditionalCycles[0x32] = 8;
	m_OPCodesConditionalCycles[0x33] = 8;
	m_OPCodesConditionalCycles[0x34] = 12;
	m_OPCodesConditionalCycles[0x35] = 12;
	m_OPCodesConditionalCycles[0x36] = 12;
	m_OPCodesConditionalCycles[0x37] = 4;
	m_OPCodesConditionalCycles[0x38] = 12;
	m_OPCodesConditionalCycles[0x39] = 8;
	m_OPCodesConditionalCycles[0x3A] = 8;
	m_OPCodesConditionalCycles[0x3B] = 8;
	m_OPCodesConditionalCycles[0x3C] = 4;
	m_OPCodesConditionalCycles[0x3D] = 4;
	m_OPCodesConditionalCycles[0x3E] = 8;
	m_OPCodesConditionalCycles[0x3F] = 4;
	m_OPCodesConditionalCycles[0x40] = 4;
	m_OPCodesConditionalCycles[0x41] = 4;
	m_OPCodesConditionalCycles[0x42] = 4;
	m_OPCodesConditionalCycles[0x43] = 4;
	m_OPCodesConditionalCycles[0x44] = 4;
	m_OPCodesConditionalCycles[0x45] = 4;
	m_OPCodesConditionalCycles[0x46] = 8;
	m_OPCodesConditionalCycles[0x47] = 4;
	m_OPCodesConditionalCycles[0x48] = 4;
	m_OPCodesConditionalCycles[0x49] = 4;
	m_OPCodesConditionalCycles[0x4A] = 4;
	m_OPCodesConditionalCycles[0x4B] = 4;
	m_OPCodesConditionalCycles[0x4C] = 4;
	m_OPCodesConditionalCycles[0x4D] = 4;
	m_OPCodesConditionalCycles[0x4E] = 8;
	m_OPCodesConditionalCycles[0x4F] = 4;
	m_OPCodesConditionalCycles[0x50] = 4;
	m_OPCodesConditionalCycles[0x51] = 4;
	m_OPCodesConditionalCycles[0x52] = 4;
	m_OPCodesConditionalCycles[0x53] = 4;
	m_OPCodesConditionalCycles[0x54] = 4;
	m_OPCodesConditionalCycles[0x55] = 4;
	m_OPCodesConditionalCycles[0x56] = 8;
	m_OPCodesConditionalCycles[0x57] = 4;
	m_OPCodesConditionalCycles[0x58] = 4;
	m_OPCodesConditionalCycles[0x59] = 4;
	m_OPCodesConditionalCycles[0x5A] = 4;
	m_OPCodesConditionalCycles[0x5B] = 4;
	m_OPCodesConditionalCycles[0x5C] = 4;
	m_OPCodesConditionalCycles[0x5D] = 4;
	m_OPCodesConditionalCycles[0x5E] = 8;
	m_OPCodesConditionalCycles[0x5F] = 4;
	m_OPCodesConditionalCycles[0x60] = 4;
	m_OPCodesConditionalCycles[0x61] = 4;
	m_OPCodesConditionalCycles[0x62] = 4;
	m_OPCodesConditionalCycles[0x63] = 4;
	m_OPCodesConditionalCycles[0x64] = 4;
	m_OPCodesConditionalCycles[0x65] = 4;
	m_OPCodesConditionalCycles[0x66] = 8;
	m_OPCodesConditionalCycles[0x67] = 4;
	m_OPCodesConditionalCycles[0x68] = 4;
	m_OPCodesConditionalCycles[0x69] = 4;
	m_OPCodesConditionalCycles[0x6A] = 4;
	m_OPCodesConditionalCycles[0x6B] = 4;
	m_OPCodesConditionalCycles[0x6C] = 4;
	m_OPCodesConditionalCycles[0x6D] = 4;
	m_OPCodesConditionalCycles[0x6E] = 8;
	m_OPCodesConditionalCycles[0x6F] = 4;
	m_OPCodesConditionalCycles[0x70] = 8;
	m_OPCodesConditionalCycles[0x71] = 8;
	m_OPCodesConditionalCycles[0x72] = 8;
	m_OPCodesConditionalCycles[0x73] = 8;
	m_OPCodesConditionalCycles[0x74] = 8;
	m_OPCodesConditionalCycles[0x75] = 8;
	m_OPCodesConditionalCycles[0x76] = 4;
	m_OPCodesConditionalCycles[0x77] = 8;
	m_OPCodesConditionalCycles[0x78] = 4;
	m_OPCodesConditionalCycles[0x79] = 4;
	m_OPCodesConditionalCycles[0x7A] = 4;
	m_OPCodesConditionalCycles[0x7B] = 4;
	m_OPCodesConditionalCycles[0x7C] = 4;
	m_OPCodesConditionalCycles[0x7D] = 4;
	m_OPCodesConditionalCycles[0x7E] = 8;
	m_OPCodesConditionalCycles[0x7F] = 4;
	m_OPCodesConditionalCycles[0x80] = 4;
	m_OPCodesConditionalCycles[0x81] = 4;
	m_OPCodesConditionalCycles[0x82] = 4;
	m_OPCodesConditionalCycles[0x83] = 4;
	m_OPCodesConditionalCycles[0x84] = 4;
	m_OPCodesConditionalCycles[0x85] = 4;
	m_OPCodesConditionalCycles[0x86] = 8;
	m_OPCodesConditionalCycles[0x87] = 4;
	m_OPCodesConditionalCycles[0x88] = 4;
	m_OPCodesConditionalCycles[0x89] = 4;
	m_OPCodesConditionalCycles[0x8A] = 4;
	m_OPCodesConditionalCycles[0x8B] = 4;
	m_OPCodesConditionalCycles[0x8C] = 4;
	m_OPCodesConditionalCycles[0x8D] = 4;
	m_OPCodesConditionalCycles[0x8E] = 8;
	m_OPCodesConditionalCycles[0x8F] = 4;
	m_OPCodesConditionalCycles[0x90] = 4;
	m_OPCodesConditionalCycles[0x91] = 4;
	m_OPCodesConditionalCycles[0x92] = 4;
	m_OPCodesConditionalCycles[0x93] = 4;
	m_OPCodesConditionalCycles[0x94] = 4;
	m_OPCodesConditionalCycles[0x95] = 4;
	m_OPCodesConditionalCycles[0x96] = 8;
	m_OPCodesConditionalCycles[0x97] = 4;
	m_OPCodesConditionalCycles[0x98] = 4;
	m_OPCodesConditionalCycles[0x99] = 4;
	m_OPCodesConditionalCycles[0x9A] = 4;
	m_OPCodesConditionalCycles[0x9B] = 4;
	m_OPCodesConditionalCycles[0x9C] = 4;
	m_OPCodesConditionalCycles[0x9D] = 4;
	m_OPCodesConditionalCycles[0x9E] = 8;
	m_OPCodesConditionalCycles[0x9F] = 4;
	m_OPCodesConditionalCycles[0xA0] = 4;
	m_OPCodesConditionalCycles[0xA1] = 4;
	m_OPCodesConditionalCycles[0xA2] = 4;
	m_OPCodesConditionalCycles[0xA3] = 4;
	m_OPCodesConditionalCycles[0xA4] = 4;
	m_OPCodesConditionalCycles[0xA5] = 4;
	m_OPCodesConditionalCycles[0xA6] = 8;
	m_OPCodesConditionalCycles[0xA7] = 4;
	m_OPCodesConditionalCycles[0xA8] = 4;
	m_OPCodesConditionalCycles[0xA9] = 4;
	m_OPCodesConditionalCycles[0xAA] = 4;
	m_OPCodesConditionalCycles[0xAB] = 4;
	m_OPCodesConditionalCycles[0xAC] = 4;
	m_OPCodesConditionalCycles[0xAD] = 4;
	m_OPCodesConditionalCycles[0xAE] = 8;
	m_OPCodesConditionalCycles[0xAF] = 4;
	m_OPCodesConditionalCycles[0xB0] = 4;
	m_OPCodesConditionalCycles[0xB1] = 4;
	m_OPCodesConditionalCycles[0xB2] = 4;
	m_OPCodesConditionalCycles[0xB3] = 4;
	m_OPCodesConditionalCycles[0xB4] = 4;
	m_OPCodesConditionalCycles[0xB5] = 4;
	m_OPCodesConditionalCycles[0xB6] = 8;
	m_OPCodesConditionalCycles[0xB7] = 4;
	m_OPCodesConditionalCycles[0xB8] = 4;
	m_OPCodesConditionalCycles[0xB9] = 4;
	m_OPCodesConditionalCycles[0xBA] = 4;
	m_OPCodesConditionalCycles[0xBB] = 4;
	m_OPCodesConditionalCycles[0xBC] = 4;
	m_OPCodesConditionalCycles[0xBD] = 4;
	m_OPCodesConditionalCycles[0xBE] = 8;
	m_OPCodesConditionalCycles[0xBF] = 4;
	m_OPCodesConditionalCycles[0xC0] = 20;
	m_OPCodesConditionalCycles[0xC1] = 12;
	m_OPCodesConditionalCycles[0xC2] = 16;
	m_OPCodesConditionalCycles[0xC3] = 16;
	m_OPCodesConditionalCycles[0xC4] = 24;
	m_OPCodesConditionalCycles[0xC5] = 16;
	m_OPCodesConditionalCycles[0xC6] = 8;
	m_OPCodesConditionalCycles[0xC7] = 16;
	m_OPCodesConditionalCycles[0xC8] = 20;
	m_OPCodesConditionalCycles[0xC9] = 16;
	m_OPCodesConditionalCycles[0xCA] = 16;
	m_OPCodesConditionalCycles[0xCB] = 0; // gate to the extended opcodes
	m_OPCodesConditionalCycles[0xCC] = 24;
	m_OPCodesConditionalCycles[0xCD] = 24;
	m_OPCodesConditionalCycles[0xCE] = 8;
	m_OPCodesConditionalCycles[0xCF] = 16;
	m_OPCodesConditionalCycles[0xD0] = 20;
	m_OPCodesConditionalCycles[0xD1] = 12;
	m_OPCodesConditionalCycles[0xD2] = 16;
	m_OPCodesConditionalCycles[0xD3] = 0; // undefined
	m_OPCodesConditionalCycles[0xD4] = 24;
	m_OPCodesConditionalCycles[0xD5] = 16;
	m_OPCodesConditionalCycles[0xD6] = 8;
	m_OPCodesConditionalCycles[0xD7] = 16;
	m_OPCodesConditionalCycles[0xD8] = 20;
	m_OPCodesConditionalCycles[0xD9] = 16;
	m_OPCodesConditionalCycles[0xDA] = 16;
	m_OPCodesConditionalCycles[0xDB] = 0; // undefined
	m_OPCodesConditionalCycles[0xDC] = 24;
	m_OPCodesConditionalCycles[0xDD] = 0; // undefined
	m_OPCodesConditionalCycles[0xDE] = 8;
	m_OPCodesConditionalCycles[0xDF] = 16;
	m_OPCodesConditionalCycles[0xE0] = 12;
	m_OPCodesConditionalCycles[0xE1] = 12;
	m_OPCodesConditionalCycles[0xE2] = 8;
	m_OPCodesConditionalCycles[0xE3] = 0; // undefined
	m_OPCodesConditionalCycles[0xE4] = 0; // undefined
	m_OPCodesConditionalCycles[0xE5] = 16;
	m_OPCodesConditionalCycles[0xE6] = 8;
	m_OPCodesConditionalCycles[0xE7] = 16;
	m_OPCodesConditionalCycles[0xE8] = 16;
	m_OPCodesConditionalCycles[0xE9] = 4;
	m_OPCodesConditionalCycles[0xEA] = 16;
	m_OPCodesConditionalCycles[0xEB] = 0; // undefined
	m_OPCodesConditionalCycles[0xEC] = 0; // undefined
	m_OPCodesConditionalCycles[0xED] = 0; // undefined
	m_OPCodesConditionalCycles[0xEE] = 8;
	m_OPCodesConditionalCycles[0xEF] = 16;
	m_OPCodesConditionalCycles[0xF0] = 12;
	m_OPCodesConditionalCycles[0xF1] = 12;
	m_OPCodesConditionalCycles[0xF2] = 8;
	m_OPCodesConditionalCycles[0xF3] = 4;
	m_OPCodesConditionalCycles[0xF4] = 0; // undefined
	m_OPCodesConditionalCycles[0xF5] = 16;
	m_OPCodesConditionalCycles[0xF6] = 8;
	m_OPCodesConditionalCycles[0xF7] = 16;
	m_OPCodesConditionalCycles[0xF8] = 12;
	m_OPCodesConditionalCycles[0xF9] = 8;
	m_OPCodesConditionalCycles[0xFA] = 16;
	m_OPCodesConditionalCycles[0xFB] = 4;
	m_OPCodesConditionalCycles[0xFC] = 0; // undefined
	m_OPCodesConditionalCycles[0xFD] = 0; // undefined
	m_OPCodesConditionalCycles[0xFE] = 8;
	m_OPCodesConditionalCycles[0xFF] = 16;
}

void CPU::initExtendedOPCodes()
{
	m_ExtendedOPCodes[0x00] = &CPU::OPCode_CB_00;
	m_ExtendedOPCodes[0x01] = &CPU::OPCode_CB_01;
	m_ExtendedOPCodes[0x02] = &CPU::OPCode_CB_02;
	m_ExtendedOPCodes[0x03] = &CPU::OPCode_CB_03;
	m_ExtendedOPCodes[0x04] = &CPU::OPCode_CB_04;
	m_ExtendedOPCodes[0x05] = &CPU::OPCode_CB_05;
	m_ExtendedOPCodes[0x06] = &CPU::OPCode_CB_06;
	m_ExtendedOPCodes[0x07] = &CPU::OPCode_CB_07;
	m_ExtendedOPCodes[0x08] = &CPU::OPCode_CB_08;
	m_ExtendedOPCodes[0x09] = &CPU::OPCode_CB_09;
	m_ExtendedOPCodes[0x0A] = &CPU::OPCode_CB_0A;
	m_ExtendedOPCodes[0x0B] = &CPU::OPCode_CB_0B;
	m_ExtendedOPCodes[0x0C] = &CPU::OPCode_CB_0C;
	m_ExtendedOPCodes[0x0D] = &CPU::OPCode_CB_0D;
	m_ExtendedOPCodes[0x0E] = &CPU::OPCode_CB_0E;
	m_ExtendedOPCodes[0x0F] = &CPU::OPCode_CB_0F;
	m_ExtendedOPCodes[0x10] = &CPU::OPCode_CB_10;
	m_ExtendedOPCodes[0x11] = &CPU::OPCode_CB_11;
	m_ExtendedOPCodes[0x12] = &CPU::OPCode_CB_12;
	m_ExtendedOPCodes[0x13] = &CPU::OPCode_CB_13;
	m_ExtendedOPCodes[0x14] = &CPU::OPCode_CB_14;
	m_ExtendedOPCodes[0x15] = &CPU::OPCode_CB_15;
	m_ExtendedOPCodes[0x16] = &CPU::OPCode_CB_16;
	m_ExtendedOPCodes[0x17] = &CPU::OPCode_CB_17;
	m_ExtendedOPCodes[0x18] = &CPU::OPCode_CB_18;
	m_ExtendedOPCodes[0x19] = &CPU::OPCode_CB_19;
	m_ExtendedOPCodes[0x1A] = &CPU::OPCode_CB_1A;
	m_ExtendedOPCodes[0x1B] = &CPU::OPCode_CB_1B;
	m_ExtendedOPCodes[0x1C] = &CPU::OPCode_CB_1C;
	m_ExtendedOPCodes[0x1D] = &CPU::OPCode_CB_1D;
	m_ExtendedOPCodes[0x1E] = &CPU::OPCode_CB_1E;
	m_ExtendedOPCodes[0x1F] = &CPU::OPCode_CB_1F;
	m_ExtendedOPCodes[0x20] = &CPU::OPCode_CB_20;
	m_ExtendedOPCodes[0x21] = &CPU::OPCode_CB_21;
	m_ExtendedOPCodes[0x22] = &CPU::OPCode_CB_22;
	m_ExtendedOPCodes[0x23] = &CPU::OPCode_CB_23;
	m_ExtendedOPCodes[0x24] = &CPU::OPCode_CB_24;
	m_ExtendedOPCodes[0x25] = &CPU::OPCode_CB_25;
	m_ExtendedOPCodes[0x26] = &CPU::OPCode_CB_26;
	m_ExtendedOPCodes[0x27] = &CPU::OPCode_CB_27;
	m_ExtendedOPCodes[0x28] = &CPU::OPCode_CB_28;
	m_ExtendedOPCodes[0x29] = &CPU::OPCode_CB_29;
	m_ExtendedOPCodes[0x2A] = &CPU::OPCode_CB_2A;
	m_ExtendedOPCodes[0x2B] = &CPU::OPCode_CB_2B;
	m_ExtendedOPCodes[0x2C] = &CPU::OPCode_CB_2C;
	m_ExtendedOPCodes[0x2D] = &CPU::OPCode_CB_2D;
	m_ExtendedOPCodes[0x2E] = &CPU::OPCode_CB_2E;
	m_ExtendedOPCodes[0x2F] = &CPU::OPCode_CB_2F;
	m_ExtendedOPCodes[0x30] = &CPU::OPCode_CB_30;
	m_ExtendedOPCodes[0x31] = &CPU::OPCode_CB_31;
	m_ExtendedOPCodes[0x32] = &CPU::OPCode_CB_32;
	m_ExtendedOPCodes[0x33] = &CPU::OPCode_CB_33;
	m_ExtendedOPCodes[0x34] = &CPU::OPCode_CB_34;
	m_ExtendedOPCodes[0x35] = &CPU::OPCode_CB_35;
	m_ExtendedOPCodes[0x36] = &CPU::OPCode_CB_36;
	m_ExtendedOPCodes[0x37] = &CPU::OPCode_CB_37;
	m_ExtendedOPCodes[0x38] = &CPU::OPCode_CB_38;
	m_ExtendedOPCodes[0x39] = &CPU::OPCode_CB_39;
	m_ExtendedOPCodes[0x3A] = &CPU::OPCode_CB_3A;
	m_ExtendedOPCodes[0x3B] = &CPU::OPCode_CB_3B;
	m_ExtendedOPCodes[0x3C] = &CPU::OPCode_CB_3C;
	m_ExtendedOPCodes[0x3D] = &CPU::OPCode_CB_3D;
	m_ExtendedOPCodes[0x3E] = &CPU::OPCode_CB_3E;
	m_ExtendedOPCodes[0x3F] = &CPU::OPCode_CB_3F;
	m_ExtendedOPCodes[0x40] = &CPU::OPCode_CB_40;
	m_ExtendedOPCodes[0x41] = &CPU::OPCode_CB_41;
	m_ExtendedOPCodes[0x42] = &CPU::OPCode_CB_42;
	m_ExtendedOPCodes[0x43] = &CPU::OPCode_CB_43;
	m_ExtendedOPCodes[0x44] = &CPU::OPCode_CB_44;
	m_ExtendedOPCodes[0x45] = &CPU::OPCode_CB_45;
	m_ExtendedOPCodes[0x46] = &CPU::OPCode_CB_46;
	m_ExtendedOPCodes[0x47] = &CPU::OPCode_CB_47;
	m_ExtendedOPCodes[0x48] = &CPU::OPCode_CB_48;
	m_ExtendedOPCodes[0x49] = &CPU::OPCode_CB_49;
	m_ExtendedOPCodes[0x4A] = &CPU::OPCode_CB_4A;
	m_ExtendedOPCodes[0x4B] = &CPU::OPCode_CB_4B;
	m_ExtendedOPCodes[0x4C] = &CPU::OPCode_CB_4C;
	m_ExtendedOPCodes[0x4D] = &CPU::OPCode_CB_4D;
	m_ExtendedOPCodes[0x4E] = &CPU::OPCode_CB_4E;
	m_ExtendedOPCodes[0x4F] = &CPU::OPCode_CB_4F;
	m_ExtendedOPCodes[0x50] = &CPU::OPCode_CB_50;
	m_ExtendedOPCodes[0x51] = &CPU::OPCode_CB_51;
	m_ExtendedOPCodes[0x52] = &CPU::OPCode_CB_52;
	m_ExtendedOPCodes[0x53] = &CPU::OPCode_CB_53;
	m_ExtendedOPCodes[0x54] = &CPU::OPCode_CB_54;
	m_ExtendedOPCodes[0x55] = &CPU::OPCode_CB_55;
	m_ExtendedOPCodes[0x56] = &CPU::OPCode_CB_56;
	m_ExtendedOPCodes[0x57] = &CPU::OPCode_CB_57;
	m_ExtendedOPCodes[0x58] = &CPU::OPCode_CB_58;
	m_ExtendedOPCodes[0x59] = &CPU::OPCode_CB_59;
	m_ExtendedOPCodes[0x5A] = &CPU::OPCode_CB_5A;
	m_ExtendedOPCodes[0x5B] = &CPU::OPCode_CB_5B;
	m_ExtendedOPCodes[0x5C] = &CPU::OPCode_CB_5C;
	m_ExtendedOPCodes[0x5D] = &CPU::OPCode_CB_5D;
	m_ExtendedOPCodes[0x5E] = &CPU::OPCode_CB_5E;
	m_ExtendedOPCodes[0x5F] = &CPU::OPCode_CB_5F;
	m_ExtendedOPCodes[0x60] = &CPU::OPCode_CB_60;
	m_ExtendedOPCodes[0x61] = &CPU::OPCode_CB_61;
	m_ExtendedOPCodes[0x62] = &CPU::OPCode_CB_62;
	m_ExtendedOPCodes[0x63] = &CPU::OPCode_CB_63;
	m_ExtendedOPCodes[0x64] = &CPU::OPCode_CB_64;
	m_ExtendedOPCodes[0x65] = &CPU::OPCode_CB_65;
	m_ExtendedOPCodes[0x66] = &CPU::OPCode_CB_66;
	m_ExtendedOPCodes[0x67] = &CPU::OPCode_CB_67;
	m_ExtendedOPCodes[0x68] = &CPU::OPCode_CB_68;
	m_ExtendedOPCodes[0x69] = &CPU::OPCode_CB_69;
	m_ExtendedOPCodes[0x6A] = &CPU::OPCode_CB_6A;
	m_ExtendedOPCodes[0x6B] = &CPU::OPCode_CB_6B;
	m_ExtendedOPCodes[0x6C] = &CPU::OPCode_CB_6C;
	m_ExtendedOPCodes[0x6D] = &CPU::OPCode_CB_6D;
	m_ExtendedOPCodes[0x6E] = &CPU::OPCode_CB_6E;
	m_ExtendedOPCodes[0x6F] = &CPU::OPCode_CB_6F;
	m_ExtendedOPCodes[0x70] = &CPU::OPCode_CB_70;
	m_ExtendedOPCodes[0x71] = &CPU::OPCode_CB_71;
	m_ExtendedOPCodes[0x72] = &CPU::OPCode_CB_72;
	m_ExtendedOPCodes[0x73] = &CPU::OPCode_CB_73;
	m_ExtendedOPCodes[0x74] = &CPU::OPCode_CB_74;
	m_ExtendedOPCodes[0x75] = &CPU::OPCode_CB_75;
	m_ExtendedOPCodes[0x76] = &CPU::OPCode_CB_76;
	m_ExtendedOPCodes[0x77] = &CPU::OPCode_CB_77;
	m_ExtendedOPCodes[0x78] = &CPU::OPCode_CB_78;
	m_ExtendedOPCodes[0x79] = &CPU::OPCode_CB_79;
	m_ExtendedOPCodes[0x7A] = &CPU::OPCode_CB_7A;
	m_ExtendedOPCodes[0x7B] = &CPU::OPCode_CB_7B;
	m_ExtendedOPCodes[0x7C] = &CPU::OPCode_CB_7C;
	m_ExtendedOPCodes[0x7D] = &CPU::OPCode_CB_7D;
	m_ExtendedOPCodes[0x7E] = &CPU::OPCode_CB_7E;
	m_ExtendedOPCodes[0x7F] = &CPU::OPCode_CB_7F;
	m_ExtendedOPCodes[0x80] = &CPU::OPCode_CB_80;
	m_ExtendedOPCodes[0x81] = &CPU::OPCode_CB_81;
	m_ExtendedOPCodes[0x82] = &CPU::OPCode_CB_82;
	m_ExtendedOPCodes[0x83] = &CPU::OPCode_CB_83;
	m_ExtendedOPCodes[0x84] = &CPU::OPCode_CB_84;
	m_ExtendedOPCodes[0x85] = &CPU::OPCode_CB_85;
	m_ExtendedOPCodes[0x86] = &CPU::OPCode_CB_86;
	m_ExtendedOPCodes[0x87] = &CPU::OPCode_CB_87;
	m_ExtendedOPCodes[0x88] = &CPU::OPCode_CB_88;
	m_ExtendedOPCodes[0x89] = &CPU::OPCode_CB_89;
	m_ExtendedOPCodes[0x8A] = &CPU::OPCode_CB_8A;
	m_ExtendedOPCodes[0x8B] = &CPU::OPCode_CB_8B;
	m_ExtendedOPCodes[0x8C] = &CPU::OPCode_CB_8C;
	m_ExtendedOPCodes[0x8D] = &CPU::OPCode_CB_8D;
	m_ExtendedOPCodes[0x8E] = &CPU::OPCode_CB_8E;
	m_ExtendedOPCodes[0x8F] = &CPU::OPCode_CB_8F;
	m_ExtendedOPCodes[0x90] = &CPU::OPCode_CB_90;
	m_ExtendedOPCodes[0x91] = &CPU::OPCode_CB_91;
	m_ExtendedOPCodes[0x92] = &CPU::OPCode_CB_92;
	m_ExtendedOPCodes[0x93] = &CPU::OPCode_CB_93;
	m_ExtendedOPCodes[0x94] = &CPU::OPCode_CB_94;
	m_ExtendedOPCodes[0x95] = &CPU::OPCode_CB_95;
	m_ExtendedOPCodes[0x96] = &CPU::OPCode_CB_96;
	m_ExtendedOPCodes[0x97] = &CPU::OPCode_CB_97;
	m_ExtendedOPCodes[0x98] = &CPU::OPCode_CB_98;
	m_ExtendedOPCodes[0x99] = &CPU::OPCode_CB_99;
	m_ExtendedOPCodes[0x9A] = &CPU::OPCode_CB_9A;
	m_ExtendedOPCodes[0x9B] = &CPU::OPCode_CB_9B;
	m_ExtendedOPCodes[0x9C] = &CPU::OPCode_CB_9C;
	m_ExtendedOPCodes[0x9D] = &CPU::OPCode_CB_9D;
	m_ExtendedOPCodes[0x9E] = &CPU::OPCode_CB_9E;
	m_ExtendedOPCodes[0x9F] = &CPU::OPCode_CB_9F;
	m_ExtendedOPCodes[0xA0] = &CPU::OPCode_CB_A0;
	m_ExtendedOPCodes[0xA1] = &CPU::OPCode_CB_A1;
	m_ExtendedOPCodes[0xA2] = &CPU::OPCode_CB_A2;
	m_ExtendedOPCodes[0xA3] = &CPU::OPCode_CB_A3;
	m_ExtendedOPCodes[0xA4] = &CPU::OPCode_CB_A4;
	m_ExtendedOPCodes[0xA5] = &CPU::OPCode_CB_A5;
	m_ExtendedOPCodes[0xA6] = &CPU::OPCode_CB_A6;
	m_ExtendedOPCodes[0xA7] = &CPU::OPCode_CB_A7;
	m_ExtendedOPCodes[0xA8] = &CPU::OPCode_CB_A8;
	m_ExtendedOPCodes[0xA9] = &CPU::OPCode_CB_A9;
	m_ExtendedOPCodes[0xAA] = &CPU::OPCode_CB_AA;
	m_ExtendedOPCodes[0xAB] = &CPU::OPCode_CB_AB;
	m_ExtendedOPCodes[0xAC] = &CPU::OPCode_CB_AC;
	m_ExtendedOPCodes[0xAD] = &CPU::OPCode_CB_AD;
	m_ExtendedOPCodes[0xAE] = &CPU::OPCode_CB_AE;
	m_ExtendedOPCodes[0xAF] = &CPU::OPCode_CB_AF;
	m_ExtendedOPCodes[0xB0] = &CPU::OPCode_CB_B0;
	m_ExtendedOPCodes[0xB1] = &CPU::OPCode_CB_B1;
	m_ExtendedOPCodes[0xB2] = &CPU::OPCode_CB_B2;
	m_ExtendedOPCodes[0xB3] = &CPU::OPCode_CB_B3;
	m_ExtendedOPCodes[0xB4] = &CPU::OPCode_CB_B4;
	m_ExtendedOPCodes[0xB5] = &CPU::OPCode_CB_B5;
	m_ExtendedOPCodes[0xB6] = &CPU::OPCode_CB_B6;
	m_ExtendedOPCodes[0xB7] = &CPU::OPCode_CB_B7;
	m_ExtendedOPCodes[0xB8] = &CPU::OPCode_CB_B8;
	m_ExtendedOPCodes[0xB9] = &CPU::OPCode_CB_B9;
	m_ExtendedOPCodes[0xBA] = &CPU::OPCode_CB_BA;
	m_ExtendedOPCodes[0xBB] = &CPU::OPCode_CB_BB;
	m_ExtendedOPCodes[0xBC] = &CPU::OPCode_CB_BC;
	m_ExtendedOPCodes[0xBD] = &CPU::OPCode_CB_BD;
	m_ExtendedOPCodes[0xBE] = &CPU::OPCode_CB_BE;
	m_ExtendedOPCodes[0xBF] = &CPU::OPCode_CB_BF;
	m_ExtendedOPCodes[0xC0] = &CPU::OPCode_CB_C0;
	m_ExtendedOPCodes[0xC1] = &CPU::OPCode_CB_C1;
	m_ExtendedOPCodes[0xC2] = &CPU::OPCode_CB_C2;
	m_ExtendedOPCodes[0xC3] = &CPU::OPCode_CB_C3;
	m_ExtendedOPCodes[0xC4] = &CPU::OPCode_CB_C4;
	m_ExtendedOPCodes[0xC5] = &CPU::OPCode_CB_C5;
	m_ExtendedOPCodes[0xC6] = &CPU::OPCode_CB_C6;
	m_ExtendedOPCodes[0xC7] = &CPU::OPCode_CB_C7;
	m_ExtendedOPCodes[0xC8] = &CPU::OPCode_CB_C8;
	m_ExtendedOPCodes[0xC9] = &CPU::OPCode_CB_C9;
	m_ExtendedOPCodes[0xCA] = &CPU::OPCode_CB_CA;
	m_ExtendedOPCodes[0xCB] = &CPU::OPCode_CB_CB;
	m_ExtendedOPCodes[0xCC] = &CPU::OPCode_CB_CC;
	m_ExtendedOPCodes[0xCD] = &CPU::OPCode_CB_CD;
	m_ExtendedOPCodes[0xCE] = &CPU::OPCode_CB_CE;
	m_ExtendedOPCodes[0xCF] = &CPU::OPCode_CB_CF;
	m_ExtendedOPCodes[0xD0] = &CPU::OPCode_CB_D0;
	m_ExtendedOPCodes[0xD1] = &CPU::OPCode_CB_D1;
	m_ExtendedOPCodes[0xD2] = &CPU::OPCode_CB_D2;
	m_ExtendedOPCodes[0xD3] = &CPU::OPCode_CB_D3;
	m_ExtendedOPCodes[0xD4] = &CPU::OPCode_CB_D4;
	m_ExtendedOPCodes[0xD5] = &CPU::OPCode_CB_D5;
	m_ExtendedOPCodes[0xD6] = &CPU::OPCode_CB_D6;
	m_ExtendedOPCodes[0xD7] = &CPU::OPCode_CB_D7;
	m_ExtendedOPCodes[0xD8] = &CPU::OPCode_CB_D8;
	m_ExtendedOPCodes[0xD9] = &CPU::OPCode_CB_D9;
	m_ExtendedOPCodes[0xDA] = &CPU::OPCode_CB_DA;
	m_ExtendedOPCodes[0xDB] = &CPU::OPCode_CB_DB;
	m_ExtendedOPCodes[0xDC] = &CPU::OPCode_CB_DC;
	m_ExtendedOPCodes[0xDD] = &CPU::OPCode_CB_DD;
	m_ExtendedOPCodes[0xDE] = &CPU::OPCode_CB_DE;
	m_ExtendedOPCodes[0xDF] = &CPU::OPCode_CB_DF;
	m_ExtendedOPCodes[0xE0] = &CPU::OPCode_CB_E0;
	m_ExtendedOPCodes[0xE1] = &CPU::OPCode_CB_E1;
	m_ExtendedOPCodes[0xE2] = &CPU::OPCode_CB_E2;
	m_ExtendedOPCodes[0xE3] = &CPU::OPCode_CB_E3;
	m_ExtendedOPCodes[0xE4] = &CPU::OPCode_CB_E4;
	m_ExtendedOPCodes[0xE5] = &CPU::OPCode_CB_E5;
	m_ExtendedOPCodes[0xE6] = &CPU::OPCode_CB_E6;
	m_ExtendedOPCodes[0xE7] = &CPU::OPCode_CB_E7;
	m_ExtendedOPCodes[0xE8] = &CPU::OPCode_CB_E8;
	m_ExtendedOPCodes[0xE9] = &CPU::OPCode_CB_E9;
	m_ExtendedOPCodes[0xEA] = &CPU::OPCode_CB_EA;
	m_ExtendedOPCodes[0xEB] = &CPU::OPCode_CB_EB;
	m_ExtendedOPCodes[0xEC] = &CPU::OPCode_CB_EC;
	m_ExtendedOPCodes[0xED] = &CPU::OPCode_CB_ED;
	m_ExtendedOPCodes[0xEE] = &CPU::OPCode_CB_EE;
	m_ExtendedOPCodes[0xEF] = &CPU::OPCode_CB_EF;
	m_ExtendedOPCodes[0xF0] = &CPU::OPCode_CB_F0;
	m_ExtendedOPCodes[0xF1] = &CPU::OPCode_CB_F1;
	m_ExtendedOPCodes[0xF2] = &CPU::OPCode_CB_F2;
	m_ExtendedOPCodes[0xF3] = &CPU::OPCode_CB_F3;
	m_ExtendedOPCodes[0xF4] = &CPU::OPCode_CB_F4;
	m_ExtendedOPCodes[0xF5] = &CPU::OPCode_CB_F5;
	m_ExtendedOPCodes[0xF6] = &CPU::OPCode_CB_F6;
	m_ExtendedOPCodes[0xF7] = &CPU::OPCode_CB_F7;
	m_ExtendedOPCodes[0xF8] = &CPU::OPCode_CB_F8;
	m_ExtendedOPCodes[0xF9] = &CPU::OPCode_CB_F9;
	m_ExtendedOPCodes[0xFA] = &CPU::OPCode_CB_FA;
	m_ExtendedOPCodes[0xFB] = &CPU::OPCode_CB_FB;
	m_ExtendedOPCodes[0xFC] = &CPU::OPCode_CB_FC;
	m_ExtendedOPCodes[0xFD] = &CPU::OPCode_CB_FD;
	m_ExtendedOPCodes[0xFE] = &CPU::OPCode_CB_FE;
	m_ExtendedOPCodes[0xFF] = &CPU::OPCode_CB_FF;

	m_ExtendedOPCodesNames[0x00] = "RLC B";
	m_ExtendedOPCodesNames[0x01] = "RLC C";
	m_ExtendedOPCodesNames[0x02] = "RLC D";
	m_ExtendedOPCodesNames[0x03] = "RLC E";
	m_ExtendedOPCodesNames[0x04] = "RLC H";
	m_ExtendedOPCodesNames[0x05] = "RLC L";
	m_ExtendedOPCodesNames[0x06] = "RLC (HL)";
	m_ExtendedOPCodesNames[0x07] = "RLC A";
	m_ExtendedOPCodesNames[0x08] = "RRC B";
	m_ExtendedOPCodesNames[0x09] = "RRC C";
	m_ExtendedOPCodesNames[0x0A] = "RRC D";
	m_ExtendedOPCodesNames[0x0B] = "RRC E";
	m_ExtendedOPCodesNames[0x0C] = "RRC H";
	m_ExtendedOPCodesNames[0x0D] = "RRC L";
	m_ExtendedOPCodesNames[0x0E] = "RRC (HL)";
	m_ExtendedOPCodesNames[0x0F] = "RRC A";
	m_ExtendedOPCodesNames[0x10] = "RL B";
	m_ExtendedOPCodesNames[0x11] = "RL C";
	m_ExtendedOPCodesNames[0x12] = "RL D";
	m_ExtendedOPCodesNames[0x13] = "RL E";
	m_ExtendedOPCodesNames[0x14] = "RL H";
	m_ExtendedOPCodesNames[0x15] = "RL L";
	m_ExtendedOPCodesNames[0x16] = "RL (HL)";
	m_ExtendedOPCodesNames[0x17] = "RL A";
	m_ExtendedOPCodesNames[0x18] = "RR B";
	m_ExtendedOPCodesNames[0x19] = "RR C";
	m_ExtendedOPCodesNames[0x1A] = "RR D";
	m_ExtendedOPCodesNames[0x1B] = "RR E";
	m_ExtendedOPCodesNames[0x1C] = "RR H";
	m_ExtendedOPCodesNames[0x1D] = "RR L";
	m_ExtendedOPCodesNames[0x1E] = "RR (HL)";
	m_ExtendedOPCodesNames[0x1F] = "RR A";
	m_ExtendedOPCodesNames[0x20] = "SLA B";
	m_ExtendedOPCodesNames[0x21] = "SLA C";
	m_ExtendedOPCodesNames[0x22] = "SLA D";
	m_ExtendedOPCodesNames[0x23] = "SLA E";
	m_ExtendedOPCodesNames[0x24] = "SLA H";
	m_ExtendedOPCodesNames[0x25] = "SLA L";
	m_ExtendedOPCodesNames[0x26] = "SLA (HL)";
	m_ExtendedOPCodesNames[0x27] = "SLA A";
	m_ExtendedOPCodesNames[0x28] = "SRA B";
	m_ExtendedOPCodesNames[0x29] = "SRA C";
	m_ExtendedOPCodesNames[0x2A] = "SRA D";
	m_ExtendedOPCodesNames[0x2B] = "SRA E";
	m_ExtendedOPCodesNames[0x2C] = "SRA H";
	m_ExtendedOPCodesNames[0x2D] = "SRA L";
	m_ExtendedOPCodesNames[0x2E] = "SRA (HL)";
	m_ExtendedOPCodesNames[0x2F] = "SRA A";
	m_ExtendedOPCodesNames[0x30] = "SWAP B";
	m_ExtendedOPCodesNames[0x31] = "SWAP C";
	m_ExtendedOPCodesNames[0x32] = "SWAP D";
	m_ExtendedOPCodesNames[0x33] = "SWAP E";
	m_ExtendedOPCodesNames[0x34] = "SWAP H";
	m_ExtendedOPCodesNames[0x35] = "SWAP L";
	m_ExtendedOPCodesNames[0x36] = "SWAP (HL)";
	m_ExtendedOPCodesNames[0x37] = "SWAP A";
	m_ExtendedOPCodesNames[0x38] = "SRL B";
	m_ExtendedOPCodesNames[0x39] = "SRL C";
	m_ExtendedOPCodesNames[0x3A] = "SRL D";
	m_ExtendedOPCodesNames[0x3B] = "SRL E";
	m_ExtendedOPCodesNames[0x3C] = "SRL H";
	m_ExtendedOPCodesNames[0x3D] = "SRL L";
	m_ExtendedOPCodesNames[0x3E] = "SRL (HL)";
	m_ExtendedOPCodesNames[0x3F] = "SRL A";
	m_ExtendedOPCodesNames[0x40] = "BIT 0, B";
	m_ExtendedOPCodesNames[0x41] = "BIT 0, C";
	m_ExtendedOPCodesNames[0x42] = "BIT 0, D";
	m_ExtendedOPCodesNames[0x43] = "BIT 0, E";
	m_ExtendedOPCodesNames[0x44] = "BIT 0, H";
	m_ExtendedOPCodesNames[0x45] = "BIT 0, L";
	m_ExtendedOPCodesNames[0x46] = "BIT 0, (HL)";
	m_ExtendedOPCodesNames[0x47] = "BIT 0, A";
	m_ExtendedOPCodesNames[0x48] = "BIT 1, B";
	m_ExtendedOPCodesNames[0x49] = "BIT 1, C";
	m_ExtendedOPCodesNames[0x4A] = "BIT 1, D";
	m_ExtendedOPCodesNames[0x4B] = "BIT 1, E";
	m_ExtendedOPCodesNames[0x4C] = "BIT 1, H";
	m_ExtendedOPCodesNames[0x4D] = "BIT 1, L";
	m_ExtendedOPCodesNames[0x4E] = "BIT 1, (HL)";
	m_ExtendedOPCodesNames[0x4F] = "BIT 1, A";
	m_ExtendedOPCodesNames[0x50] = "BIT 2, B";
	m_ExtendedOPCodesNames[0x51] = "BIT 2, C";
	m_ExtendedOPCodesNames[0x52] = "BIT 2, D";
	m_ExtendedOPCodesNames[0x53] = "BIT 2, E";
	m_ExtendedOPCodesNames[0x54] = "BIT 2, H";
	m_ExtendedOPCodesNames[0x55] = "BIT 2, L";
	m_ExtendedOPCodesNames[0x56] = "BIT 2, (HL)";
	m_ExtendedOPCodesNames[0x57] = "BIT 2, A";
	m_ExtendedOPCodesNames[0x58] = "BIT 3, B";
	m_ExtendedOPCodesNames[0x59] = "BIT 3, C";
	m_ExtendedOPCodesNames[0x5A] = "BIT 3, D";
	m_ExtendedOPCodesNames[0x5B] = "BIT 3, E";
	m_ExtendedOPCodesNames[0x5C] = "BIT 3, H";
	m_ExtendedOPCodesNames[0x5D] = "BIT 3, L";
	m_ExtendedOPCodesNames[0x5E] = "BIT 3, (HL)";
	m_ExtendedOPCodesNames[0x5F] = "BIT 3, A";
	m_ExtendedOPCodesNames[0x60] = "BIT 4, B";
	m_ExtendedOPCodesNames[0x61] = "BIT 4, C";
	m_ExtendedOPCodesNames[0x62] = "BIT 4, D";
	m_ExtendedOPCodesNames[0x63] = "BIT 4, E";
	m_ExtendedOPCodesNames[0x64] = "BIT 4, H";
	m_ExtendedOPCodesNames[0x65] = "BIT 4, L";
	m_ExtendedOPCodesNames[0x66] = "BIT 4, (HL)";
	m_ExtendedOPCodesNames[0x67] = "BIT 4, A";
	m_ExtendedOPCodesNames[0x68] = "BIT 5, B";
	m_ExtendedOPCodesNames[0x69] = "BIT 5, C";
	m_ExtendedOPCodesNames[0x6A] = "BIT 5, D";
	m_ExtendedOPCodesNames[0x6B] = "BIT 5, E";
	m_ExtendedOPCodesNames[0x6C] = "BIT 5, H";
	m_ExtendedOPCodesNames[0x6D] = "BIT 5, L";
	m_ExtendedOPCodesNames[0x6E] = "BIT 5, (HL)";
	m_ExtendedOPCodesNames[0x6F] = "BIT 5, A";
	m_ExtendedOPCodesNames[0x70] = "BIT 6, B";
	m_ExtendedOPCodesNames[0x71] = "BIT 6, C";
	m_ExtendedOPCodesNames[0x72] = "BIT 6, D";
	m_ExtendedOPCodesNames[0x73] = "BIT 6, E";
	m_ExtendedOPCodesNames[0x74] = "BIT 6, H";
	m_ExtendedOPCodesNames[0x75] = "BIT 6, L";
	m_ExtendedOPCodesNames[0x76] = "BIT 6, (HL)";
	m_ExtendedOPCodesNames[0x77] = "BIT 6, A";
	m_ExtendedOPCodesNames[0x78] = "BIT 7, B";
	m_ExtendedOPCodesNames[0x79] = "BIT 7, C";
	m_ExtendedOPCodesNames[0x7A] = "BIT 7, D";
	m_ExtendedOPCodesNames[0x7B] = "BIT 7, E";
	m_ExtendedOPCodesNames[0x7C] = "BIT 7, H";
	m_ExtendedOPCodesNames[0x7D] = "BIT 7, L";
	m_ExtendedOPCodesNames[0x7E] = "BIT 7, (HL)";
	m_ExtendedOPCodesNames[0x7F] = "BIT 7, A";
	m_ExtendedOPCodesNames[0x80] = "RES 0, B";
	m_ExtendedOPCodesNames[0x81] = "RES 0, C";
	m_ExtendedOPCodesNames[0x82] = "RES 0, D";
	m_ExtendedOPCodesNames[0x83] = "RES 0, E";
	m_ExtendedOPCodesNames[0x84] = "RES 0, H";
	m_ExtendedOPCodesNames[0x85] = "RES 0, L";
	m_ExtendedOPCodesNames[0x86] = "RES 0, (HL)";
	m_ExtendedOPCodesNames[0x87] = "RES 0, A";
	m_ExtendedOPCodesNames[0x88] = "RES 1, B";
	m_ExtendedOPCodesNames[0x89] = "RES 1, C";
	m_ExtendedOPCodesNames[0x8A] = "RES 1, D";
	m_ExtendedOPCodesNames[0x8B] = "RES 1, E";
	m_ExtendedOPCodesNames[0x8C] = "RES 1, H";
	m_ExtendedOPCodesNames[0x8D] = "RES 1, L";
	m_ExtendedOPCodesNames[0x8E] = "RES 1, (HL)";
	m_ExtendedOPCodesNames[0x8F] = "RES 1, A";
	m_ExtendedOPCodesNames[0x90] = "RES 2, B";
	m_ExtendedOPCodesNames[0x91] = "RES 2, C";
	m_ExtendedOPCodesNames[0x92] = "RES 2, D";
	m_ExtendedOPCodesNames[0x93] = "RES 2, E";
	m_ExtendedOPCodesNames[0x94] = "RES 2, H";
	m_ExtendedOPCodesNames[0x95] = "RES 2, L";
	m_ExtendedOPCodesNames[0x96] = "RES 2, (HL)";
	m_ExtendedOPCodesNames[0x97] = "RES 2, A";
	m_ExtendedOPCodesNames[0x98] = "RES 3, B";
	m_ExtendedOPCodesNames[0x99] = "RES 3, C";
	m_ExtendedOPCodesNames[0x9A] = "RES 3, D";
	m_ExtendedOPCodesNames[0x9B] = "RES 3, E";
	m_ExtendedOPCodesNames[0x9C] = "RES 3, H";
	m_ExtendedOPCodesNames[0x9D] = "RES 3, L";
	m_ExtendedOPCodesNames[0x9E] = "RES 3, (HL)";
	m_ExtendedOPCodesNames[0x9F] = "RES 3, A";
	m_ExtendedOPCodesNames[0xA0] = "RES 4, B";
	m_ExtendedOPCodesNames[0xA1] = "RES 4, C";
	m_ExtendedOPCodesNames[0xA2] = "RES 4, D";
	m_ExtendedOPCodesNames[0xA3] = "RES 4, E";
	m_ExtendedOPCodesNames[0xA4] = "RES 4, H";
	m_ExtendedOPCodesNames[0xA5] = "RES 4, L";
	m_ExtendedOPCodesNames[0xA6] = "RES 4, (HL)";
	m_ExtendedOPCodesNames[0xA7] = "RES 4, A";
	m_ExtendedOPCodesNames[0xA8] = "RES 5, B";
	m_ExtendedOPCodesNames[0xA9] = "RES 5, C";
	m_ExtendedOPCodesNames[0xAA] = "RES 5, D";
	m_ExtendedOPCodesNames[0xAB] = "RES 5, E";
	m_ExtendedOPCodesNames[0xAC] = "RES 5, H";
	m_ExtendedOPCodesNames[0xAD] = "RES 5, L";
	m_ExtendedOPCodesNames[0xAE] = "RES 5, (HL)";
	m_ExtendedOPCodesNames[0xAF] = "RES 5, A";
	m_ExtendedOPCodesNames[0xB0] = "RES 6, B";
	m_ExtendedOPCodesNames[0xB1] = "RES 6, C";
	m_ExtendedOPCodesNames[0xB2] = "RES 6, D";
	m_ExtendedOPCodesNames[0xB3] = "RES 6, E";
	m_ExtendedOPCodesNames[0xB4] = "RES 6, H";
	m_ExtendedOPCodesNames[0xB5] = "RES 6, L";
	m_ExtendedOPCodesNames[0xB6] = "RES 6, (HL)";
	m_ExtendedOPCodesNames[0xB7] = "RES 6, A";
	m_ExtendedOPCodesNames[0xB8] = "RES 7, B";
	m_ExtendedOPCodesNames[0xB9] = "RES 7, C";
	m_ExtendedOPCodesNames[0xBA] = "RES 7, D";
	m_ExtendedOPCodesNames[0xBB] = "RES 7, E";
	m_ExtendedOPCodesNames[0xBC] = "RES 7, H";
	m_ExtendedOPCodesNames[0xBD] = "RES 7, L";
	m_ExtendedOPCodesNames[0xBE] = "RES 7, (HL)";
	m_ExtendedOPCodesNames[0xBF] = "RES 7, A";
	m_ExtendedOPCodesNames[0xC0] = "SET 0, B";
	m_ExtendedOPCodesNames[0xC1] = "SET 0, C";
	m_ExtendedOPCodesNames[0xC2] = "SET 0, D";
	m_ExtendedOPCodesNames[0xC3] = "SET 0, E";
	m_ExtendedOPCodesNames[0xC4] = "SET 0, H";
	m_ExtendedOPCodesNames[0xC5] = "SET 0, L";
	m_ExtendedOPCodesNames[0xC6] = "SET 0, (HL)";
	m_ExtendedOPCodesNames[0xC7] = "SET 0, A";
	m_ExtendedOPCodesNames[0xC8] = "SET 1, B";
	m_ExtendedOPCodesNames[0xC9] = "SET 1, C";
	m_ExtendedOPCodesNames[0xCA] = "SET 1, D";
	m_ExtendedOPCodesNames[0xCB] = "SET 1, E";
	m_ExtendedOPCodesNames[0xCC] = "SET 1, H";
	m_ExtendedOPCodesNames[0xCD] = "SET 1, L";
	m_ExtendedOPCodesNames[0xCE] = "SET 1, (HL)";
	m_ExtendedOPCodesNames[0xCF] = "SET 1, A";
	m_ExtendedOPCodesNames[0xD0] = "SET 2, B";
	m_ExtendedOPCodesNames[0xD1] = "SET 2, C";
	m_ExtendedOPCodesNames[0xD2] = "SET 2, D";
	m_ExtendedOPCodesNames[0xD3] = "SET 2, E";
	m_ExtendedOPCodesNames[0xD4] = "SET 2, H";
	m_ExtendedOPCodesNames[0xD5] = "SET 2, L";
	m_ExtendedOPCodesNames[0xD6] = "SET 2, (HL)";
	m_ExtendedOPCodesNames[0xD7] = "SET 2, A";
	m_ExtendedOPCodesNames[0xD8] = "SET 3, B";
	m_ExtendedOPCodesNames[0xD9] = "SET 3, C";
	m_ExtendedOPCodesNames[0xDA] = "SET 3, D";
	m_ExtendedOPCodesNames[0xDB] = "SET 3, E";
	m_ExtendedOPCodesNames[0xDC] = "SET 3, H";
	m_ExtendedOPCodesNames[0xDD] = "SET 3, L";
	m_ExtendedOPCodesNames[0xDE] = "SET 3, (HL)";
	m_ExtendedOPCodesNames[0xDF] = "SET 3, A";
	m_ExtendedOPCodesNames[0xE0] = "SET 4, B";
	m_ExtendedOPCodesNames[0xE1] = "SET 4, C";
	m_ExtendedOPCodesNames[0xE2] = "SET 4, D";
	m_ExtendedOPCodesNames[0xE3] = "SET 4, E";
	m_ExtendedOPCodesNames[0xE4] = "SET 4, H";
	m_ExtendedOPCodesNames[0xE5] = "SET 4, L";
	m_ExtendedOPCodesNames[0xE6] = "SET 4, (HL)";
	m_ExtendedOPCodesNames[0xE7] = "SET 4, A";
	m_ExtendedOPCodesNames[0xE8] = "SET 5, B";
	m_ExtendedOPCodesNames[0xE9] = "SET 5, C";
	m_ExtendedOPCodesNames[0xEA] = "SET 5, D";
	m_ExtendedOPCodesNames[0xEB] = "SET 5, E";
	m_ExtendedOPCodesNames[0xEC] = "SET 5, H";
	m_ExtendedOPCodesNames[0xED] = "SET 5, L";
	m_ExtendedOPCodesNames[0xEE] = "SET 5, (HL)";
	m_ExtendedOPCodesNames[0xEF] = "SET 5, A";
	m_ExtendedOPCodesNames[0xF0] = "SET 6, B";
	m_ExtendedOPCodesNames[0xF1] = "SET 6, C";
	m_ExtendedOPCodesNames[0xF2] = "SET 6, D";
	m_ExtendedOPCodesNames[0xF3] = "SET 6, E";
	m_ExtendedOPCodesNames[0xF4] = "SET 6, H";
	m_ExtendedOPCodesNames[0xF5] = "SET 6, L";
	m_ExtendedOPCodesNames[0xF6] = "SET 6, (HL)";
	m_ExtendedOPCodesNames[0xF7] = "SET 6, A";
	m_ExtendedOPCodesNames[0xF8] = "SET 7, B";
	m_ExtendedOPCodesNames[0xF9] = "SET 7, C";
	m_ExtendedOPCodesNames[0xFA] = "SET 7, D";
	m_ExtendedOPCodesNames[0xFB] = "SET 7, E";
	m_ExtendedOPCodesNames[0xFC] = "SET 7, H";
	m_ExtendedOPCodesNames[0xFD] = "SET 7, L";
	m_ExtendedOPCodesNames[0xFE] = "SET 7, (HL)";
	m_ExtendedOPCodesNames[0xFF] = "SET 7, A";

	m_ExtendedOPCodesCycles[0x00] = 8;
	m_ExtendedOPCodesCycles[0x01] = 8;
	m_ExtendedOPCodesCycles[0x02] = 8;
	m_ExtendedOPCodesCycles[0x03] = 8;
	m_ExtendedOPCodesCycles[0x04] = 8;
	m_ExtendedOPCodesCycles[0x05] = 8;
	m_ExtendedOPCodesCycles[0x06] = 16;
	m_ExtendedOPCodesCycles[0x07] = 8;
	m_ExtendedOPCodesCycles[0x08] = 8;
	m_ExtendedOPCodesCycles[0x09] = 8;
	m_ExtendedOPCodesCycles[0x0A] = 8;
	m_ExtendedOPCodesCycles[0x0B] = 8;
	m_ExtendedOPCodesCycles[0x0C] = 8;
	m_ExtendedOPCodesCycles[0x0D] = 8;
	m_ExtendedOPCodesCycles[0x0E] = 16;
	m_ExtendedOPCodesCycles[0x0F] = 8;
	m_ExtendedOPCodesCycles[0x10] = 8;
	m_ExtendedOPCodesCycles[0x11] = 8;
	m_ExtendedOPCodesCycles[0x12] = 8;
	m_ExtendedOPCodesCycles[0x13] = 8;
	m_ExtendedOPCodesCycles[0x14] = 8;
	m_ExtendedOPCodesCycles[0x15] = 8;
	m_ExtendedOPCodesCycles[0x16] = 16;
	m_ExtendedOPCodesCycles[0x17] = 8;
	m_ExtendedOPCodesCycles[0x18] = 8;
	m_ExtendedOPCodesCycles[0x19] = 8;
	m_ExtendedOPCodesCycles[0x1A] = 8;
	m_ExtendedOPCodesCycles[0x1B] = 8;
	m_ExtendedOPCodesCycles[0x1C] = 8;
	m_ExtendedOPCodesCycles[0x1D] = 8;
	m_ExtendedOPCodesCycles[0x1E] = 16;
	m_ExtendedOPCodesCycles[0x1F] = 8;
	m_ExtendedOPCodesCycles[0x20] = 8;
	m_ExtendedOPCodesCycles[0x21] = 8;
	m_ExtendedOPCodesCycles[0x22] = 8;
	m_ExtendedOPCodesCycles[0x23] = 8;
	m_ExtendedOPCodesCycles[0x24] = 8;
	m_ExtendedOPCodesCycles[0x25] = 8;
	m_ExtendedOPCodesCycles[0x26] = 16;
	m_ExtendedOPCodesCycles[0x27] = 8;
	m_ExtendedOPCodesCycles[0x28] = 8;
	m_ExtendedOPCodesCycles[0x29] = 8;
	m_ExtendedOPCodesCycles[0x2A] = 8;
	m_ExtendedOPCodesCycles[0x2B] = 8;
	m_ExtendedOPCodesCycles[0x2C] = 8;
	m_ExtendedOPCodesCycles[0x2D] = 8;
	m_ExtendedOPCodesCycles[0x2E] = 16;
	m_ExtendedOPCodesCycles[0x2F] = 8;
	m_ExtendedOPCodesCycles[0x30] = 8;
	m_ExtendedOPCodesCycles[0x31] = 8;
	m_ExtendedOPCodesCycles[0x32] = 8;
	m_ExtendedOPCodesCycles[0x33] = 8;
	m_ExtendedOPCodesCycles[0x34] = 8;
	m_ExtendedOPCodesCycles[0x35] = 8;
	m_ExtendedOPCodesCycles[0x36] = 16;
	m_ExtendedOPCodesCycles[0x37] = 8;
	m_ExtendedOPCodesCycles[0x38] = 8;
	m_ExtendedOPCodesCycles[0x39] = 8;
	m_ExtendedOPCodesCycles[0x3A] = 8;
	m_ExtendedOPCodesCycles[0x3B] = 8;
	m_ExtendedOPCodesCycles[0x3C] = 8;
	m_ExtendedOPCodesCycles[0x3D] = 8;
	m_ExtendedOPCodesCycles[0x3E] = 16;
	m_ExtendedOPCodesCycles[0x3F] = 8;
	m_ExtendedOPCodesCycles[0x40] = 8;
	m_ExtendedOPCodesCycles[0x41] = 8;
	m_ExtendedOPCodesCycles[0x42] = 8;
	m_ExtendedOPCodesCycles[0x43] = 8;
	m_ExtendedOPCodesCycles[0x44] = 8;
	m_ExtendedOPCodesCycles[0x45] = 8;
	m_ExtendedOPCodesCycles[0x46] = 12;
	m_ExtendedOPCodesCycles[0x47] = 8;
	m_ExtendedOPCodesCycles[0x48] = 8;
	m_ExtendedOPCodesCycles[0x49] = 8;
	m_ExtendedOPCodesCycles[0x4A] = 8;
	m_ExtendedOPCodesCycles[0x4B] = 8;
	m_ExtendedOPCodesCycles[0x4C] = 8;
	m_ExtendedOPCodesCycles[0x4D] = 8;
	m_ExtendedOPCodesCycles[0x4E] = 12;
	m_ExtendedOPCodesCycles[0x4F] = 8;
	m_ExtendedOPCodesCycles[0x50] = 8;
	m_ExtendedOPCodesCycles[0x51] = 8;
	m_ExtendedOPCodesCycles[0x52] = 8;
	m_ExtendedOPCodesCycles[0x53] = 8;
	m_ExtendedOPCodesCycles[0x54] = 8;
	m_ExtendedOPCodesCycles[0x55] = 8;
	m_ExtendedOPCodesCycles[0x56] = 12;
	m_ExtendedOPCodesCycles[0x57] = 8;
	m_ExtendedOPCodesCycles[0x58] = 8;
	m_ExtendedOPCodesCycles[0x59] = 8;
	m_ExtendedOPCodesCycles[0x5A] = 8;
	m_ExtendedOPCodesCycles[0x5B] = 8;
	m_ExtendedOPCodesCycles[0x5C] = 8;
	m_ExtendedOPCodesCycles[0x5D] = 8;
	m_ExtendedOPCodesCycles[0x5E] = 12;
	m_ExtendedOPCodesCycles[0x5F] = 8;
	m_ExtendedOPCodesCycles[0x60] = 8;
	m_ExtendedOPCodesCycles[0x61] = 8;
	m_ExtendedOPCodesCycles[0x62] = 8;
	m_ExtendedOPCodesCycles[0x63] = 8;
	m_ExtendedOPCodesCycles[0x64] = 8;
	m_ExtendedOPCodesCycles[0x65] = 8;
	m_ExtendedOPCodesCycles[0x66] = 12;
	m_ExtendedOPCodesCycles[0x67] = 8;
	m_ExtendedOPCodesCycles[0x68] = 8;
	m_ExtendedOPCodesCycles[0x69] = 8;
	m_ExtendedOPCodesCycles[0x6A] = 8;
	m_ExtendedOPCodesCycles[0x6B] = 8;
	m_ExtendedOPCodesCycles[0x6C] = 8;
	m_ExtendedOPCodesCycles[0x6D] = 8;
	m_ExtendedOPCodesCycles[0x6E] = 12;
	m_ExtendedOPCodesCycles[0x6F] = 8;
	m_ExtendedOPCodesCycles[0x70] = 8;
	m_ExtendedOPCodesCycles[0x71] = 8;
	m_ExtendedOPCodesCycles[0x72] = 8;
	m_ExtendedOPCodesCycles[0x73] = 8;
	m_ExtendedOPCodesCycles[0x74] = 8;
	m_ExtendedOPCodesCycles[0x75] = 8;
	m_ExtendedOPCodesCycles[0x76] = 12;
	m_ExtendedOPCodesCycles[0x77] = 8;
	m_ExtendedOPCodesCycles[0x78] = 8;
	m_ExtendedOPCodesCycles[0x79] = 8;
	m_ExtendedOPCodesCycles[0x7A] = 8;
	m_ExtendedOPCodesCycles[0x7B] = 8;
	m_ExtendedOPCodesCycles[0x7C] = 8;
	m_ExtendedOPCodesCycles[0x7D] = 8;
	m_ExtendedOPCodesCycles[0x7E] = 12;
	m_ExtendedOPCodesCycles[0x7F] = 8;
	m_ExtendedOPCodesCycles[0x80] = 8;
	m_ExtendedOPCodesCycles[0x81] = 8;
	m_ExtendedOPCodesCycles[0x82] = 8;
	m_ExtendedOPCodesCycles[0x83] = 8;
	m_ExtendedOPCodesCycles[0x84] = 8;
	m_ExtendedOPCodesCycles[0x85] = 8;
	m_ExtendedOPCodesCycles[0x86] = 16;
	m_ExtendedOPCodesCycles[0x87] = 8;
	m_ExtendedOPCodesCycles[0x88] = 8;
	m_ExtendedOPCodesCycles[0x89] = 8;
	m_ExtendedOPCodesCycles[0x8A] = 8;
	m_ExtendedOPCodesCycles[0x8B] = 8;
	m_ExtendedOPCodesCycles[0x8C] = 8;
	m_ExtendedOPCodesCycles[0x8D] = 8;
	m_ExtendedOPCodesCycles[0x8E] = 16;
	m_ExtendedOPCodesCycles[0x8F] = 8;
	m_ExtendedOPCodesCycles[0x90] = 8;
	m_ExtendedOPCodesCycles[0x91] = 8;
	m_ExtendedOPCodesCycles[0x92] = 8;
	m_ExtendedOPCodesCycles[0x93] = 8;
	m_ExtendedOPCodesCycles[0x94] = 8;
	m_ExtendedOPCodesCycles[0x95] = 8;
	m_ExtendedOPCodesCycles[0x96] = 16;
	m_ExtendedOPCodesCycles[0x97] = 8;
	m_ExtendedOPCodesCycles[0x98] = 8;
	m_ExtendedOPCodesCycles[0x99] = 8;
	m_ExtendedOPCodesCycles[0x9A] = 8;
	m_ExtendedOPCodesCycles[0x9B] = 8;
	m_ExtendedOPCodesCycles[0x9C] = 8;
	m_ExtendedOPCodesCycles[0x9D] = 8;
	m_ExtendedOPCodesCycles[0x9E] = 16;
	m_ExtendedOPCodesCycles[0x9F] = 8;
	m_ExtendedOPCodesCycles[0xA0] = 8;
	m_ExtendedOPCodesCycles[0xA1] = 8;
	m_ExtendedOPCodesCycles[0xA2] = 8;
	m_ExtendedOPCodesCycles[0xA3] = 8;
	m_ExtendedOPCodesCycles[0xA4] = 8;
	m_ExtendedOPCodesCycles[0xA5] = 8;
	m_ExtendedOPCodesCycles[0xA6] = 16;
	m_ExtendedOPCodesCycles[0xA7] = 8;
	m_ExtendedOPCodesCycles[0xA8] = 8;
	m_ExtendedOPCodesCycles[0xA9] = 8;
	m_ExtendedOPCodesCycles[0xAA] = 8;
	m_ExtendedOPCodesCycles[0xAB] = 8;
	m_ExtendedOPCodesCycles[0xAC] = 8;
	m_ExtendedOPCodesCycles[0xAD] = 8;
	m_ExtendedOPCodesCycles[0xAE] = 16;
	m_ExtendedOPCodesCycles[0xAF] = 8;
	m_ExtendedOPCodesCycles[0xB0] = 8;
	m_ExtendedOPCodesCycles[0xB1] = 8;
	m_ExtendedOPCodesCycles[0xB2] = 8;
	m_ExtendedOPCodesCycles[0xB3] = 8;
	m_ExtendedOPCodesCycles[0xB4] = 8;
	m_ExtendedOPCodesCycles[0xB5] = 8;
	m_ExtendedOPCodesCycles[0xB6] = 16;
	m_ExtendedOPCodesCycles[0xB7] = 8;
	m_ExtendedOPCodesCycles[0xB8] = 8;
	m_ExtendedOPCodesCycles[0xB9] = 8;
	m_ExtendedOPCodesCycles[0xBA] = 8;
	m_ExtendedOPCodesCycles[0xBB] = 8;
	m_ExtendedOPCodesCycles[0xBC] = 8;
	m_ExtendedOPCodesCycles[0xBD] = 8;
	m_ExtendedOPCodesCycles[0xBE] = 16;
	m_ExtendedOPCodesCycles[0xBF] = 8;
	m_ExtendedOPCodesCycles[0xC0] = 8;
	m_ExtendedOPCodesCycles[0xC1] = 8;
	m_ExtendedOPCodesCycles[0xC2] = 8;
	m_ExtendedOPCodesCycles[0xC3] = 8;
	m_ExtendedOPCodesCycles[0xC4] = 8;
	m_ExtendedOPCodesCycles[0xC5] = 8;
	m_ExtendedOPCodesCycles[0xC6] = 16;
	m_ExtendedOPCodesCycles[0xC7] = 8;
	m_ExtendedOPCodesCycles[0xC8] = 8;
	m_ExtendedOPCodesCycles[0xC9] = 8;
	m_ExtendedOPCodesCycles[0xCA] = 8;
	m_ExtendedOPCodesCycles[0xCB] = 8;
	m_ExtendedOPCodesCycles[0xCC] = 8;
	m_ExtendedOPCodesCycles[0xCD] = 8;
	m_ExtendedOPCodesCycles[0xCE] = 16;
	m_ExtendedOPCodesCycles[0xCF] = 8;
	m_ExtendedOPCodesCycles[0xD0] = 8;
	m_ExtendedOPCodesCycles[0xD1] = 8;
	m_ExtendedOPCodesCycles[0xD2] = 8;
	m_ExtendedOPCodesCycles[0xD3] = 8;
	m_ExtendedOPCodesCycles[0xD4] = 8;
	m_ExtendedOPCodesCycles[0xD5] = 8;
	m_ExtendedOPCodesCycles[0xD6] = 16;
	m_ExtendedOPCodesCycles[0xD7] = 8;
	m_ExtendedOPCodesCycles[0xD8] = 8;
	m_ExtendedOPCodesCycles[0xD9] = 8;
	m_ExtendedOPCodesCycles[0xDA] = 8;
	m_ExtendedOPCodesCycles[0xDB] = 8;
	m_ExtendedOPCodesCycles[0xDC] = 8;
	m_ExtendedOPCodesCycles[0xDD] = 8;
	m_ExtendedOPCodesCycles[0xDE] = 16;
	m_ExtendedOPCodesCycles[0xDF] = 8;
	m_ExtendedOPCodesCycles[0xE0] = 8;
	m_ExtendedOPCodesCycles[0xE1] = 8;
	m_ExtendedOPCodesCycles[0xE2] = 8;
	m_ExtendedOPCodesCycles[0xE3] = 8;
	m_ExtendedOPCodesCycles[0xE4] = 8;
	m_ExtendedOPCodesCycles[0xE5] = 8;
	m_ExtendedOPCodesCycles[0xE6] = 16;
	m_ExtendedOPCodesCycles[0xE7] = 8;
	m_ExtendedOPCodesCycles[0xE8] = 8;
	m_ExtendedOPCodesCycles[0xE9] = 8;
	m_ExtendedOPCodesCycles[0xEA] = 8;
	m_ExtendedOPCodesCycles[0xEB] = 8;
	m_ExtendedOPCodesCycles[0xEC] = 8;
	m_ExtendedOPCodesCycles[0xED] = 8;
	m_ExtendedOPCodesCycles[0xEE] = 16;
	m_ExtendedOPCodesCycles[0xEF] = 8;
	m_ExtendedOPCodesCycles[0xF0] = 8;
	m_ExtendedOPCodesCycles[0xF1] = 8;
	m_ExtendedOPCodesCycles[0xF2] = 8;
	m_ExtendedOPCodesCycles[0xF3] = 8;
	m_ExtendedOPCodesCycles[0xF4] = 8;
	m_ExtendedOPCodesCycles[0xF5] = 8;
	m_ExtendedOPCodesCycles[0xF6] = 16;
	m_ExtendedOPCodesCycles[0xF7] = 8;
	m_ExtendedOPCodesCycles[0xF8] = 8;
	m_ExtendedOPCodesCycles[0xF9] = 8;
	m_ExtendedOPCodesCycles[0xFA] = 8;
	m_ExtendedOPCodesCycles[0xFB] = 8;
	m_ExtendedOPCodesCycles[0xFC] = 8;
	m_ExtendedOPCodesCycles[0xFD] = 8;
	m_ExtendedOPCodesCycles[0xFE] = 16;
	m_ExtendedOPCodesCycles[0xFF] = 8;
}

byte CPU::readNextByte()
{
	byte nextByte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextByte;
}

word CPU::readNextWord()
{
	byte lowByte = readNextByte();
	byte highByte = readNextByte();
	return static_cast<word>(highByte << 8 | lowByte);
}

sbyte CPU::readNextSignedByte()
{
	sbyte nextSbyte = m_MMU.Read(PC.GetValue());
	PC.Increment();
	return nextSbyte;
}

bool CPU::checkJumpCondition(JumpConditions i_Condition)
{
	switch (i_Condition)
	{
	case CPU::JumpConditions::NZ:
		if (!F.GetZ())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::Z:
		if (F.GetZ())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::NC:
		if (!F.GetC())
		{
			return true;
		}
		break;
	case CPU::JumpConditions::C:
		if (F.GetC())
		{
			return true;
		}
		break;
	}
	return false;
}

/* OPCodes */

/*
	Operation:
	LD nn, n

	Description:
	Put value n (byte from memory) into nn (register).
*/
inline void CPU::LD_nn_n(ByteRegister& i_DestRegister)
{
	byte n = readNextByte();
	i_DestRegister.SetValue(n);
}

/*
	Operation:
	LD n, nn

	Description:
	Put value nn (word from memory) into n (16 bit register).
*/
inline void CPU::LD_n_nn(WordRegister& i_DestRegister)
{
	word nn = readNextWord();
	i_DestRegister.SetValue(nn);
}

inline void CPU::LD_n_nn(Pair8BRegisters& i_DestRegister)
{
	word nn = readNextWord();
	i_DestRegister.SetValue(nn);
}

/*
	Operation:
	LD r1, r2

	Description:
	Put value r2 into r1.

*/
inline void CPU::LD_r1_r2(ByteRegister& i_DestRegister, const ByteRegister& i_SrcRegister)
{
	byte val = i_SrcRegister.GetValue();
	i_DestRegister.SetValue(val);
}

inline void CPU::LD_r1_r2(ByteRegister& i_DestRegister, word i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	i_DestRegister.SetValue(val);
}

inline void CPU::LD_r1_r2(word i_DestMemory, const ByteRegister& i_SrcRegister)
{
	byte val = i_SrcRegister.GetValue();
	m_MMU.Write(i_DestMemory, val);
}

inline void CPU::LD_r1_r2(word i_DestMemory, byte i_Value)
{
	m_MMU.Write(i_DestMemory, i_Value);
}

inline void CPU::LD_r1_r2(WordRegister& i_DestRegister, const Pair8BRegisters& i_SrcRegister)
{
	word val = i_SrcRegister.GetValue();
	i_DestRegister.SetValue(val);
}

inline void CPU::LD_r1_r2(word i_DestMemory, const WordRegister& i_SrcRegister)
{
	byte lowVal = i_SrcRegister.GetLowByte();
	byte highVal = i_SrcRegister.GetHighByte();
	m_MMU.Write(i_DestMemory, lowVal);
	m_MMU.Write(i_DestMemory + 1, highVal);
}

/*
	Operation:
	LDHL SP,n

	Description:
	Put SP + n effective address into HL
	n = one byte signed immediate value
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
inline void CPU::LD_HL_SP_n()
{
	sbyte n = readNextSignedByte();
	word spVal = SP.GetValue();
	word res = spVal + n;

	F.SetZ(false);
	F.SetN(false);
	(((spVal ^ n ^ res) & 0x10) == 0x10) ? F.SetH(true) : F.SetH(false);
	(((spVal ^ n ^ res) & 0x100) == 0x100) ? F.SetC(true) : F.SetC(false);

	HL.SetValue(res);
}

/*
	Operation:
	PUSH nn

	Description:
	Push nn onto stack
	Decrement Stack Pointer (SP) twice
*/
inline void CPU::PUSH(word i_Value)
{
	SP.Decrement();
	word addr = SP.GetValue();
	byte highVal = static_cast<byte>(i_Value >> 8);
	m_MMU.Write(addr, highVal);

	SP.Decrement();
	addr = SP.GetValue();
	byte lowVal = static_cast<byte>(i_Value);
	m_MMU.Write(addr, lowVal);
}

/*
	Operation:
	POP nn

	Description:
	Pop two bytes off stack into nn
	Increment Stack Pointer (SP) twice
*/
inline void CPU::POP(word& i_Value)
{
	word addr = SP.GetValue();
	byte lowVal = m_MMU.Read(addr);
	SP.Increment();

	addr = SP.GetValue();
	byte highVal = m_MMU.Read(addr);
	SP.Increment();

	i_Value = static_cast<word>(highVal << 8 | lowVal);
}

/*
	Operation:
	ADD A, n

	Description:
	Add n to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
inline void CPU::ADD(byte i_Value)
{
	byte aVal = A.GetValue();
	int res = aVal + i_Value;
	A.SetValue(static_cast<byte>(res));

	static_cast<byte>(res) == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	((aVal ^ i_Value ^ res) & 0x10) != 0 ? F.SetH(true) : F.SetH(false);
	((aVal ^ i_Value ^ res) & 0x100) != 0 ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	ADD HL, n

	Description:
	Add n to HL
	Z - Not affected
	N - Reset
	H - Set if carry from bit 11
	C - Set if carry from bit 15
*/
inline void CPU::ADD_HL(word i_Value)
{
	word hlVal = HL.GetValue();
	HL.SetValue(hlVal + i_Value);

	F.SetN(false);
	(hlVal & 0x0FFF) + (i_Value & 0x0FFF) > 0x0FFF ? F.SetH(true) : F.SetH(false);
	(hlVal + i_Value) > 0xFFFF ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	ADD SP, n

	Description:
	Add n to SP
	n = one byte signed immediate value (#).
	Z - Reset
	N - Reset
	H - Set or reset according to operation
	C - Set or reset according to operation
*/
inline void CPU::ADD_SP()
{
	sbyte val = readNextSignedByte();
	word spVal = SP.GetValue();
	int res = spVal + val;

	F.SetZ(false);
	F.SetN(false);
	((SP.GetValue() ^ val ^ (res & 0xFFFF)) & 0x10) == 0x10 ? F.SetH(true) : F.SetH(false);
	((SP.GetValue() ^ val ^ (res & 0xFFFF)) & 0x100) == 0x100 ? F.SetC(true) : F.SetC(false);
	SP.SetValue(static_cast<word>(res));
}

/*
	Operation:
	ADC A, n

	Description:
	Add n + Carry flag to A
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Set if carry from bit 7
*/
inline void CPU::ADC(byte i_Value)
{
	byte aVal = A.GetValue();
	byte carry = F.GetC();
	int res = aVal + i_Value + carry;
	A.SetValue(static_cast<byte>(res));

	static_cast<byte>(res) == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	((aVal & 0xF) + (i_Value & 0xF) + carry) > 0xF ? F.SetH(true) : F.SetH(false);
	res > 0xFF ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	SUB n

	Description:
	Subtract n to A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
inline void CPU::SUB(byte i_Value)
{
	byte aVal = A.GetValue();
	int res = aVal - i_Value;
	A.SetValue(static_cast<byte>(res));

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	(aVal ^ i_Value ^ res) & 0x10 ? F.SetH(true) : F.SetH(false);
	(aVal ^ i_Value ^ res) & 0x100 ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	SBC A, n

	Description:
	Subtract n + Carry flag from A
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
inline void CPU::SBC(byte i_Value)
{
	byte aVal = A.GetValue();
	byte carry = F.GetC();
	int res = aVal - carry - i_Value;
	A.SetValue(static_cast<byte>(res));

	static_cast<byte>(res) == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	((aVal & 0xF) - (i_Value & 0xF) - carry) < 0x0 ? F.SetH(true) : F.SetH(false);
	res < 0 ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	AND n

	Description:
	Logically AND n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Set
	C - Reset
*/
inline void CPU::AND(byte i_Value)
{
	byte aVal = A.GetValue();
	byte res = aVal & i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(true);
	F.SetC(false);
}

/*
	Operation:
	OR n

	Description:
	Logical OR n with A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
inline void CPU::OR(byte i_Value)
{
	byte aVal = A.GetValue();
	byte res = aVal | i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);
}

/*
	Operation:
	XOR n

	Description:
	Logical exclusive OR n with register A, result in A
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
inline void CPU::XOR(byte i_Value)
{
	byte aVal = A.GetValue();
	byte res = aVal ^ i_Value;
	A.SetValue(res);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);
}

/*
	Operation:
	CP n

	Description:
	Compare A with n. This is basically an A - n
	subtraction instruction but the results are thrown away
	Z - Set if result is zero
	N - Set
	H - Set if no borrow from bit 4
	C - Set if no borrow
*/
inline void CPU::CP(byte i_Value)
{
	byte aVal = A.GetValue();

	aVal == i_Value ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	(((aVal - i_Value) & 0xF) > (aVal & 0xF)) ? F.SetH(true) : F.SetH(false);
	aVal < i_Value ? F.SetC(true) : F.SetC(false);
}

/*
	Operation:
	INC n

	Description:
	Increment register n
	Z - Set if result is zero
	N - Reset
	H - Set if carry from bit 3
	C - Not affected
*/
inline void CPU::INC(ByteRegister& i_DestRegister)
{
	i_DestRegister.Increment();
	byte regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	(regVal & 0xF) == 0x0 ? F.SetH(true) : F.SetH(false);
}

/*
	Operation:
	INC nn

	Description:
	Increment register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
inline void CPU::INC_no_flags(WordRegister& i_DestRegister)
{
	i_DestRegister.Increment();
}

inline void CPU::INC_no_flags(Pair8BRegisters& i_DestRegister)
{
	i_DestRegister.Increment();
}

/*
	Operation:
	DEC n

	Description:
	Decrement register n
	Z - Set if result is zero
	N - Set
	H - Set if carry from bit 3
	C - Not affected
*/
inline void CPU::DEC(ByteRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
	byte regVal = i_DestRegister.GetValue();

	regVal == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	(regVal & 0xF) == 0xF ? F.SetH(true) : F.SetH(false);
}

/*
	Operation:
	DEC nn

	Description:
	Decrement register nn
	nn = BC, DE, HL, SP
	No flags affected
*/
inline void CPU::DEC_no_flags(WordRegister& i_DestRegister)
{
	i_DestRegister.Decrement();
}

inline void CPU::DEC_no_flags(Pair8BRegisters& i_DestRegister)
{
	i_DestRegister.Decrement();
}

/*
	Operation:
	DAA

	Description:
	Decimal adjust register A.
	This instruction adjusts register A so that the
	correct representation of Binary Coded Decimal (BCD)
	is obtained.
	This is done AFTER ADD/ADC/SUB/SBC.

	Z - Set if register A is zero.
	N - Not affected.
	H - Reset.
	C - Set or reset according to operation.
*/
inline void CPU::DAA()
{
	int res = A.GetValue();
	
	if (F.GetN()) // POST SUB / SBC
	{
		if (F.GetH())
		{
			res -= 0x6;
			res &= 0xFF;
		}
		if (F.GetC())
		{
			res -= 0x60;
		}
	}
	else // POST ADD / ADC
	{
		if (F.GetH() || (res & 0xF) > 9)
		{
			res += 0x06;
		}

		if (F.GetC() || (res > 0x9F))
		{
			res += 0x60;
		}
	}
	
	static_cast<byte>(res) == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetH(false);
	if ((res & 0x100) == 0x100)	F.SetC(true);

	A.SetValue(static_cast<byte>(res));
}

/*
	Operation:
	CPL

	Description:
	Complement A register. (Flip all bits.)
	Z - Not affected
	N - Set
	H - Set
	C - Not affected
*/
inline void CPU::CPL()
{
	byte aVal = A.GetValue();
	aVal = ~aVal;
	A.SetValue(aVal);

	F.SetN(true);
	F.SetH(true);
}

/*
	Operation:
	CCF

	Description:
	Complement carry flag.
	If C flag is set, then reset it.
	If C flag is reset, then set it.
	Z - Not affected
	N - Reset
	H - Reset
	C - Complemented
*/
inline void CPU::CCF()
{
	F.SetC(!F.GetC());

	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SCF

	Description:
	Set Carry flag
	Z - Not affected
	N - Reset
	H - Reset
	C - Set
*/
inline void CPU::SCF()
{
	F.SetC(true);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	NOP

	Description:
	No operation
*/
inline void CPU::_NOP()
{
	// Do nothing
}

/*
	Operation:
	HALT

	Description:
	Power down CPU until an interrupt occurs. Use this
	when ever possible to reduce energy consumption.
*/
inline void CPU::HALT()
{
	m_HALT = true;
}

/*
	Operation:
	NOP

	Description:
	Halt CPU & LCD display until button pressed.
*/
inline void CPU::STOP()
{
	// do nothing
}

/*
	Operation:
	DI

	Description:
	This instruction disables interrupts
*/
inline void CPU::DI()
{
	m_IME = false;
}

/*
	Operation:
	EI

	Description:
	This intruction enables interrupts
*/
inline void CPU::EI()
{
	m_IME = true;
}

/*
	Operation:
	JP nn

	Description:
	Jump to address nn
	nn = two byte immediate value. (LS byte first.)
*/
inline void CPU::JP_nn()
{
	word addr = readNextWord();
	PC.SetValue(addr);
}

/*
	Operation:
	JP cc, nn

	Description:
	Jump to address n if following condition is true:
	cc = NZ, Jump if Z flag is reset
	cc = Z, Jump if Z flag is set
	cc = NC, Jump if C flag is reset
	cc = C, Jump if C flag is set
*/
inline void CPU::JP_cc_nn(JumpConditions i_Condition)
{
	if (checkJumpCondition(i_Condition))
	{
		JP_nn();
		m_IsConditionalJumpTaken = true;
	}
	else
	{
		// consume next word
		PC.Increment();
		PC.Increment();
		m_IsConditionalJumpTaken = false;
	}
}

/*
	Operation:
	JP (HL)

	Description:
	Jump to address contained in HL
*/
inline void CPU::JP_hl()
{
	word addr = HL.GetValue();
	PC.SetValue(addr);
}

/*
	Operation:
	JR n

	Description:
	Add n to current address and jump to it
*/
inline void CPU::JR_n()
{
	sbyte nextSb = readNextSignedByte();
	word newPc = static_cast<word>(PC.GetValue() + nextSb);
	PC.SetValue(newPc);
}

/*
	Operation:
	JR cc, n

	Description:
	If following condition is true then add n to current
	address and jump to it:
	n = one byte signed immediate value
	cc = NZ, Jump if Z flag is reset
	cc = Z, Jump if Z flag is set
	cc = NC, Jump if C flag is reset
	cc = C, Jump if C flag is set
*/
inline void CPU::JR_cc_n(JumpConditions i_Condition)
{
	if (checkJumpCondition(i_Condition))
	{
		JR_n();
		m_IsConditionalJumpTaken = true;
	}
	else
	{
		PC.Increment();
		m_IsConditionalJumpTaken = false;
	}
}

/*
	Operation:
	CALL nn

	Description:
	Push address of next instruction onto stack and then jump to address nn
*/
inline void CPU::CALL_nn()
{
	word addr = readNextWord(); // addr = nn, pc now is post instruction CALL nn
	word pcVal = PC.GetValue();
	PUSH(pcVal); // store PC in the stack
	PC.SetValue(addr); // jump to nn
}

/*
	Operation:
	CALL cc nn

	Description:
	Call address nn if following condition is true:
	cc = NZ, Call if Z flag is reset
	cc = Z, Call if Z flag is set
	cc = NC, Call if C flag is reset
	cc = C, Call if C flag is set
*/
inline void CPU::CALL_cc_nn(JumpConditions i_Condition)
{
	if (checkJumpCondition(i_Condition))
	{
		CALL_nn();
		m_IsConditionalJumpTaken = true;
	}
	else
	{
		PC.Increment();
		PC.Increment();
		m_IsConditionalJumpTaken = false;
	}
}

/*
	Operation:
	RST n

	Description:
	Push present address onto stack
	Jump to address $0000 + n
*/
inline void CPU::RST_n(word i_Value)
{
	word pcVal = PC.GetValue();
	PUSH(pcVal);
	PC.SetValue(i_Value);
}

/*
	Operation:
	RET

	Description:
	Pop two bytes from stack & jump to that address
*/
inline void CPU::RET()
{
	word val = 0x0;
	POP(val);
	PC.SetValue(val);
}

/*
	Operation:
	RET cc

	Description:
	Return if following condition is true:
	cc = NZ, Return if Z flag is reset
	cc = Z, Return if Z flag is set
	cc = NC, Return if C flag is reset
	cc = C, Return if C flag is set
*/
inline void CPU::RET_cc(JumpConditions i_Condition)
{
	if (checkJumpCondition(i_Condition))
	{
		RET();
		m_IsConditionalJumpTaken = true;
	}
	else
	{
		m_IsConditionalJumpTaken = false;
	}
}

/*
	Operation:
	RETI

	Description:
	Pop two bytes from stack & jump to that address
	and enable interrupts 
	this is being called after an interrupt routine
*/
inline void CPU::RETI()
{
	RET();
	m_IME = true;
}

/*
	Operation:
	SWAP n

	Description:
	Swap upper & lower nibles of n
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Reset
*/
inline void CPU::SWAP(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	byte res = (val >> 4) | (val << 4);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);

	i_DestRegister.SetValue(res);
}

inline void CPU::SWAP(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	byte res = (val >> 4) | (val << 4);

	res == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
	F.SetC(false);

	m_MMU.Write(i_SrcMemory, res);
}

/*
	Operation:
	RLC n

	Description:
	Rotate n left. Old bit 7 to Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
inline void CPU::RLC_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	i_DestRegister.GetBit(7) ? F.SetC(true) : F.SetC(false);
	
	val = (val << 1) | F.GetC(); // rotate left and add bit 7 to bit 0
	i_DestRegister.SetValue(val);
	
	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::RLC_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	bitwise::IsBitSet(7, val) ? F.SetC(true) : F.SetC(false);

	val = (val << 1) | F.GetC(); // rotate left and add bit 7 to bit 0
	m_MMU.Write(i_SrcMemory, val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RL n

	Description:
	Rotate n left through Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
inline void CPU::RL_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	// save bit 7 from n
	bool bit7 = i_DestRegister.GetBit(7);

	val = (val << 1) | F.GetC(); // rotate left and add carry to bit 0
	i_DestRegister.SetValue(val);

	// set old bit 7 into carry flag
	bit7 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::RL_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	// save bit 7 from n
	bool bit7 = bitwise::IsBitSet(7, val);

	val = (val << 1) | F.GetC(); // rotate left and add carry to bit 0
	m_MMU.Write(i_SrcMemory, val);

	// set old bit 7 into carry flag
	bit7 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RRC n

	Description:
	Rotate n right. Old bit 0 to Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
inline void CPU::RRC_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	i_DestRegister.GetBit(0) ? F.SetC(true) : F.SetC(false);

	val = (val >> 1) | (F.GetC() << 7); // rotate right and add bit 0 to bit 7
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::RRC_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	bitwise::IsBitSet(0, val) ? F.SetC(true) : F.SetC(false);

	val = (val >> 1) | (F.GetC() << 7); // rotate right and add bit 0 to bit 7
	m_MMU.Write(i_SrcMemory, val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	RR n

	Description:
	Rotate n right through Carry flag
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
inline void CPU::RR_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	// save bit 0 from n
	bool bit0 = i_DestRegister.GetBit(0);

	val = (val >> 1) | (F.GetC() << 7); // rotate left and add carry to bit 7
	i_DestRegister.SetValue(val);

	// set old bit 0 into carry flag
	bit0 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::RR_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	// save bit 0 from n
	bool bit0 = bitwise::IsBitSet(0, val);

	val = (val >> 1) | (F.GetC() << 7); // rotate left and add carry to bit 7
	m_MMU.Write(i_SrcMemory, val);

	// set old bit 0 into carry flag
	bit0 ? F.SetC(true) : F.SetC(false);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SLA n

	Description:
	Shift n left into Carry. LSB of n set to 0.
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 7 data
*/
inline void CPU::SLA_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();

	F.SetC(i_DestRegister.GetBit(7)); // set carry to be bit 7
	val = val << 1; // shift n left (LSB will be 0)
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::SLA_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);

	F.SetC(bitwise::IsBitSet(7, val)); // set carry to be bit 7
	val = val << 1; // shift n left (LSB will be 0)
	m_MMU.Write(i_SrcMemory, val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SRA n

	Description:
	Shift n right into Carry. MSB doesn't change.
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
inline void CPU::SRA_n(ByteRegister& i_DestRegister)
{
	byte val = i_DestRegister.GetValue();
	bool bit7 = i_DestRegister.GetBit(7);

	F.SetC(i_DestRegister.GetBit(0)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	if (bit7) // if MSB was set, restore it
	{
		val |= (1 << 7);
	}
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::SRA_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	bool bit7 = bitwise::IsBitSet(7, val);

	F.SetC(bitwise::IsBitSet(0, val)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	if (bit7) // if MSB was set, restore it
	{
		val |= (1 << 7);
	}
	m_MMU.Write(i_SrcMemory, val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	SRL n

	Description:
	Shift n right into Carry. MSB set to 0
	Z - Set if result is zero
	N - Reset
	H - Reset
	C - Contains old bit 0 data
*/
inline void CPU::SRL_n(ByteRegister& i_DestRegister)
{
	byte val = static_cast<byte>(i_DestRegister.GetValue());
	F.SetC(i_DestRegister.GetBit(0)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	i_DestRegister.SetValue(val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

inline void CPU::SRL_n(word& i_SrcMemory)
{
	byte val = m_MMU.Read(i_SrcMemory);
	F.SetC(bitwise::IsBitSet(0, val)); // set carry to be bit 0
	val = val >> 1; // shift n right, MSB is 0 now
	m_MMU.Write(i_SrcMemory, val);

	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	F.SetH(false);
}

/*
	Operation:
	BIT b, r

	Description:
	Copies the complement of the contents of the specified bit in register r to the Z flag
	Z - Complement of bit b in r
	N - Reset
	H - Set
	C - Not affected
*/
inline void CPU::BIT_b_r(byte i_BitNumber, byte i_Value)
{
	bool res = !bitwise::IsBitSet(i_BitNumber, i_Value);
	
	F.SetZ(res);
	F.SetN(false);
	F.SetH(true);
}

/*
	Operation:
	SET b, r

	Description:
	Sets the specified bit in register r
*/
inline void CPU::SET_b_r(byte i_BitNumber, byte& o_Value)
{
	bitwise::SetBit(i_BitNumber, o_Value);
}

/*
	Operation:
	RES b, r

	Description:
	Resets the specified bit in register r
*/
inline void CPU::RES_b_r(byte i_BitNumber, byte& o_Value)
{
	bitwise::ClearBit(i_BitNumber, o_Value);
}

void CPU::OPCode_06()
{
	LD_nn_n(B);
}

void CPU::OPCode_0E()
{
	LD_nn_n(C);
}

void CPU::OPCode_16()
{
	LD_nn_n(D);
}

void CPU::OPCode_1E()
{
	LD_nn_n(E);
}

void CPU::OPCode_26()
{
	LD_nn_n(H);
}

void CPU::OPCode_2E()
{
	LD_nn_n(L);
}

void CPU::OPCode_7F()
{
	LD_r1_r2(A, A);
}

void CPU::OPCode_78()
{
	LD_r1_r2(A, B);
}

void CPU::OPCode_79()
{
	LD_r1_r2(A, C);
}

void CPU::OPCode_7A()
{
	LD_r1_r2(A, D);
}

void CPU::OPCode_7B()
{
	LD_r1_r2(A, E);
}

void CPU::OPCode_7C()
{
	LD_r1_r2(A, H);
}

void CPU::OPCode_7D()
{
	LD_r1_r2(A, L);
}

void CPU::OPCode_7E()
{
	word addr = HL.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_40()
{
	LD_r1_r2(B, B);
}

void CPU::OPCode_41()
{
	LD_r1_r2(B, C);
}

void CPU::OPCode_42()
{
	LD_r1_r2(B, D);
}

void CPU::OPCode_43()
{
	LD_r1_r2(B, E);
}

void CPU::OPCode_44()
{
	LD_r1_r2(B, H);
}

void CPU::OPCode_45()
{
	LD_r1_r2(B, L);
}

void CPU::OPCode_46()
{
	word addr = HL.GetValue();
	LD_r1_r2(B, addr);
}

void CPU::OPCode_48()
{
	LD_r1_r2(C, B);
}

void CPU::OPCode_49()
{
	LD_r1_r2(C, C);
}

void CPU::OPCode_4A()
{
	LD_r1_r2(C, D);
}

void CPU::OPCode_4B()
{
	LD_r1_r2(C, E);
}

void CPU::OPCode_4C()
{
	LD_r1_r2(C, H);
}

void CPU::OPCode_4D()
{
	LD_r1_r2(C, L);
}

void CPU::OPCode_4E()
{
	word addr = HL.GetValue();
	LD_r1_r2(C, addr);
}

void CPU::OPCode_50()
{
	LD_r1_r2(D, B);
}

void CPU::OPCode_51()
{
	LD_r1_r2(D, C);
}

void CPU::OPCode_52()
{
	LD_r1_r2(D, D);
}

void CPU::OPCode_53()
{
	LD_r1_r2(D, E);
}

void CPU::OPCode_54()
{
	LD_r1_r2(D, H);
}

void CPU::OPCode_55()
{
	LD_r1_r2(D, L);
}

void CPU::OPCode_56()
{
	word addr = HL.GetValue();
	LD_r1_r2(D, addr);
}

void CPU::OPCode_58()
{
	LD_r1_r2(E, B);
}

void CPU::OPCode_59()
{
	LD_r1_r2(E, C);
}

void CPU::OPCode_5A()
{
	LD_r1_r2(E, D);
}

void CPU::OPCode_5B()
{
	LD_r1_r2(E, E);
}

void CPU::OPCode_5C()
{
	LD_r1_r2(E, H);
}

void CPU::OPCode_5D()
{
	LD_r1_r2(E, L);
}

void CPU::OPCode_5E()
{
	word addr = HL.GetValue();
	LD_r1_r2(E, addr);
}

void CPU::OPCode_60()
{
	LD_r1_r2(H, B);
}

void CPU::OPCode_61()
{
	LD_r1_r2(H, C);
}

void CPU::OPCode_62()
{
	LD_r1_r2(H, D);
}

void CPU::OPCode_63()
{
	LD_r1_r2(H, E);
}

void CPU::OPCode_64()
{
	LD_r1_r2(H, H);
}

void CPU::OPCode_65()
{
	LD_r1_r2(H, L);
}

void CPU::OPCode_66()
{
	word addr = HL.GetValue();
	LD_r1_r2(H, addr);
}

void CPU::OPCode_68()
{
	LD_r1_r2(L, B);
}

void CPU::OPCode_69()
{
	LD_r1_r2(L, C);
}

void CPU::OPCode_6A()
{
	LD_r1_r2(L, D);
}

void CPU::OPCode_6B()
{
	LD_r1_r2(L, E);
}

void CPU::OPCode_6C()
{
	LD_r1_r2(L, H);
}

void CPU::OPCode_6D()
{
	LD_r1_r2(L, L);
}

void CPU::OPCode_6E()
{
	word addr = HL.GetValue();
	LD_r1_r2(L, addr);
}

void CPU::OPCode_70()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, B);
}

void CPU::OPCode_71()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, C);
}

void CPU::OPCode_72()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, D);
}

void CPU::OPCode_73()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, E);
}

void CPU::OPCode_74()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, H);
}

void CPU::OPCode_75()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, L);
}

void CPU::OPCode_36()
{
	byte n = readNextByte();
	word addr = HL.GetValue();
	LD_r1_r2(addr, n);
}

void CPU::OPCode_0A()
{
	word addr = BC.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_1A()
{
	word addr = DE.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_FA()
{
	word addr = readNextWord();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_3E()
{
	LD_nn_n(A);
}

void CPU::OPCode_47()
{
	LD_r1_r2(B, A);
}

void CPU::OPCode_4F()
{
	LD_r1_r2(C, A);
}

void CPU::OPCode_57()
{
	LD_r1_r2(D, A);
}

void CPU::OPCode_5F()
{
	LD_r1_r2(E, A);
}

void CPU::OPCode_67()
{
	LD_r1_r2(H, A);
}

void CPU::OPCode_6F()
{
	LD_r1_r2(L, A);
}

void CPU::OPCode_02()
{
	word addr = BC.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_12()
{
	word addr = DE.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_77()
{
	word addr = HL.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_EA()
{
	word addr = readNextWord();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(A, addr);
}

void CPU::OPCode_E2()
{
	word addr = 0xFF00 + C.GetValue();
	LD_r1_r2(addr, A);
}

void CPU::OPCode_3A()
{
	OPCode_7E();
	HL.Decrement();
}

void CPU::OPCode_32()
{
	OPCode_77();
	HL.Decrement();
}

void CPU::OPCode_2A()
{
	OPCode_7E();
	HL.Increment();
}

void CPU::OPCode_22()
{
	OPCode_77();
	HL.Increment();
}

void CPU::OPCode_E0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(addr, A);
}

void CPU::OPCode_F0()
{
	byte val = readNextByte();
	word addr = 0xFF00 + val;
	LD_r1_r2(A, addr);
}

void CPU::OPCode_01()
{
	LD_n_nn(BC);
}

void CPU::OPCode_11()
{
	LD_n_nn(DE);
}

void CPU::OPCode_21()
{
	LD_n_nn(HL);
}

void CPU::OPCode_31()
{
	LD_n_nn(SP);
}

void CPU::OPCode_F9()
{
	LD_r1_r2(SP, HL);
}

void CPU::OPCode_F8()
{
	LD_HL_SP_n();
}

void CPU::OPCode_08()
{
	word addr = readNextWord();
	LD_r1_r2(addr, SP);
}

void CPU::OPCode_F5()
{
	word val = AF.GetValue();
	PUSH(val);
}

void CPU::OPCode_C5()
{
	word val = BC.GetValue();
	PUSH(val);
}

void CPU::OPCode_D5()
{
	word val = DE.GetValue();
	PUSH(val);
}

void CPU::OPCode_E5()
{
	word val = HL.GetValue();
	PUSH(val);
}

void CPU::OPCode_F1()
{
	word val = 0x0;
	POP(val);
	AF.SetValue(val);
	byte maskedFVal = AF.GetLowRegister().GetValue();
	maskedFVal &= 0xF0;
	AF.GetLowRegister().SetValue(maskedFVal);
}

void CPU::OPCode_C1()
{
	word val = 0x0;
	POP(val);
	BC.SetValue(val);
}

void CPU::OPCode_D1()
{
	word val = 0x0;
	POP(val);
	DE.SetValue(val);
}

void CPU::OPCode_E1()
{
	word val = 0x0;
	POP(val);
	HL.SetValue(val);
}

void CPU::OPCode_87()
{
	byte val = static_cast<byte>(A.GetValue());
	ADD(val);
}

void CPU::OPCode_80()
{
	byte val = static_cast<byte>(B.GetValue());
	ADD(val);
}

void CPU::OPCode_81()
{
	byte val = static_cast<byte>(C.GetValue());
	ADD(val);
}

void CPU::OPCode_82()
{
	byte val = static_cast<byte>(D.GetValue());
	ADD(val);
}

void CPU::OPCode_83()
{
	byte val = static_cast<byte>(E.GetValue());
	ADD(val);
}

void CPU::OPCode_84()
{
	byte val = static_cast<byte>(H.GetValue());
	ADD(val);
}

void CPU::OPCode_85()
{
	byte val = static_cast<byte>(L.GetValue());
	ADD(val);
}

void CPU::OPCode_86()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADD(val);
}

void CPU::OPCode_C6()
{
	byte val = readNextByte();
	ADD(val);
}

void CPU::OPCode_8F()
{
	byte val = static_cast<byte>(A.GetValue());
	ADC(val);
}

void CPU::OPCode_88()
{
	byte val = static_cast<byte>(B.GetValue());
	ADC(val);
}

void CPU::OPCode_89()
{
	byte val = static_cast<byte>(C.GetValue());
	ADC(val);
}

void CPU::OPCode_8A()
{
	byte val = static_cast<byte>(D.GetValue());
	ADC(val);
}

void CPU::OPCode_8B()
{
	byte val = static_cast<byte>(E.GetValue());
	ADC(val);
}

void CPU::OPCode_8C()
{
	byte val = static_cast<byte>(H.GetValue());
	ADC(val);
}

void CPU::OPCode_8D()
{
	byte val = static_cast<byte>(L.GetValue());
	ADC(val);
}

void CPU::OPCode_8E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	ADC(val);
}

void CPU::OPCode_CE()
{
	byte val = readNextByte();
	ADC(val);
}

void CPU::OPCode_97()
{
	byte val = static_cast<byte>(A.GetValue());
	SUB(val);
}

void CPU::OPCode_90()
{
	byte val = static_cast<byte>(B.GetValue());
	SUB(val);
}

void CPU::OPCode_91()
{
	byte val = static_cast<byte>(C.GetValue());
	SUB(val);
}

void CPU::OPCode_92()
{
	byte val = static_cast<byte>(D.GetValue());
	SUB(val);
}

void CPU::OPCode_93()
{
	byte val = static_cast<byte>(E.GetValue());
	SUB(val);
}

void CPU::OPCode_94()
{
	byte val = static_cast<byte>(H.GetValue());
	SUB(val);
}

void CPU::OPCode_95()
{
	byte val = static_cast<byte>(L.GetValue());
	SUB(val);
}

void CPU::OPCode_96()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SUB(val);
}

void CPU::OPCode_D6()
{
	byte val = readNextByte();
	SUB(val);
}

void CPU::OPCode_9F()
{
	byte val = static_cast<byte>(A.GetValue());
	SBC(val);
}

void CPU::OPCode_98()
{
	byte val = static_cast<byte>(B.GetValue());
	SBC(val);
}

void CPU::OPCode_99()
{
	byte val = static_cast<byte>(C.GetValue());
	SBC(val);
}

void CPU::OPCode_9A()
{
	byte val = static_cast<byte>(D.GetValue());
	SBC(val);
}

void CPU::OPCode_9B()
{
	byte val = static_cast<byte>(E.GetValue());
	SBC(val);
}

void CPU::OPCode_9C()
{
	byte val = static_cast<byte>(H.GetValue());
	SBC(val);
}

void CPU::OPCode_9D()
{
	byte val = static_cast<byte>(L.GetValue());
	SBC(val);
}

void CPU::OPCode_9E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SBC(val);
}

void CPU::OPCode_DE()
{
	byte val = readNextByte();
	SBC(val);
}

void CPU::OPCode_A7()
{
	byte val = static_cast<byte>(A.GetValue());
	AND(val);
}

void CPU::OPCode_A0()
{
	byte val = static_cast<byte>(B.GetValue());
	AND(val);
}

void CPU::OPCode_A1()
{
	byte val = static_cast<byte>(C.GetValue());
	AND(val);
}

void CPU::OPCode_A2()
{
	byte val = static_cast<byte>(D.GetValue());
	AND(val);
}

void CPU::OPCode_A3()
{
	byte val = static_cast<byte>(E.GetValue());
	AND(val);
}

void CPU::OPCode_A4()
{
	byte val = static_cast<byte>(H.GetValue());
	AND(val);
}

void CPU::OPCode_A5()
{
	byte val = static_cast<byte>(L.GetValue());
	AND(val);
}

void CPU::OPCode_A6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	AND(val);
}

void CPU::OPCode_E6()
{
	byte val = readNextByte();
	AND(val);
}

void CPU::OPCode_B7()
{
	byte val = static_cast<byte>(A.GetValue());
	OR(val);
}

void CPU::OPCode_B0()
{
	byte val = static_cast<byte>(B.GetValue());
	OR(val);
}

void CPU::OPCode_B1()
{
	byte val = static_cast<byte>(C.GetValue());
	OR(val);
}

void CPU::OPCode_B2()
{
	byte val = static_cast<byte>(D.GetValue());
	OR(val);
}

void CPU::OPCode_B3()
{
	byte val = static_cast<byte>(E.GetValue());
	OR(val);
}

void CPU::OPCode_B4()
{
	byte val = static_cast<byte>(H.GetValue());
	OR(val);
}

void CPU::OPCode_B5()
{
	byte val = static_cast<byte>(L.GetValue());
	OR(val);
}

void CPU::OPCode_B6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	OR(val);
}

void CPU::OPCode_F6()
{
	byte val = readNextByte();
	OR(val);
}

void CPU::OPCode_AF()
{
	byte val = A.GetValue();
	XOR(val);
}

void CPU::OPCode_A8()
{
	byte val = B.GetValue();
	XOR(val);
}

void CPU::OPCode_A9()
{
	byte val = C.GetValue();
	XOR(val);
}

void CPU::OPCode_AA()
{
	byte val = D.GetValue();
	XOR(val);
}

void CPU::OPCode_AB()
{
	byte val = E.GetValue();
	XOR(val);
}

void CPU::OPCode_AC()
{
	byte val = H.GetValue();
	XOR(val);
}

void CPU::OPCode_AD()
{
	byte val = L.GetValue();
	XOR(val);
}

void CPU::OPCode_AE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	XOR(val);
}

void CPU::OPCode_EE()
{
	byte val = readNextByte();
	XOR(val);
}

void CPU::OPCode_BF()
{
	byte val = static_cast<byte>(A.GetValue());
	CP(val);
}

void CPU::OPCode_B8()
{
	byte val = static_cast<byte>(B.GetValue());
	CP(val);
}

void CPU::OPCode_B9()
{
	byte val = static_cast<byte>(C.GetValue());
	CP(val);
}

void CPU::OPCode_BA()
{
	byte val = static_cast<byte>(D.GetValue());
	CP(val);
}

void CPU::OPCode_BB()
{
	byte val = static_cast<byte>(E.GetValue());
	CP(val);
}

void CPU::OPCode_BC()
{
	byte val = static_cast<byte>(H.GetValue());
	CP(val);
}

void CPU::OPCode_BD()
{
	byte val = static_cast<byte>(L.GetValue());
	CP(val);
}

void CPU::OPCode_BE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	CP(val);
}

void CPU::OPCode_FE()
{
	byte val = readNextByte();
	CP(val);
}

void CPU::OPCode_3C()
{
	INC(A);
}

void CPU::OPCode_04()
{
	INC(B);
}

void CPU::OPCode_0C()
{
	INC(C);
}

void CPU::OPCode_14()
{
	INC(D);
}

void CPU::OPCode_1C()
{
	INC(E);
}

void CPU::OPCode_24()
{
	INC(H);
}

void CPU::OPCode_2C()
{
	INC(L);
}

void CPU::OPCode_34()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	val += 1;

	// affect flags 
	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(false);
	(val & 0xF) == 0x0 ? F.SetH(true) : F.SetH(false);

	m_MMU.Write(addr, val);
}

void CPU::OPCode_3D()
{
	DEC(A);
}

void CPU::OPCode_05()
{
	DEC(B);
}

void CPU::OPCode_0D()
{
	DEC(C);
}

void CPU::OPCode_15()
{
	DEC(D);
}

void CPU::OPCode_1D()
{
	DEC(E);
}

void CPU::OPCode_25()
{
	DEC(H);
}

void CPU::OPCode_2D()
{
	DEC(L);
}

void CPU::OPCode_35()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	val -= 1;

	// affect flags
	val == 0x0 ? F.SetZ(true) : F.SetZ(false);
	F.SetN(true);
	(val & 0x0F) == 0x0F ? F.SetH(true) : F.SetH(false);

	m_MMU.Write(addr, val);
}

void CPU::OPCode_09()
{
	word val = BC.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_19()
{
	word val = DE.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_29()
{
	word val = HL.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_39()
{
	word val = SP.GetValue();
	ADD_HL(val);
}

void CPU::OPCode_E8()
{
	ADD_SP();
}

void CPU::OPCode_03()
{
	INC_no_flags(BC);
}

void CPU::OPCode_13()
{
	INC_no_flags(DE);
}

void CPU::OPCode_23()
{
	INC_no_flags(HL);
}

void CPU::OPCode_33()
{
	INC_no_flags(SP);
}

void CPU::OPCode_0B()
{
	DEC_no_flags(BC);
}

void CPU::OPCode_1B()
{
	DEC_no_flags(DE);
}

void CPU::OPCode_2B()
{
	DEC_no_flags
	(HL);
}

void CPU::OPCode_3B()
{
	DEC_no_flags(SP);
}

void CPU::OPCode_27()
{
	DAA();
}

void CPU::OPCode_2F()
{
	CPL();
}

void CPU::OPCode_3F()
{
	CCF();
}

void CPU::OPCode_37()
{
	SCF();
}

void CPU::OPCode_00()
{
	_NOP();
}

void CPU::OPCode_76()
{
	HALT();
}

void CPU::OPCode_10()
{
	STOP();
}

void CPU::OPCode_F3()
{
	DI();
}

void CPU::OPCode_FB()
{
	EI();
}

void CPU::OPCode_07()
{
	RLC_n(A);
	F.SetZ(false);
}

void CPU::OPCode_17()
{
	RL_n(A);
	F.SetZ(false);
}

void CPU::OPCode_0F()
{
	RRC_n(A);
	F.SetZ(false);
}

void CPU::OPCode_1F()
{
	RR_n(A);
	F.SetZ(false);
}

void CPU::OPCode_C3()
{
	JP_nn();
}

void CPU::OPCode_C2()
{
	JP_cc_nn(JumpConditions::NZ);
}

void CPU::OPCode_CA()
{
	JP_cc_nn(JumpConditions::Z);
}

void CPU::OPCode_D2()
{
	JP_cc_nn(JumpConditions::NC);
}

void CPU::OPCode_DA()
{
	JP_cc_nn(JumpConditions::C);
}

void CPU::OPCode_E9()
{
	JP_hl();
}

void CPU::OPCode_18()
{
	JR_n();
}

void CPU::OPCode_20()
{
	JR_cc_n(JumpConditions::NZ);
}

void CPU::OPCode_28()
{
	JR_cc_n(JumpConditions::Z);
}

void CPU::OPCode_30()
{
	JR_cc_n(JumpConditions::NC);
}

void CPU::OPCode_38()
{
	JR_cc_n(JumpConditions::C);
}

void CPU::OPCode_CD()
{
	CALL_nn();
}

void CPU::OPCode_C4()
{
	CALL_cc_nn(JumpConditions::NZ);
}

void CPU::OPCode_CC()
{
	CALL_cc_nn(JumpConditions::Z);
}

void CPU::OPCode_D4()
{
	CALL_cc_nn(JumpConditions::NC);
}

void CPU::OPCode_DC()
{
	CALL_cc_nn(JumpConditions::C);
}

void CPU::OPCode_C7()
{
	RST_n(0x0000);
}

void CPU::OPCode_CF()
{
	RST_n(0x0008);
}

void CPU::OPCode_D7()
{
	RST_n(0x0010);
}

void CPU::OPCode_DF()
{
	RST_n(0x0018);
}

void CPU::OPCode_E7()
{
	RST_n(0x0020);
}

void CPU::OPCode_EF()
{
	RST_n(0x0028);
}

void CPU::OPCode_F7()
{
	RST_n(0x0030);
}

void CPU::OPCode_FF()
{
	RST_n(0x0038);
}

void CPU::OPCode_C9()
{
	RET();
}

void CPU::OPCode_C0()
{
	RET_cc(JumpConditions::NZ);
}

void CPU::OPCode_C8()
{
	RET_cc(JumpConditions::Z);
}

void CPU::OPCode_D0()
{
	RET_cc(JumpConditions::NC);
}

void CPU::OPCode_D8()
{
	RET_cc(JumpConditions::C);
}

void CPU::OPCode_D9()
{
	RETI();
}

void CPU::OPCode_CB()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode CB! - this should never happen, CB is for extended operations!");
}

void CPU::OPCode_D3()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode D3!");
}

void CPU::OPCode_DB()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode DB!");
}

void CPU::OPCode_DD()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode DD!");
}

void CPU::OPCode_E3()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode E3!");
}

void CPU::OPCode_E4()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode E4!");
}

void CPU::OPCode_EB()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode EB!");
}

void CPU::OPCode_EC()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode EC!");
}

void CPU::OPCode_ED()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode ED!");
}

void CPU::OPCode_F4()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode F4!");
}

void CPU::OPCode_FC()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode FC!");
}

void CPU::OPCode_FD()
{
	LOG_ERROR(true, NOP, "Attempting to access an undefined OPCode FD!");
}

void CPU::OPCode_CB_37()
{
	SWAP(A);
}

void CPU::OPCode_CB_30()
{
	SWAP(B);
}

void CPU::OPCode_CB_31()
{
	SWAP(C);
}

void CPU::OPCode_CB_32()
{
	SWAP(D);
}

void CPU::OPCode_CB_33()
{
	SWAP(E);
}

void CPU::OPCode_CB_34()
{
	SWAP(H);
}

void CPU::OPCode_CB_35()
{
	SWAP(L);
}

void CPU::OPCode_CB_36()
{
	word addr = HL.GetValue();
	SWAP(addr);
}

void CPU::OPCode_CB_07()
{
	RLC_n(A);
}

void CPU::OPCode_CB_00()
{
	RLC_n(B);
}

void CPU::OPCode_CB_01()
{
	RLC_n(C);
}

void CPU::OPCode_CB_02()
{
	RLC_n(D);
}

void CPU::OPCode_CB_03()
{
	RLC_n(E);
}

void CPU::OPCode_CB_04()
{
	RLC_n(H);
}

void CPU::OPCode_CB_05()
{
	RLC_n(L);
}

void CPU::OPCode_CB_06()
{
	word addr = HL.GetValue();
	RLC_n(addr);
}

void CPU::OPCode_CB_17()
{
	RL_n(A);
}

void CPU::OPCode_CB_10()
{
	RL_n(B);
}

void CPU::OPCode_CB_11()
{
	RL_n(C);
}

void CPU::OPCode_CB_12()
{
	RL_n(D);
}

void CPU::OPCode_CB_13()
{
	RL_n(E);
}

void CPU::OPCode_CB_14()
{
	RL_n(H);
}

void CPU::OPCode_CB_15()
{
	RL_n(L);
}

void CPU::OPCode_CB_16()
{
	word addr = HL.GetValue();
	RL_n(addr);
}

void CPU::OPCode_CB_0F()
{
	RRC_n(A);
}

void CPU::OPCode_CB_08()
{
	RRC_n(B);
}

void CPU::OPCode_CB_09()
{
	RRC_n(C);
}

void CPU::OPCode_CB_0A()
{
	RRC_n(D);
}

void CPU::OPCode_CB_0B()
{
	RRC_n(E);
}

void CPU::OPCode_CB_0C()
{
	RRC_n(H);
}

void CPU::OPCode_CB_0D()
{
	RRC_n(L);
}

void CPU::OPCode_CB_0E()
{
	word addr = HL.GetValue();
	RRC_n(addr);
}

void CPU::OPCode_CB_1F()
{
	RR_n(A);
}

void CPU::OPCode_CB_18()
{
	RR_n(B);
}

void CPU::OPCode_CB_19()
{
	RR_n(C);
}

void CPU::OPCode_CB_1A()
{
	RR_n(D);
}

void CPU::OPCode_CB_1B()
{
	RR_n(E);
}

void CPU::OPCode_CB_1C()
{
	RR_n(H);
}

void CPU::OPCode_CB_1D()
{
	RR_n(L);
}

void CPU::OPCode_CB_1E()
{
	word addr = HL.GetValue();
	RR_n(addr);
}

void CPU::OPCode_CB_27()
{
	SLA_n(A);
}

void CPU::OPCode_CB_20()
{
	SLA_n(B);
}

void CPU::OPCode_CB_21()
{
	SLA_n(C);
}

void CPU::OPCode_CB_22()
{
	SLA_n(D);
}

void CPU::OPCode_CB_23()
{
	SLA_n(E);
}

void CPU::OPCode_CB_24()
{
	SLA_n(H);
}

void CPU::OPCode_CB_25()
{
	SLA_n(L);
}

void CPU::OPCode_CB_26()
{
	word addr = HL.GetValue();
	SLA_n(addr);
}

void CPU::OPCode_CB_2F()
{
	SRA_n(A);
}

void CPU::OPCode_CB_28()
{
	SRA_n(B);
}

void CPU::OPCode_CB_29()
{
	SRA_n(C);
}

void CPU::OPCode_CB_2A()
{
	SRA_n(D);
}

void CPU::OPCode_CB_2B()
{
	SRA_n(E);
}

void CPU::OPCode_CB_2C()
{
	SRA_n(H);
}

void CPU::OPCode_CB_2D()
{
	SRA_n(L);
}

void CPU::OPCode_CB_2E()
{
	word addr = HL.GetValue();
	SRA_n(addr);
}

void CPU::OPCode_CB_3F()
{
	SRL_n(A);
}

void CPU::OPCode_CB_38()
{
	SRL_n(B);
}

void CPU::OPCode_CB_39()
{
	SRL_n(C);
}

void CPU::OPCode_CB_3A()
{
	SRL_n(D);
}

void CPU::OPCode_CB_3B()
{
	SRL_n(E);
}

void CPU::OPCode_CB_3C()
{
	SRL_n(H);
}

void CPU::OPCode_CB_3D()
{
	SRL_n(L);
}

void CPU::OPCode_CB_3E()
{
	word addr = HL.GetValue();
	SRL_n(addr);
}

void CPU::OPCode_CB_47()
{
	byte val = A.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_40()
{
	byte val = B.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_41()
{
	byte val = C.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_42()
{
	byte val = D.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_43()
{
	byte val = E.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_44()
{
	byte val = H.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_45()
{
	byte val = L.GetValue();
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_46()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(0, val);
}

void CPU::OPCode_CB_4F()
{
	byte val = A.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_48()
{
	byte val = B.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_49()
{
	byte val = static_cast<byte>(C.GetValue());
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4A()
{
	byte val = D.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4B()
{
	byte val = E.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4C()
{
	byte val = H.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4D()
{
	byte val = L.GetValue();
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_4E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(1, val);
}

void CPU::OPCode_CB_57()
{
	byte val = A.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_50()
{
	byte val = B.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_51()
{
	byte val = C.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_52()
{
	byte val = D.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_53()
{
	byte val = E.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_54()
{
	byte val = H.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_55()
{
	byte val = L.GetValue();
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_56()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(2, val);
}

void CPU::OPCode_CB_5F()
{
	byte val = A.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_58()
{
	byte val = B.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_59()
{
	byte val = C.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5A()
{
	byte val = D.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5B()
{
	byte val = E.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5C()
{
	byte val = H.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5D()
{
	byte val = L.GetValue();
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_5E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(3, val);
}

void CPU::OPCode_CB_67()
{
	byte val = A.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_60()
{
	byte val = B.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_61()
{
	byte val = C.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_62()
{
	byte val = D.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_63()
{
	byte val = E.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_64()
{
	byte val = H.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_65()
{
	byte val = L.GetValue();
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_66()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(4, val);
}

void CPU::OPCode_CB_6F()
{
	byte val = A.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_68()
{
	byte val = B.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_69()
{
	byte val = C.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6A()
{
	byte val = D.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6B()
{
	byte val = E.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6C()
{
	byte val = H.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6D()
{
	byte val = L.GetValue();
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_6E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(5, val);
}

void CPU::OPCode_CB_77()
{
	byte val = A.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_70()
{
	byte val = B.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_71()
{
	byte val = C.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_72()
{
	byte val = D.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_73()
{
	byte val = E.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_74()
{
	byte val = H.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_75()
{
	byte val = L.GetValue();
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_76()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(6, val);
}

void CPU::OPCode_CB_7F()
{
	byte val = A.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_78()
{
	byte val = B.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_79()
{
	byte val = C.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7A()
{
	byte val = D.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7B()
{
	byte val = E.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7C()
{
	byte val = H.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7D()
{
	byte val = L.GetValue();
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_7E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	BIT_b_r(7, val);
}

void CPU::OPCode_CB_C7()
{
	byte val = A.GetValue();
	SET_b_r(0, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_C0()
{
	byte val = B.GetValue();
	SET_b_r(0, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_C1()
{
	byte val = C.GetValue();
	SET_b_r(0, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_C2()
{
	byte val = D.GetValue();
	SET_b_r(0, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_C3()
{
	byte val = E.GetValue();
	SET_b_r(0, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_C4()
{
	byte val = H.GetValue();
	SET_b_r(0, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_C5()
{
	byte val = L.GetValue();
	SET_b_r(0, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_C6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(0, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_CF()
{
	byte val = A.GetValue();
	SET_b_r(1, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_C8()
{
	byte val = B.GetValue();
	SET_b_r(1, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_C9()
{
	byte val = C.GetValue();
	SET_b_r(1, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_CA()
{
	byte val = D.GetValue();
	SET_b_r(1, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_CB()
{
	byte val = E.GetValue();
	SET_b_r(1, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_CC()
{
	byte val = H.GetValue();
	SET_b_r(1, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_CD()
{
	byte val = L.GetValue();
	SET_b_r(1, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_CE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(1, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_D7()
{
	byte val = A.GetValue();
	SET_b_r(2, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_D0()
{
	byte val = B.GetValue();
	SET_b_r(2, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_D1()
{
	byte val = C.GetValue();
	SET_b_r(2, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_D2()
{
	byte val = D.GetValue();
	SET_b_r(2, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_D3()
{
	byte val = E.GetValue();
	SET_b_r(2, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_D4()
{
	byte val = H.GetValue();
	SET_b_r(2, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_D5()
{
	byte val = L.GetValue();
	SET_b_r(2, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_D6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(2, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_DF()
{
	byte val = A.GetValue();
	SET_b_r(3, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_D8()
{
	byte val = B.GetValue();
	SET_b_r(3, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_D9()
{
	byte val = C.GetValue();
	SET_b_r(3, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_DA()
{
	byte val = D.GetValue();
	SET_b_r(3, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_DB()
{
	byte val = E.GetValue();
	SET_b_r(3, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_DC()
{
	byte val = H.GetValue();
	SET_b_r(3, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_DD()
{
	byte val = L.GetValue();
	SET_b_r(3, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_DE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(3, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_E7()
{
	byte val = A.GetValue();
	SET_b_r(4, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_E0()
{
	byte val = B.GetValue();
	SET_b_r(4, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_E1()
{
	byte val = C.GetValue();
	SET_b_r(4, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_E2()
{
	byte val = D.GetValue();
	SET_b_r(4, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_E3()
{
	byte val = E.GetValue();
	SET_b_r(4, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_E4()
{
	byte val = H.GetValue();
	SET_b_r(4, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_E5()
{
	byte val = L.GetValue();
	SET_b_r(4, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_E6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(4, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_EF()
{
	byte val = A.GetValue();
	SET_b_r(5, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_E8()
{
	byte val = B.GetValue();
	SET_b_r(5, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_E9()
{
	byte val = C.GetValue();
	SET_b_r(5, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_EA()
{
	byte val = D.GetValue();
	SET_b_r(5, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_EB()
{
	byte val = E.GetValue();
	SET_b_r(5, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_EC()
{
	byte val = H.GetValue();
	SET_b_r(5, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_ED()
{
	byte val = L.GetValue();
	SET_b_r(5, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_EE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(5, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_F7()
{
	byte val = A.GetValue();
	SET_b_r(6, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_F0()
{
	byte val = B.GetValue();
	SET_b_r(6, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_F1()
{
	byte val = C.GetValue();
	SET_b_r(6, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_F2()
{
	byte val = D.GetValue();
	SET_b_r(6, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_F3()
{
	byte val = E.GetValue();
	SET_b_r(6, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_F4()
{
	byte val = H.GetValue();
	SET_b_r(6, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_F5()
{
	byte val = L.GetValue();
	SET_b_r(6, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_F6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(6, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_FF()
{
	byte val = A.GetValue();
	SET_b_r(7, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_F8()
{
	byte val = B.GetValue();
	SET_b_r(7, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_F9()
{
	byte val = C.GetValue();
	SET_b_r(7, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_FA()
{
	byte val = D.GetValue();
	SET_b_r(7, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_FB()
{
	byte val = E.GetValue();
	SET_b_r(7, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_FC()
{
	byte val = H.GetValue();
	SET_b_r(7, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_FD()
{
	byte val = L.GetValue();
	SET_b_r(7, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_FE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	SET_b_r(7, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_87()
{
	byte val = A.GetValue();
	RES_b_r(0, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_80()
{
	byte val = B.GetValue();
	RES_b_r(0, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_81()
{
	byte val = C.GetValue();
	RES_b_r(0, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_82()
{
	byte val = D.GetValue();
	RES_b_r(0, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_83()
{
	byte val = E.GetValue();
	RES_b_r(0, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_84()
{
	byte val = H.GetValue();
	RES_b_r(0, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_85()
{
	byte val = L.GetValue();
	RES_b_r(0, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_86()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(0, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_8F()
{
	byte val = A.GetValue();
	RES_b_r(1, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_88()
{
	byte val = B.GetValue();
	RES_b_r(1, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_89()
{
	byte val = C.GetValue();
	RES_b_r(1, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_8A()
{
	byte val = D.GetValue();
	RES_b_r(1, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_8B()
{
	byte val = E.GetValue();
	RES_b_r(1, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_8C()
{
	byte val = H.GetValue();
	RES_b_r(1, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_8D()
{
	byte val = L.GetValue();
	RES_b_r(1, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_8E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(1, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_97()
{
	byte val = A.GetValue();
	RES_b_r(2, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_90()
{
	byte val = B.GetValue();
	RES_b_r(2, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_91()
{
	byte val = C.GetValue();
	RES_b_r(2, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_92()
{
	byte val = D.GetValue();
	RES_b_r(2, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_93()
{
	byte val = E.GetValue();
	RES_b_r(2, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_94()
{
	byte val = H.GetValue();
	RES_b_r(2, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_95()
{
	byte val = L.GetValue();
	RES_b_r(2, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_96()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(2, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_9F()
{
	byte val = A.GetValue();
	RES_b_r(3, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_98()
{
	byte val = B.GetValue();
	RES_b_r(3, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_99()
{
	byte val = C.GetValue();
	RES_b_r(3, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_9A()
{
	byte val = D.GetValue();
	RES_b_r(3, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_9B()
{
	byte val = E.GetValue();
	RES_b_r(3, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_9C()
{
	byte val = H.GetValue();
	RES_b_r(3, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_9D()
{
	byte val = L.GetValue();
	RES_b_r(3, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_9E()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(3, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_A7()
{
	byte val = A.GetValue();
	RES_b_r(4, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_A0()
{
	byte val = B.GetValue();
	RES_b_r(4, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_A1()
{
	byte val = C.GetValue();
	RES_b_r(4, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_A2()
{
	byte val = D.GetValue();
	RES_b_r(4, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_A3()
{
	byte val = E.GetValue();
	RES_b_r(4, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_A4()
{
	byte val = H.GetValue();
	RES_b_r(4, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_A5()
{
	byte val = L.GetValue();
	RES_b_r(4, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_A6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(4, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_AF()
{
	byte val = A.GetValue();
	RES_b_r(5, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_A8()
{
	byte val = B.GetValue();
	RES_b_r(5, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_A9()
{
	byte val = C.GetValue();
	RES_b_r(5, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_AA()
{
	byte val = D.GetValue();
	RES_b_r(5, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_AB()
{
	byte val = E.GetValue();
	RES_b_r(5, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_AC()
{
	byte val = H.GetValue();
	RES_b_r(5, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_AD()
{
	byte val = L.GetValue();
	RES_b_r(5, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_AE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(5, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_B7()
{
	byte val = A.GetValue();
	RES_b_r(6, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_B0()
{
	byte val = B.GetValue();
	RES_b_r(6, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_B1()
{
	byte val = C.GetValue();
	RES_b_r(6, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_B2()
{
	byte val = D.GetValue();
	RES_b_r(6, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_B3()
{
	byte val = E.GetValue();
	RES_b_r(6, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_B4()
{
	byte val = H.GetValue();
	RES_b_r(6, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_B5()
{
	byte val = L.GetValue();
	RES_b_r(6, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_B6()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(6, val);
	m_MMU.Write(addr, val);
}

void CPU::OPCode_CB_BF()
{
	byte val = A.GetValue();
	RES_b_r(7, val);
	A.SetValue(val);
}

void CPU::OPCode_CB_B8()
{
	byte val = B.GetValue();
	RES_b_r(7, val);
	B.SetValue(val);
}

void CPU::OPCode_CB_B9()
{
	byte val = C.GetValue();
	RES_b_r(7, val);
	C.SetValue(val);
}

void CPU::OPCode_CB_BA()
{
	byte val = D.GetValue();
	RES_b_r(7, val);
	D.SetValue(val);
}

void CPU::OPCode_CB_BB()
{
	byte val = E.GetValue();
	RES_b_r(7, val);
	E.SetValue(val);
}

void CPU::OPCode_CB_BC()
{
	byte val = H.GetValue();
	RES_b_r(7, val);
	H.SetValue(val);
}

void CPU::OPCode_CB_BD()
{
	byte val = L.GetValue();
	RES_b_r(7, val);
	L.SetValue(val);
}

void CPU::OPCode_CB_BE()
{
	word addr = HL.GetValue();
	byte val = m_MMU.Read(addr);
	RES_b_r(7, val);
	m_MMU.Write(addr, val);
}

/* debug methods */

/* registers dump into input ostream (could be stdout or a file) */
void CPU::dumpRegisters(std::ostream& i_OStream)
{
	i_OStream << "Registers dump:" << endl;
	i_OStream << "A: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)A.GetValue() << " | F: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)F.GetValue() << " | AF: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)AF.GetValue() << endl;
	i_OStream << "B: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)B.GetValue() << " | C: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)C.GetValue() << " | BC: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)BC.GetValue() << endl;
	i_OStream << "D: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)D.GetValue() << " | E: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)E.GetValue() << " | DE: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)DE.GetValue() << endl;
	i_OStream << "H: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)H.GetValue() << " | L: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)L.GetValue() << " | HL: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)HL.GetValue() << endl;
	i_OStream << "------------------------------" << endl;
	i_OStream << "PC: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)PC.GetValue() << " | SP: 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)SP.GetValue() << " | (HL): 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)m_MMU.Read(HL.GetValue()) << endl;
	i_OStream << "------------------------------" << endl;
	i_OStream << "Z: " << F.GetZ() << " | N: " << F.GetN() << " | H: " << F.GetH() << " | C: " << F.GetC() << endl;
}