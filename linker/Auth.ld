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
 * @brief Linker file for VPTemplate Application Project Target system STM32G474
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
ENTRY(Reset_Handler)

/*
 * Specifies the memory areas for the target system.
 * The current configuration only provides the two main memories FLASH and RAM.
 * Hereby there is no differentiation betwenn the different SRAM banks of the controller
 *
 */
MEMORY
{
  /* RAM Memory Region incl. SRAM1 and SRAM2 */
  RAM    (rw)      : ORIGIN = 0x20000000,  LENGTH = 128K
  /* FLASH Memory Region using mapped address 0x08000000. */
  FLASH  (rx)      : ORIGIN = 0x08000000,  LENGTH = 512K
}

/***** GLOBAL SYMBOLS ********************************************************/

/* Symbols used for memory management
 * This includes the stack and also a possible heap memory
 */
_size_of_stack = 0x200;                             /* Size of stack in bytes */
_size_of_heap  = 0x0;                               /* Size of heap in bytes */

/* Highest usable address of the RAM. */
_end_of_ram = ORIGIN(RAM) + LENGTH(RAM) - 4;        /* End of RAM */
_initial_stack_pointer = _end_of_ram;               /* Set the symbol for initial stack pointer */

/***** SECTIONS **************************************************************/

/* Define output sections */
SECTIONS
{
  /* Section for Interrupt Vector Table */
  .isr_vector :
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

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
