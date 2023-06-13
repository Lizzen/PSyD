ENTRY( main )
SECTIONS
{
    . = 0x0c100000;
    .text : { * (.text) } 
    .rodata : { *(.rodata) } 
    .data : { *(.data) } 
    .bss : { *(.bss) }     
    end = .;
}
