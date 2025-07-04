
#pragma once

#include "DS5State.h"

namespace __DS5W {
	namespace Output {
		/// <summary>
		/// Creates the hid output buffer
		/// </summary>
		/// <param name="hidOutBuffer">HID Output buffer</param>
		/// <param name="ptrOutputState">Pointer to state to read from</param>
		void createHidOutputBuffer(unsigned char* hidOutBuffer, DS5W::DS5OutputState* ptrOutputState);

		/// <summary>
		/// Process trigger
		/// </summary>
		/// <param name="ptrEffect">Pointer to effect to be applied</param>
		/// <param name="buffer">Buffer for trigger parameters</param>
		void processTrigger(const DS5W::TriggerEffect* ptrEffect, unsigned char* buffer);
	}
}