        sw      1       2       -1
six     lw      1       2       six
start   add     1       2       1
        nor     1       2       1
        beq     0       1       2
        beq     0       0       start
        noop                    end
        jalr    1       12      
done    halt
five    .fill   5
neg1    .fill   -1
stAddr  .fill   start
