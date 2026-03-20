/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"
#include "System.h"
#include "HardwareConfig.h"

#include "Util/Global.h"
#include "Util/Log/printf.h"
#include "Util/Log/LogOutput.h"

#include "UARTModule.h"
#include "ButtonModule.h"
#include "LEDModule.h"
#include "DisplayModule.h"
#include "ADCModule.h"
#include "TimerModule.h"
#include "GlobalObjects.h"

#include "Auth/Authfunc.h"

/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();

/***** MAIN FUNCTION *********************************************************/
int main(void) {
	// Initialize the HAL
	HAL_Init();

	SystemClock_Config();

	// Initialize Peripherals
	initializePeripherals();

	while (1) {
		Auth_Process();
	}

}

/***** PRIVATE FUNCTIONS *****************************************************/
static int32_t initializePeripherals() {
	// Initialize UART used for Debug-Outputs
	uartInitialize(115200);

	// Initialize GPIOs for LED and 7-Segment output
	ledInitialize();
	displayInitialize();

	// Initialize GPIOs for Buttons
	buttonInitialize();

	// Initialize Timer, DMA and ADC for sensor measurements
	timerInitialize();
	adcInitialize();

	return ERROR_OK;
}
