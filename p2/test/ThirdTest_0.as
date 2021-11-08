      lw   0  1  First  local defined global label
      lw   0  1  Glob1  other place defined global label
four  beq  0  1  4
      beq  0  0  GlobD  local defined global label
Five  add  1  1  1
      sw   0  1  GlobF  other place defined global label
GlobD nor  1  1  1
      jalr 4  6
      halt
First .fill GlobF
One   .fill Glob1
local .fill 1
      .fill four
      .fill local
      .fill GlobD
      .fill Stack
      .fill First
