# Sam's cancer map format

    aa bb cc
    dd ee ff
    gg hh ii

`aa` = ceiling texture

`gg` = floor texture

`bb` = top wall texture  
`dd` = left wall texture  
`ff` = right wall texture  
`hh` = bottom wall texture

Textures are shortnames (usually 2 chars long) and are defined in a mapping at the top of the `texture_store.cpp` file.

`ee` = sprite texture/player position

The player position is specified with `@c` (where `c` is a cardinal direction `n s w e`).

`cc` = Top texture in partial height mode

`ii` = Height of the block in hex (`00`-`FF`)  
`FF` is the default, `00` means fully high (this makes no sense, blame isard)