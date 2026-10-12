
proof/tdg_joint_schedule/overhead_revision/build/probe_reference:     file format elf64-x86-64


Disassembly of section .init:

Disassembly of section .plt:

Disassembly of section .plt.got:

Disassembly of section .plt.sec:

Disassembly of section .text:

00000000000054b0 <range>:
    54b0:	f3 0f 1e fa          	endbr64
    54b4:	48 85 f6             	test   %rsi,%rsi
    54b7:	0f 8e 86 00 00 00    	jle    5543 <range+0x93>
    54bd:	c5 7b 10 05 7b 72 01 	vmovsd 0x1727b(%rip),%xmm8        # 1c740 <__PRETTY_FUNCTION__.1+0x10>
    54c4:	00 
    54c5:	44 8b 41 08          	mov    0x8(%rcx),%r8d
    54c9:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
    54cd:	c5 fb 10 3d 73 72 01 	vmovsd 0x17273(%rip),%xmm7        # 1c748 <__PRETTY_FUNCTION__.1+0x18>
    54d4:	00 
    54d5:	c5 fb 10 35 5b 72 01 	vmovsd 0x1725b(%rip),%xmm6        # 1c738 <__PRETTY_FUNCTION__.1+0x8>
    54dc:	00 
    54dd:	c5 fb 10 2d 6b 72 01 	vmovsd 0x1726b(%rip),%xmm5        # 1c750 <__PRETTY_FUNCTION__.1+0x20>
    54e4:	00 
    54e5:	4c 8b 49 20          	mov    0x20(%rcx),%r9
    54e9:	48 8d 4f 01          	lea    0x1(%rdi),%rcx
    54ed:	31 ff                	xor    %edi,%edi
    54ef:	90                   	nop
    54f0:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
    54f5:	c4 c1 73 59 c8       	vmulsd %xmm8,%xmm1,%xmm1
    54fa:	c5 f3 59 d7          	vmulsd %xmm7,%xmm1,%xmm2
    54fe:	45 85 c0             	test   %r8d,%r8d
    5501:	7e 2d                	jle    5530 <range+0x80>
    5503:	31 c0                	xor    %eax,%eax
    5505:	0f 1f 00             	nopl   (%rax)
    5508:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
    550c:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
    5510:	83 c0 01             	add    $0x1,%eax
    5513:	c5 fb 58 c6          	vaddsd %xmm6,%xmm0,%xmm0
    5517:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
    551b:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
    551f:	c5 f3 59 cd          	vmulsd %xmm5,%xmm1,%xmm1
    5523:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
    5527:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
    552b:	44 39 c0             	cmp    %r8d,%eax
    552e:	75 d8                	jne    5508 <range+0x58>
    5530:	48 83 c7 01          	add    $0x1,%rdi
    5534:	c4 c1 7b 11 54 c9 f8 	vmovsd %xmm2,-0x8(%r9,%rcx,8)
    553b:	48 01 d1             	add    %rdx,%rcx
    553e:	48 39 fe             	cmp    %rdi,%rsi
    5541:	75 ad                	jne    54f0 <range+0x40>
    5543:	c3                   	ret

Disassembly of section .fini:
