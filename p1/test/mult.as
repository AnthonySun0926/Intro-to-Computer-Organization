        lw      0       1       zero
        lw      0       2       mcand
        lw      0       3       mplier
        lw      0       4       one
        lw      0       6       digend
loop    beq     4       6       end
        nor     3       3       5
        nor     4       4       7
        nor     5       7       5
        beq     5       0       shift
        add     1       2       1
shift   add     4       4       4
        add     2       2       2
        beq     0       0       loop
end     halt
mcand   .fill   1103
mplier  .fill   7043
zero    .fill   0
one     .fill   1
fiften  .fill   15
digend  .fill   65536
