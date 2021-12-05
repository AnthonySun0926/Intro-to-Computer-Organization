        lw  0   1   index
        add 1   1   2
        nor 1   2   3
        lw  0   2   index
        add 1   1   3
        sw  1   3   2
        add 3   3   3
        lw  2   3   index
        sw  3   4   1
        add 1   2   3
        halt
index   .fill   1
