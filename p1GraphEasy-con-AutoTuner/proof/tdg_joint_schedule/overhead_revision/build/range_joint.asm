
proof/tdg_joint_schedule/overhead_revision/build/probe_joint:     file format elf64-x86-64


Disassembly of section .init:

Disassembly of section .plt:

Disassembly of section .plt.got:

Disassembly of section .plt.sec:

Disassembly of section .text:

0000000000006c10 <range>:
    6c10:	f3 0f 1e fa          	endbr64
    6c14:	48 85 f6             	test   %rsi,%rsi
    6c17:	0f 8e 86 00 00 00    	jle    6ca3 <range+0x93>
    6c1d:	c5 7b 10 05 bb ec 01 	vmovsd 0x1ecbb(%rip),%xmm8        # 258e0 <__PRETTY_FUNCTION__.1+0x10>
    6c24:	00 
    6c25:	44 8b 41 08          	mov    0x8(%rcx),%r8d
    6c29:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
    6c2d:	c5 fb 10 3d b3 ec 01 	vmovsd 0x1ecb3(%rip),%xmm7        # 258e8 <__PRETTY_FUNCTION__.1+0x18>
    6c34:	00 
    6c35:	c5 fb 10 35 9b ec 01 	vmovsd 0x1ec9b(%rip),%xmm6        # 258d8 <__PRETTY_FUNCTION__.1+0x8>
    6c3c:	00 
    6c3d:	c5 fb 10 2d ab ec 01 	vmovsd 0x1ecab(%rip),%xmm5        # 258f0 <__PRETTY_FUNCTION__.1+0x20>
    6c44:	00 
    6c45:	4c 8b 49 20          	mov    0x20(%rcx),%r9
    6c49:	48 8d 4f 01          	lea    0x1(%rdi),%rcx
    6c4d:	31 ff                	xor    %edi,%edi
    6c4f:	90                   	nop
    6c50:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
    6c55:	c4 c1 73 59 c8       	vmulsd %xmm8,%xmm1,%xmm1
    6c5a:	c5 f3 59 d7          	vmulsd %xmm7,%xmm1,%xmm2
    6c5e:	45 85 c0             	test   %r8d,%r8d
    6c61:	7e 2d                	jle    6c90 <range+0x80>
    6c63:	31 c0                	xor    %eax,%eax
    6c65:	0f 1f 00             	nopl   (%rax)
    6c68:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
    6c6c:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
    6c70:	83 c0 01             	add    $0x1,%eax
    6c73:	c5 fb 58 c6          	vaddsd %xmm6,%xmm0,%xmm0
    6c77:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
    6c7b:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
    6c7f:	c5 f3 59 cd          	vmulsd %xmm5,%xmm1,%xmm1
    6c83:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
    6c87:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
    6c8b:	44 39 c0             	cmp    %r8d,%eax
    6c8e:	75 d8                	jne    6c68 <range+0x58>
    6c90:	48 83 c7 01          	add    $0x1,%rdi
    6c94:	c4 c1 7b 11 54 c9 f8 	vmovsd %xmm2,-0x8(%r9,%rcx,8)
    6c9b:	48 01 d1             	add    %rdx,%rcx
    6c9e:	48 39 fe             	cmp    %rdi,%rsi
    6ca1:	75 ad                	jne    6c50 <range+0x40>
    6ca3:	c3                   	ret

Disassembly of section .fini:
