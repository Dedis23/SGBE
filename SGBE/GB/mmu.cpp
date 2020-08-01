#include "mmu.h"

MMU::MMU(Gameboy& i_Gameboy, Cartridge& i_Cartridge) : m_Gameboy(i_Gameboy), m_Cartridge(i_Cartridge)
{
    memory = std::vector<byte>(0x10000);
}

byte MMU::Read(word i_Address) const
{
    /* more info about the sections is in the h file */

    /* Cartridge ROM banks and bootstrap */
    if (i_Address >= 0x0000 && i_Address <= 0x7FFF)
    {
        if (i_Address >= 0x00 && i_Address <= 0xFF)
        {
            /* bootstrap */
            if (m_MappedIO[BOOTSTRAP_DONE_ADDR - 0xFF00] == false)
            {
                //cout << "read from BOOTSTRAP rom address is: " << (int)i_Address << endl;
                //cout << "the value is: 0x" << std::hex << (int)s_Bootstrap[i_Address] << endl;
                return s_Bootstrap[i_Address];
            }
            else
            {
                cout << "BOOTSTRAP is done!" << endl;
                return m_Cartridge.Read(i_Address);
            }
        }
    }

    /* Video RAM */
    else if (i_Address >= 0x8000 && i_Address <= 0x9FFF)
    {
        return memory.at(i_Address);
        //return m_VRAM[i_Address - 0x8000];
    }

    /* External RAM (RAM on specific cartridge types which supported this) */
    else if (i_Address >= 0xA000 && i_Address <= 0xBFFF)
    {
        return m_Cartridge.Read(i_Address - 0xA000);
    }

    /* Internal RAM */
    else if (i_Address >= 0xC000 && i_Address <= 0xDFFF)
    {
        return memory.at(i_Address);
        //return m_RAM[i_Address - 0xC000];
    }

    /* Shadow RAM */
    else if (i_Address >= 0xE000 && i_Address <= 0xFDFF)
    {
        /* reading exact copy from the internal RAM which is 0x2000 addresses below */
        auto mirrored_address = i_Address - 0x2000;
        return memory.at(mirrored_address);
        //return m_RAM[i_Address - 0x2000];
    }

    /* OAM */
    else if (i_Address >= 0xFE00 && i_Address <= 0xFE9F)
    {
        return memory.at(i_Address);
        //return m_OAM[i_Address - 0xFE00];
    }

    /* Unusable area - shouldn't get here */
    else if (i_Address >= 0xFEA0 && i_Address <= 0xFEFF)
    {
        LOG_ERROR(true, return 0, "Attempting to read from unusable memory address: 0x" << i_Address);
    }

    /* Mapped IO */
    else if (i_Address >= 0xFF00 && i_Address <= 0xFF7F)
    {
        return readMappedIO(i_Address);
    }

    /* Zero Page RAM */
    else if (i_Address >= 0xFF80 && i_Address <= 0xFFFF)
    {
        return memory.at(i_Address);
        //return m_ZeroPageRAM[i_Address - 0xFF80];
    }

    LOG_ERROR(true, return 0, "Attempting to read from unmapped memory address: 0x" << i_Address);
}

