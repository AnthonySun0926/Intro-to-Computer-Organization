        lw      0       1       one     reg1 = 1
        lw      0       2       eight   reg2 = 8
        lw      0       3       ten     reg3 = 10
        add     1       2       2       increment reg2
        nor     1       2       5       reg5 = reg1 nor reg2
        sw      0       5       3       mem[3] = reg5
loop    add     1       2       2       increment reg2   
        jalr    3       2               reg2 = 8;branch to #10
        beq     0       0       loop    loop
        add     1       2       2       increment reg2
        add     1       2       2       increment reg2
no      noop
        halt
one     .fill   1
eight   .fill   8
ten     .fill   10
