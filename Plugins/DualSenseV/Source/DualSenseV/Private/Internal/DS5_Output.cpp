#include "DS5_Output.h"

#include "DS5_Input.h"

void __DS5W::Output::createHidOutputBuffer(unsigned char* hidOutBuffer, DS5W::DS5OutputState* ptrOutputState) {
	// Feature mask
	hidOutBuffer[0x00] = 0xFF;  // 0xFC for haptic feedback and 0xFF for rumble
	hidOutBuffer[0x01] = 0xF7;

	// Rumble motors
	hidOutBuffer[0x02] = ptrOutputState->rightRumble;
	hidOutBuffer[0x03] = ptrOutputState->leftRumble;

	// Volumes
	//hidOutBuffer[0x04] = 0x7F; // Headphones (max. at 0x7F)
	//hidOutBuffer[0x05] = 0x3D; // Speaker (0x3D-0x64)
	//hidOutBuffer[0x06] = 0x40; // Microphone (max. at 0x40; 0 not fully muted)
	
	// Audio control
	//hidOutBuffer[0x07] = 0x05;
	
	// Mic led
	hidOutBuffer[0x08] = (unsigned char)ptrOutputState->microphoneLed;

	// Player led
	hidOutBuffer[0x2B] = ptrOutputState->playerLeds.bitmask;
	if (ptrOutputState->playerLeds.playerLedFade) {
		hidOutBuffer[0x2B] &= ~(0x20);
	}
	else {
		hidOutBuffer[0x2B] |= 0x20;
	}

	// Player led brightness
	hidOutBuffer[0x26] = 0x03;
	hidOutBuffer[0x29] = ptrOutputState->disableLeds ? 0x01 : 0x2;
	hidOutBuffer[0x2A] = ptrOutputState->playerLeds.brightness;

	// Lightbar
	hidOutBuffer[0x2C] = ptrOutputState->lightbar.r;
	hidOutBuffer[0x2D] = ptrOutputState->lightbar.g;
	hidOutBuffer[0x2E] = ptrOutputState->lightbar.b;

	// Adaptive Triggers
	processTrigger(&ptrOutputState->leftTriggerEffect, &hidOutBuffer[0x15]);
	processTrigger(&ptrOutputState->rightTriggerEffect, &hidOutBuffer[0x0A]);
}

void __DS5W::Output::processTrigger(const DS5W::TriggerEffect* ptrEffect, unsigned char* buffer) {
#if PLATFORM_WINDOWS
	constexpr rsize_t BufferSize = sizeof(DS5W::TriggerEffect);
	memcpy_s(
		buffer, BufferSize,
		ptrEffect, BufferSize
	);
#endif
}
