/******************************************************************************
 * @file UARTModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header File for UART module
 *
 *
 *****************************************************************************/
#ifndef _UART_MODULE_H_
#define _UART_MODULE_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define UART_ERR_OK                  0          //!< No error occured
#define UART_ERR_INIT_FAILURE        -1         //!< Error during UART initialization
#define UART_ERR_TRANSMIT            -2         //!< Error during UART tranmission
#define UART_ERR_RECEIVE             -3         //!< Error during UART receive


/***** TYPES *****************************************************************/


/***** PROTOTYPES ************************************************************/


/**
 * @brief Initializes the UART2 peripheral to the specified baudrate
 *
 * Additionally, the communication parameter are set to 8 data bit,
 * 1 stop bit and none parity
 *
 * @param baudrate Baudrate to setup the UART to
 *
 * @return Returns UART_ERR_OK if no error occured
 */
int32_t uartInitialize(uint32_t baudrate);

/**
 * @brief Sends data to the UART interface
 *
 * @param pDataBuffer Pointer to the data buffer which should be send out
 * @param bufferLength Length of the buffer (number of bytes) to send
 *
 * @return Returns UART_ERR_OK if no error occured, otherwise UART_ERR_TRANSMIT
 */
int32_t uartSendData(uint8_t* pDataBuffer, int32_t bufferLength);

/**
 * @brief Receives data from the UART interface
 *
 * @param pDataBuffer Pointer to the data buffer which is used to store the recevied bytes
 * @param bufferLength Length of the buffer (number of bytes)
 *
 * @return Returns UART_ERR_OK if no error occured, otherwise UART_ERR_RECEIVE
 */
int32_t uartReceiveData(uint8_t* pDataBuffer, int32_t bufferLength);

/**
 * @brief Checks for available data in the UART RX buffer
 *
 * @param pHasData	Pointer to store flag whether data is available.
 * 					0 = no data available
 * 					1 = data available
 *
 * 	@return Returns UART_ERR_OK if no error occured, otherwise UART_ERR_RECEIVE
 */
int32_t uartHasData(int8_t* pHasData);

#endif
