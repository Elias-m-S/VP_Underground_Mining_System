/******************************************************************************
 * @file FaultHandler.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief efault Fault-Handler for VP Project
 *
 * @details Usually those handler only contain an endless loop to keep the
 * controller stuck in the fault handler
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/


/**
 * @brief Default-Implementation of the HardFault Handler
 *
 * According Programming Manual:
 * A hard fault is an exception that occurs because of an error during
 * exception processing, or because an exception cannot be managed
 * by any other exception mechanism. Hard faults have a fixed priority
 * of -1, meaning they have higher priority than any exception with
 * configurable priority.
 *
 */
void HardFault_Handler(void)
{
  while (1) {}
}

/**
 * @brief Default-Implementation of the MemManage Handler
 *
 * According Programming Manual:
 * memory management fault is an exception that occurs because of
 * a memory protection related fault. The fixed memory protection
 * constraints determines this fault, for both instruction and data
 * memory transactions. This fault is used to abort instruction accesses
 * to Execute Never (XN) memory regions.
 *
 */
void MemManage_Handler(void)
{
  while (1) {}
}

/**
 * @brief Default-Implementation of the BusFault Handler
 *
 * According Programming Manual
 * A bus fault is an exception that occurs because of a memory related
 * fault for an instruction or data memory transaction. This might be
 * from an error detected on a bus in the memory system.
 *
 */
void BusFault_Handler(void)
{
  while (1) {}
}

/**
 * @brief Default-Implementation of the UsageFault Handler
 *
 * According Programming Manual:
 * A usage fault is an exception that occurs because of a fault related
 * to instruction execution. This includes:
 *   • An undefined instruction
 *   • An illegal unaligned access
 *   • Invalid state on instruction execution
 *   • An error on exception return.
 *
 * The following can cause a usage fault when the core is configured to
 * report them:
 *   • An unaligned address on word and halfword memory access
 *   • Division by zero
 *
 */
void UsageFault_Handler(void)
{
  while (1) {}
}

/***** PRIVATE FUNCTIONS *****************************************************/
