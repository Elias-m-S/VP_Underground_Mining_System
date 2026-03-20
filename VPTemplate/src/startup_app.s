/******************************************************************************
 * @file startup.s
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Startup Code for VPTemplate Application Project
 *
 *
 *****************************************************************************/

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section .text.StartApp_Handler
.type StartApp_Handler, %function
.global StartApp_Handler
StartApp_Handler:
    /* Copy the data segment initializers from flash to SRAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sloaddata
    movs r3, #0
    b .loopCopyData

.copyData:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

.loopCopyData:
    adds r4, r0, r3
    cmp r4, r1
    bcc .copyData

    /* Zero fill the bss segment. */
    ldr r2, =_sbss
    ldr r4, =_ebss
    movs r3, #0
    b .loopFillZerobss

.fillZerobss:
    str  r3, [r2]
    adds r2, r2, #4

.loopFillZerobss:
    cmp r2, r4
    bcc .fillZerobss

    /* Fill stack section with known pattern for stack monitoring */
    ldr r0, =_stack_start
    ldr r1, =_stack_end
    ldr r2, =0xA5A5A5A5
    b .loopFillStackCheck

.fillStack:
    str r2, [r0]
    adds r0, r0, #4

.loopFillStackCheck:
    cmp r0, r1
    bcc .fillStack

    /* Initialize the Stack-Pointer */
    ldr r0, =_initial_stack_pointer
    mov sp, r0

    /* Call the clock system intitialization function.*/
    bl SystemInit

    /* Call the application's entry point.*/
    bl main
    bx lr

.size StartApp_Handler, .-StartApp_Handler
