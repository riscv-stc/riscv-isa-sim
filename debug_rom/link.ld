OUTPUT_ARCH( "riscv" )
ENTRY( entry )
SECTIONS
{
    . = 0x40000000;
    .whereto 0x40000300 :
    {
        *(.whereto)
    }   
    . = 0x40000800;
    .text :
    {
        *(.text)
    }
    _end = .;
}
