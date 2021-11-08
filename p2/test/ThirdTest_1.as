    lw  0   1   First
    lw  0   1   Glob1
Glob1   beq 0   1   4
    beq 0   1   Glob1
    lw  0   3   local
local   sw  0   1   GlobD
    sw  0   2   Stack   Stack defined in T
GlobF   .fill   5
        .fill   Stack   Stack defined in D
data    .fill   GlobF
        .fill   GlobD
        .fill   Glob1
        .fill   local
        .fill   data
