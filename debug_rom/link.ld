OUTPUT_ARCH( "riscv" )
ENTRY( entry )
SECTIONS
{
    . = 0xc0780000;
    .whereto 0xc0780300 :
    {
        *(.whereto)
    }   
    . = 0xc0780800;
    .text :
    {
        *(.text)
    }
    _end = .;
}
