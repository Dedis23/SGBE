#include "‏‏gb_mmu.h"

GBMMU::GBMMU(GBCartridge& i_Cartridge) : m_Cartridge(i_Cartridge) {}

byte GBMMU::Read(const WordAddress& i_Address) const
{
    // bootstrap
    // if(!bootstrap_done)
    // check 0 to 256
    // if (address.in_range(0x0, 0xFF) && boot_rom_active()) {
    //     return bootDMG[address.value()];
    // }

    // cartridge section
    //if (address.in_range(0x0, 0x7FFF))
    //{
    //  return cartridge->read(address);
    //}

    ///* VRAM */
    //if (address.in_range(0x8000, 0x9FFF)) {
    //    return memory_read(address);
    //}
    //
    ///* External (cartridge) RAM */
    //if (address.in_range(0xA000, 0xBFFF)) {
    //    return cartridge->read(address);
    //}
    //
    ///* Internal work RAM */
    //if (address.in_range(0xC000, 0xDFFF)) {
    //    return memory_read(address);
    //}
    //
    //if (address.in_range(0xE000, 0xFDFF)) {
    //    /* log_warn("Attempting to read from mirrored work RAM"); */
    //    auto mirrored_address = Address(address.value() - 0x2000);
    //    return memory_read(mirrored_address);
    //}
    //
    ///* OAM */
    //if (address.in_range(0xFE00, 0xFE9F)) {
    //    return memory_read(address);
    //}
    //
    //if (address.in_range(0xFEA0, 0xFEFF)) {
    //    log_warn("Attempting to read from unusable memory 0x%x", address.value());
    //    return 0xFF;
    //}
    //
    ///* Mapped IO */
    //if (address.in_range(0xFF00, 0xFF7F)) {
    //    return read_io(address);
    //}
    //
    ///* Zero Page ram */
    //if (address.in_range(0xFF80, 0xFFFE)) {
    //    return memory_read(address);
    //}
    //
    ///* Interrupt Enable register */
    //if (address == 0xFFFF) {
    //    return cpu.interrupt_enabled.value();
    //}

    byte stam = 255; // remove this
    return stam;
}

void GBMMU::Write(const WordAddress& i_Address, byte i_Value)
{
	// TODO
}
