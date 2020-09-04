/*******************************************************
 *		Created by: Dedi Sidi, 2020                    *
 *											           *
 *		Joypad - responsible for the gameboy input     *
 *******************************************************/

#pragma once
#include "utility.h"
#include "gameboy.h"

/*
    info mostly taken from:
    http://www.codeslinger.co.uk/pages/projects/gameboy/joypad.html
    https://gbdev.gg8.se/wiki/articles/Joypad_Input
*/

/* Joypad registers addresses in memory */
const word JOYPAD_ADDR = 0xFF00;

/* Joypad keys bits bits                *
 * Note - 0 = pressed, 1 = unpressed!   */
#define JOYPAD_SELECT_BUTTON_KEYS_BIT       5 // R/W
#define JOYPAD_SELECT_DIRECTION_KEYS_BIT    4 // R/W
#define JOYPAD_INPUT_DOWN_OR_START_BIT      3 // R/O for the game (only the user alters this)
#define JOYPAD_INPUT_UP_OR_SELECT_BIT       2 // R/O for the game (only the user alters this)
#define JOYPAD_INPUT_LEFT_OR_BUTTON_B_BIT   1 // R/O for the game (only the user alters this)
#define JOYPAD_INPUT_RIGHT_OR_BUTTON_A_BIT  0 // R/O for the game (only the user alters this)

enum class GBButtons
{
    /* lower nibble of the state */
    Right = 0, // bit 0
    Left = 1, // bit 1
    Up = 2, // bit 2
    Down = 3, // bit 3
    /* upper nibble of the state */
    A = 4, // bit 0 in the upper nibble
    B = 5, // bit 1 in the upper nibble
    Select = 6, // bit 2 in the upper nibble
    Start = 7, // bit 3 in the upper nibble
    /* Any other key which will not be handled */
    None,
};

class Joypad
{

public:
    Joypad(Gameboy& i_Gameboy);
	virtual ~Joypad() = default;
    Joypad(const Joypad&) = delete;
    Joypad& operator=(const Joypad&) = delete;

    void Reset();
    void KeyPressed(const GBButtons& i_PressedButton);
    void KeyReleased(const GBButtons& i_ReleasedButton);
    byte GetJoypadState() const;
    void SetJoypadState(byte i_Value);

private:
    byte m_KeysState;
    bool m_DirectionSelector;
    bool m_ButtonSelector;

    /* gameboy ref */
    Gameboy& m_Gameboy;
};