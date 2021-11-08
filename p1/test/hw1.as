 		lw  0   1   ct 	comment
		lw  0	2	limit
		lw	0	3	one
loop	beq 1	2   done
		add	1	3	1	(incrementing)
		beq 0   0   loop
done	noop 13	(Think about what happens to the 13!)
		sw	0	1	1000
		halt 		yet another comment
		jalr    4   7   3 (Think about how that 3 is treated)
ct	.fill	0
limit	.fill	7
one 	.fill	1
