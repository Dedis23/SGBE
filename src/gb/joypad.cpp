#include "joypad.h"

Joypad::Joypad(Gameboy& i_Gameboy) : m_Gameboy(i_Gameboy), m_JoypadState(0xFF) {}

void Joypad::Reset()
{
	m_JoypadState = 0xFF;
}

/* used when the player has pressed the button */
void Joypad::KeyPressed(const Joypad::Buttons& i_PressedButton)
{

}

/* used when the player has released the button */
void Joypad::KeyReleased(const Joypad::Buttons& i_ReleasedButton)
{

}

/* used when the game request the joypad state */
byte Joypad::GetJoypadState() const
{
	return m_JoypadState;
}

/* used when the game alters the joypad state */
void Joypad::SetJoypadState(byte i_Value)
{
	
}