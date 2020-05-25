OUTPUT_ARCH( "riscv" )
ENTRY( entry )
SECTIONS
{
    . = 0xc0500000;
    .whereto 0xc0500300 :
    {
        *(.whereto)
    }   
    . = 0xc0500800;
    .text :
    {
        *(.text)
    }
    _end = .;
}