void MMU::Write(word i_Address, byte i_Value)
{
    bool wroteToAddr = false;
    /* more info about the sections is in the h file */

    /* Cartridge ROM banks */
    if (i_Address >= 0x0000 && i_Address <= 0x7FFF)
    {
        LOG_ERROR(true, return, "Attempting to write to address: 0x" << i_Address << " which is a cartridge ROM bank address!");
    }

    /* Video RAM */
    else if (i_Address >= 0x8000 && i_Address <= 0x9FFF)
    {
        memory.at(i_Address) = i_Value; return;
        //m_VRAM[i_Address - 0x8000] = i_Value;
        //wroteToAddr = true;
    }

    /* External RAM (RAM on specific cartridge types which supported this) */
    else if (i_Address >= 0xA000 && i_Address <= 0xBFFF)
    {
        m_Cartridge.Write(i_Address - 0xA000, i_Value);
        wroteToAddr = true;
    }

    /* Internal RAM */
    else if (i_Address >= 0xC000 && i_Address <= 0xDFFF)
    {
        memory.at(i_Address) = i_Value; return;
        //m_RAM[i_Address - 0xC000] = i_Value;
        //wroteToAddr = true;
    }

    /* Shadow RAM */
    else if (i_Address >= 0xE000 && i_Address <= 0xFDFF)
    {
        /* writing in the Shadow RAM is like writing in the internal RAM banks. the shadow ram is an exact copy from the internal RAM which is 0x2000 addresses below */
        /* i.e writing to 0xE000 is like writing to 0xC000, so 0x2000 is reduced and then 0xC000 (0xE000 total) to get to the place in the m_RAM array */
        auto mirrored_address = i_Address - 0x2000;
        memory.at(mirrored_address) = i_Value; return;
        //m_RAM[i_Address - 0xE000] = i_Value;
        //wroteToAddr = true;
    }

    /* OAM */
    else if (i_Address >= 0xFE00 && i_Address <= 0xFE9F)
    {
        memory.at(i_Address) = i_Value; return;
        //m_OAM[i_Address - 0xFE00] = i_Value;
        //wroteToAddr = true;
    }

    /* Unusable area - shouldn't get here */
    else if (i_Address >= 0xFEA0 && i_Address <= 0xFEFF)
    {
        LOG_ERROR(true, return, "Attempting to write to an unusable memory address: 0x" << i_Address);
    }

    /* Mapped IO */
    else if (i_Address >= 0xFF00 && i_Address <= 0xFF7F)
    {
        writeMappedIO(i_Address, i_Value);
        wroteToAddr = true;
    }

    /* Zero Page RAM */
    else if (i_Address >= 0xFF80 && i_Address <= 0xFFFF)
    {
        memory.at(i_Address) = i_Value; return;
        //m_ZeroPageRAM[i_Address - 0xFF80] = i_Value;
        //wroteToAddr = true;
    }
    
    /* debug printing without timer writes, this should be commented */
    //LOG_INFO(wroteToAddr == true
    //    && i_Address.GetValue() != TIMER_DIVIDER_ADDR
    //    && i_Address.GetValue() != TIMER_COUNTER_ADDR
    //    && i_Address.GetValue() != TIMER_MODULO_ADDR
    //    && i_Address.GetValue() != TIMER_CONTROL_ADDR,
    //    return, "Wrote 0x" << std::hex << static_cast<word>(i_Value) << " in address 0x" << std::hex << i_Address.GetValue());
    if (wroteToAddr) return;

    LOG_ERROR(true, return, "Attempting to write to an unmapped memory address: 0x" << i_Address);
}

/* When the game writes to the 0xFF46 it will initiate
   a DMA (direct memory access) which will copy A0h (160) values from RAM into OAM (Sprites attribute table) in addresses: 0xFE00-0xFE9F
   the source to be copied is taken by the value that is written, however it needs to be multiplied by 100h (256) */
void MMU::DMATransfer(byte i_SourceAdress)
{
    word adr = i_SourceAdress * 0x100;
    for (int i = 0; i < 0xA0; i++)
    {
        byte val = Read(adr + i);
        Write(0xFE00 + i, val);
    }
}

byte MMU::readMappedIO(word i_Address) const
{
    if (i_Address >= TIMER_DIVIDER_ADDR && i_Address <= TIMER_CONTROL_ADDR)
    {
        return m_Gameboy.GetTimer().GetRegister(i_Address);
    }
    else if (i_Address >= GPU_LCD_CONTROL_ADDR && i_Address <= GPU_WINDOW_X_POSITION_MINUS_7_ADDR)
    {
        return m_Gameboy.GetGPU().GetRegister(i_Address);
    }
    else
    {
        // any other mapped i/o
        return m_MappedIO[i_Address - 0xFF00];
    }
}

/* write to memory modules from other componenets will sometimes invoke specific methods of the modules
   or will override the new value to a different, specifc value */
void MMU::writeMappedIO(word i_Address, byte i_Value)
{
    if (i_Address >= TIMER_DIVIDER_ADDR && i_Address <= TIMER_CONTROL_ADDR)
    {
        m_Gameboy.GetTimer().SetRegister(i_Address, i_Value);
    }
    else if (i_Address >= GPU_LCD_CONTROL_ADDR && i_Address <= GPU_WINDOW_X_POSITION_MINUS_7_ADDR)
    {
        m_Gameboy.GetGPU().SetRegister(i_Address, i_Value);
    }
    else
    {
        // any other mapped i/o
        m_MappedIO[i_Address - 0xFF00] = i_Value;
    }
}

bool MMU::isBootstrapDone() const
{
    // at the end of the bootstrap, a value of 1 is written to address 0xFF50
    // this way the addresses 0x00 ~ 0xFF from the cartridge can be access because we know that we are post boot
    return Read(BOOTSTRAP_DONE_ADDR);
}

const vector<byte> MMU::s_Bootstrap =
{
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50,
};