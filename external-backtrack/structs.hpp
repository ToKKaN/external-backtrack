#pragma once
#include "vector.hpp"

enum playercontrols
{
	IN_ATTACK = ( 1 << 0 ),
	IN_JUMP = ( 1 << 1 ),
	IN_DUCK = ( 1 << 2 ),
	IN_FORWARD = ( 1 << 3 ),
	IN_BACK = ( 1 << 4 ),
	IN_USE = ( 1 << 5 ),
	IN_CANCEL = ( 1 << 6 ),
	IN_LEFT = ( 1 << 7 ),
	IN_RIGHT = ( 1 << 8 ),
	IN_MOVELEFT = ( 1 << 9 ),
	IN_MOVERIGHT = ( 1 << 10 ),
	IN_ATTACK2 = ( 1 << 11 ),
	IN_RUN = ( 1 << 12 ),
	IN_RELOAD = ( 1 << 13 ),
	IN_ALT1 = ( 1 << 14 ),
	IN_ALT2 = ( 1 << 15 ),
	IN_SCORE = ( 1 << 16 ),
	IN_SPEED = ( 1 << 17 ),
	IN_WALK = ( 1 << 18 ),
	IN_ZOOM = ( 1 << 19 ),
	IN_WEAPON1 = ( 1 << 20 ),
	IN_WEAPON2 = ( 1 << 21 ),
	IN_BULLRUSH = ( 1 << 22 ),
};
struct input_t
{
	char	  pad_0x00[ 0x0C ];                   // 0x00
	bool      m_bTrackIRAvailable;          // 0x04
	bool      m_bMouseInitialized;          // 0x05
	bool      m_bMouseActive;               // 0x06
	bool      m_bJoystickAdvancedInit;      // 0x07
	uint8_t   Unk1[ 44 ];                     // 0x08
	uintptr_t m_pKeys;                      // 0x34
	uint8_t   Unk2[ 100 ];                    // 0x38
	bool      m_bCameraInterceptingMouse;   // 0x9C
	bool      m_bCameraInThirdPerson;       // 0x9D
	bool      m_bCameraMovingWithMouse;     // 0x9E
	Vector	  m_vecCameraOffset;            // 0xA0
	bool      m_bCameraDistanceMove;        // 0xAC
	int32_t   m_nCameraOldX;                // 0xB0
	int32_t   m_nCameraOldY;                // 0xB4
	int32_t   m_nCameraX;                   // 0xB8
	int32_t   m_nCameraY;                   // 0xBC
	bool      m_bCameraIsOrthographic;      // 0xC0
	Vector	  m_vecPreviousViewAngles;      // 0xC4
	Vector	  m_vecPreviousViewAnglesTilt;  // 0xD0
	float     m_flLastForwardMove;          // 0xDC
	int32_t   m_nClearInputState;           // 0xE0
	uint8_t   Unk3[ 0x8 ];                    // 0xE4
	uintptr_t m_pCommands;                  // 0xEC
	uintptr_t m_pVerifiedCommands;          // 0xF0
};
struct usercmd_t
{
	uintptr_t pVft;                // 0x00
	int32_t   m_iCmdNumber;        // 0x04
	int32_t   m_iTickCount;        // 0x08
	Vector    m_vecViewAngles;     // 0x0C
	Vector    m_vecAimDirection;   // 0x18
	float     m_flForwardmove;     // 0x24
	float     m_flSidemove;        // 0x28
	float     m_flUpmove;          // 0x2C
	int32_t   m_iButtons;          // 0x30
	uint8_t   m_bImpulse;          // 0x34
	uint8_t   Pad1[ 3 ];
	int32_t   m_iWeaponSelect;     // 0x38
	int32_t   m_iWeaponSubtype;    // 0x3C
	int32_t   m_iRandomSeed;       // 0x40
	int16_t   m_siMouseDx;         // 0x44
	int16_t   m_siMouseDy;         // 0x46
	bool      m_bHasBeenPredicted; // 0x48
	uint8_t   Pad2[ 27 ];
}; //0x64
struct verified_usercmd_t
{
	usercmd_t m_Command;
	uint32_t  m_Crc;
};
struct globalvars_t
{
	float    realtime;
	int      framecount;
	float    absoluteframetime;
	float    absoluteframestarttimestddev;
	float    curtime;
	float    frametime;
	int      maxClients;
	int      tickcount;
	float    interval_per_tick;
	float    interpolation_amount;
	int      simTicksThisFrame;
	int      network_protocol;
	void*    pSaveData;
	bool     m_bClient;
	int      nTimestampNetworkingBase;
	int      nTimestampRandomizeWindow;
};
struct backtrack_data_t
{
	float simtime;
	Vector hitboxpos;
};