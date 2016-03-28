#include "xtftConfig.h"

/*
 * Initialize the TFT controller
 */
int TftInit(u32 TftDeviceId, XTft* TftInstance) {
	int status;
	XTft_Config *TftConfigPtr;
	/*
	 * Get address of the XTft_Config structure for the given device id.
	 */
	TftConfigPtr = XTft_LookupConfig(TftDeviceId);
	if (TftConfigPtr == (XTft_Config *)NULL) {
		return XST_FAILURE;
	}

	/*
	 * Initialize all the TftInstance members and fills the screen with
	 * default background color.
	 */
	status = XTft_CfgInitialize(TftInstance, TftConfigPtr, TftConfigPtr->BaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XTft_IntrEnable(TftInstance);
	/*
	 * Wait till Vsync(Video address latch) status bit is set before writing
	 * the frame address into the Address Register. This ensures that the
	 * current frame has been displayed and we can display a new frame of
	 * data. Checking the Vsync state ensures that there is no data flicker
	 * when displaying frames in real time though there is some delay due to
	 * polling.
	 */
	while (XTft_GetVsyncStatus(TftInstance) !=
			XTFT_IESR_VADDRLATCH_STATUS_MASK);

	/*
	 * Change the Video Memory Base Address from default value to
	 * a valid Memory Address and clear the screen.
	 */
	XTft_SetFrameBaseAddr(TftInstance, TFT_FRAME_ADDR);
	XTft_ClearScreen(TftInstance);

	return XST_SUCCESS;
}
