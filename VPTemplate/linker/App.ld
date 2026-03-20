/******************************************************************************
 * @file VP.ld
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Linker file for VPTemplate Auth Project Target system STM32G474
 *
 *
 *****************************************************************************/

/*
 * Sets the entry point for the executable
 * This is only necessary for ELF binaries which are executed on systems like
 * Linux or Windows. For embedded targets this only helps to provide the linker
 * the information what the entry point is.
 *
 * The entry point in this case is the Reset_Handler function which is defined
 * in the startup.s file. The Reset_Handler is responsible for setting up a
 * correct C-Environemt and call the main() function.
 *
 */
ENTRY(StartApp_Handler)

/*
 * Specifies the memory areas for the target system.
 * The current configuration only provides the two main memories FLASH and RAM.
 * Hereby there is no differentiation betwenn the different SRAM banks of the controller
 *
 */
MEMORY
{
  RAM    (rw)      : ORIGIN = 0x20004000,  LENGTH = 64K
  FLASH  (rx)      : ORIGIN = 0x08010000,  LENGTH = 128K
}

/***** GLOBAL SYMBOLS ********************************************************/

/* Symbols used for memory management
 * This includes the stack and also a possible heap memory
 */
_size_of_stack = 0x1000;                             /* Size of stack: 4 KiB */
_size_of_heap  = 0x0;                               /* Size of heap in bytes */

/* Application stack starts at the top of its RAM region (0x20014000 - 4) */
_end_of_ram = ORIGIN(RAM) + LENGTH(RAM) - 4;
_initial_stack_pointer = _end_of_ram;

/***** SECTIONS **************************************************************/

/* Define output sections */
SECTIONS
{

.signature  :
  {
    . = ALIGN(4);                       
    KEEP(*(.signature))
    . = ALIGN(4);
  } >FLASH

  
  /* Section for Interrupt Vector Table */
  .isr_vector ALIGN(512):
  {
    . = ALIGN(4);
    /* Include the .isr_vector section which must include the vector interrupt table */
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* .text Section to include program code */
  .text :
  {
    . = ALIGN(4);
    _stext = .;         /* Define global symbol for start of code */

    *(.text)            /* .text sections (code) */
    *(.text*)           /* .text* sections (code) */

    . = ALIGN(4);

    _etext = .;         /* Define a global symbols at end of code */
  } >FLASH

  /* Section for read-only data (stored in flash) */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH


  /* Define a symbol for the load address of the initialized data
   * This address is located in FLASH
   */
  _sloaddata = LOADADDR(.data);

  /* Initialized data section */
  .data :
  {
    . = ALIGN(4);

    _sdata = .;        /* create a global symbol at data start */

    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);

    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD):
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */

    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */

  } >RAM

  .heap (NOLOAD):
  {
    . = ALIGN(4);

    _start_of_heap = .;
    . += _size_of_heap;

    . = ALIGN(4);

     _end_of_heap = .;
  } >RAM
  
    .stack (NOLOAD):
  {
    . = ORIGIN(RAM) + LENGTH(RAM) - _size_of_stack;
    . = ALIGN(8);

    _stack_start = .;
    . += _size_of_stack;
    . = ALIGN(8);
    _stack_end = .;
  } >RAM

  _initial_stack_pointer = _stack_end;

  ASSERT(_end_of_heap <= _stack_start, "RAM overflow: .heap/.bss/.data overlap with .stack")


  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
