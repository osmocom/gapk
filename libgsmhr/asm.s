
.globl sat_adds32b
.type   sat_adds32b,@function
sat_adds32b:
	mov    %edi, %eax
	shr    $0x1f, %edi
	add    $0x7fffffff, %edi
	add    %esi, %eax
	cmovo  %edi, %eax
	retq
.size	sat_adds32b, .-sat_adds32b

.globl sat_subs32b
.type   sat_subs32b,@function
sat_subs32b:
	mov    %edi, %eax
	shr    $0x1f, %edi
	add    $0x7fffffff, %edi
	sub    %esi, %eax
	cmovo  %edi, %eax
	retq
.size	sat_subs32b, .-sat_subs32b

.globl sat_divs32b
.type   sat_divs32b,@function
sat_divs32b:
	mov    %edi, %eax
	lea    0x1(%rsi), %edx
	add    $0x80000000, %edi
	or     %edx, %edi
	cdq
	neg    %edi
	sbb    $-1, %eax
	idiv   %esi
	retq
.size	sat_divs32b, .-sat_divs32b

.globl sat_muls32b
.type   sat_muls32b,@function
sat_muls32b:
	mov    %edi, %eax
	xor    %esi, %edi
	shr    $0x1f, %edi
	add    $0x7fffffff, %edi
	imul   %esi
	cmovc  %edi, %eax
	retq
.size	sat_muls32b, .-sat_muls32b

