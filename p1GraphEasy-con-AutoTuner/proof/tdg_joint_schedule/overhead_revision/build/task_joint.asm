
proof/tdg_joint_schedule/overhead_revision/build/probe_joint:     file format elf64-x86-64


Disassembly of section .init:

Disassembly of section .plt:

Disassembly of section .plt.got:

Disassembly of section .plt.sec:

Disassembly of section .text:

000000000001c960 <task>:
   1c960:	f3 0f 1e fa          	endbr64
   1c964:	41 57                	push   %r15
   1c966:	41 56                	push   %r14
   1c968:	41 55                	push   %r13
   1c96a:	41 54                	push   %r12
   1c96c:	55                   	push   %rbp
   1c96d:	53                   	push   %rbx
   1c96e:	48 89 fb             	mov    %rdi,%rbx
   1c971:	48 83 ec 28          	sub    $0x28,%rsp
   1c975:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
   1c97c:	00 00 
   1c97e:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
   1c983:	31 c0                	xor    %eax,%eax
   1c985:	48 89 e5             	mov    %rsp,%rbp
   1c988:	e8 93 5c fe ff       	call   2620 <pthread_self@plt>
   1c98d:	83 83 90 00 00 00 01 	addl   $0x1,0x90(%rbx)
   1c994:	48 89 ee             	mov    %rbp,%rsi
   1c997:	bf 01 00 00 00       	mov    $0x1,%edi
   1c99c:	48 89 43 78          	mov    %rax,0x78(%rbx)
   1c9a0:	e8 9b 5c fe ff       	call   2640 <clock_gettime@plt>
   1c9a5:	48 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%rax
   1c9ac:	3b 
   1c9ad:	48 03 44 24 08       	add    0x8(%rsp),%rax
   1c9b2:	48 89 83 80 00 00 00 	mov    %rax,0x80(%rbx)
   1c9b9:	64 8b 04 25 a0 ff ff 	mov    %fs:0xffffffffffffffa0,%eax
   1c9c0:	ff 
   1c9c1:	89 43 10             	mov    %eax,0x10(%rbx)
   1c9c4:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
   1c9c9:	64 48 8b 14 25 58 ff 	mov    %fs:0xffffffffffffff58,%rdx
   1c9d0:	ff ff 
   1c9d2:	48 85 d2             	test   %rdx,%rdx
   1c9d5:	74 03                	je     1c9da <task+0x7a>
   1c9d7:	8b 42 08             	mov    0x8(%rdx),%eax
   1c9da:	8b 4b 04             	mov    0x4(%rbx),%ecx
   1c9dd:	89 43 14             	mov    %eax,0x14(%rbx)
   1c9e0:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
   1c9e4:	85 c9                	test   %ecx,%ecx
   1c9e6:	0f 85 d5 00 00 00    	jne    1cac1 <task+0x161>
   1c9ec:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   1c9f0:	48 85 f6             	test   %rsi,%rsi
   1c9f3:	7e 7e                	jle    1ca73 <task+0x113>
   1c9f5:	8b 53 08             	mov    0x8(%rbx),%edx
   1c9f8:	48 8b 7b 20          	mov    0x20(%rbx),%rdi
   1c9fc:	31 c9                	xor    %ecx,%ecx
   1c9fe:	c5 fb 10 35 da 8e 00 	vmovsd 0x8eda(%rip),%xmm6        # 258e0 <__PRETTY_FUNCTION__.1+0x10>
   1ca05:	00 
   1ca06:	c5 fb 10 2d da 8e 00 	vmovsd 0x8eda(%rip),%xmm5        # 258e8 <__PRETTY_FUNCTION__.1+0x18>
   1ca0d:	00 
   1ca0e:	c5 7b 10 05 c2 8e 00 	vmovsd 0x8ec2(%rip),%xmm8        # 258d8 <__PRETTY_FUNCTION__.1+0x8>
   1ca15:	00 
   1ca16:	c5 fb 10 3d d2 8e 00 	vmovsd 0x8ed2(%rip),%xmm7        # 258f0 <__PRETTY_FUNCTION__.1+0x20>
   1ca1d:	00 
   1ca1e:	66 90                	xchg   %ax,%ax
   1ca20:	48 83 c1 01          	add    $0x1,%rcx
   1ca24:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
   1ca29:	c5 f3 59 ce          	vmulsd %xmm6,%xmm1,%xmm1
   1ca2d:	c5 f3 59 d5          	vmulsd %xmm5,%xmm1,%xmm2
   1ca31:	85 d2                	test   %edx,%edx
   1ca33:	7e 33                	jle    1ca68 <task+0x108>
   1ca35:	31 c0                	xor    %eax,%eax
   1ca37:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
   1ca3e:	00 00 
   1ca40:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
   1ca44:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
   1ca48:	83 c0 01             	add    $0x1,%eax
   1ca4b:	c4 c1 7b 58 c0       	vaddsd %xmm8,%xmm0,%xmm0
   1ca50:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
   1ca54:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
   1ca58:	c5 f3 59 cf          	vmulsd %xmm7,%xmm1,%xmm1
   1ca5c:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
   1ca60:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
   1ca64:	39 d0                	cmp    %edx,%eax
   1ca66:	75 d8                	jne    1ca40 <task+0xe0>
   1ca68:	c5 fb 11 54 cf f8    	vmovsd %xmm2,-0x8(%rdi,%rcx,8)
   1ca6e:	48 39 ce             	cmp    %rcx,%rsi
   1ca71:	75 ad                	jne    1ca20 <task+0xc0>
   1ca73:	f0 83 05 4d d6 00 00 	lock addl $0x1,0xd64d(%rip)        # 2a0c8 <ordinary_done>
   1ca7a:	01 
   1ca7b:	48 89 ee             	mov    %rbp,%rsi
   1ca7e:	bf 01 00 00 00       	mov    $0x1,%edi
   1ca83:	e8 b8 5b fe ff       	call   2640 <clock_gettime@plt>
   1ca88:	48 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%rax
   1ca8f:	3b 
   1ca90:	48 03 44 24 08       	add    0x8(%rsp),%rax
   1ca95:	48 89 83 88 00 00 00 	mov    %rax,0x88(%rbx)
   1ca9c:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
   1caa1:	64 48 2b 04 25 28 00 	sub    %fs:0x28,%rax
   1caa8:	00 00 
   1caaa:	0f 85 bf 01 00 00    	jne    1cc6f <task+0x30f>
   1cab0:	48 83 c4 28          	add    $0x28,%rsp
   1cab4:	31 c0                	xor    %eax,%eax
   1cab6:	5b                   	pop    %rbx
   1cab7:	5d                   	pop    %rbp
   1cab8:	41 5c                	pop    %r12
   1caba:	41 5d                	pop    %r13
   1cabc:	41 5e                	pop    %r14
   1cabe:	41 5f                	pop    %r15
   1cac0:	c3                   	ret
   1cac1:	8b 05 01 d6 00 00    	mov    0xd601(%rip),%eax        # 2a0c8 <ordinary_done>
   1cac7:	3b 05 03 d6 00 00    	cmp    0xd603(%rip),%eax        # 2a0d0 <ordinary_count>
   1cacd:	0f 8c 75 01 00 00    	jl     1cc48 <task+0x2e8>
   1cad3:	8b 7b 28             	mov    0x28(%rbx),%edi
   1cad6:	4c 8d 6b 28          	lea    0x28(%rbx),%r13
   1cada:	e8 91 a5 fe ff       	call   7070 <sgpl_loop_effective_decision_threads_for_loop_id>
   1cadf:	44 8b 63 2c          	mov    0x2c(%rbx),%r12d
   1cae3:	48 8b 53 18          	mov    0x18(%rbx),%rdx
   1cae7:	31 f6                	xor    %esi,%esi
   1cae9:	89 43 0c             	mov    %eax,0xc(%rbx)
   1caec:	b9 01 00 00 00       	mov    $0x1,%ecx
   1caf1:	4c 89 ef             	mov    %r13,%rdi
   1caf4:	41 83 fc 01          	cmp    $0x1,%r12d
   1caf8:	0f 84 40 01 00 00    	je     1cc3e <task+0x2de>
   1cafe:	e8 3d b2 ff ff       	call   17d40 <sgpl_should_parallelize_doall>
   1cb03:	85 c0                	test   %eax,%eax
   1cb05:	0f 85 e6 00 00 00    	jne    1cbf1 <task+0x291>
   1cb0b:	c7 43 0c 01 00 00 00 	movl   $0x1,0xc(%rbx)
   1cb12:	48 89 ee             	mov    %rbp,%rsi
   1cb15:	bf 01 00 00 00       	mov    $0x1,%edi
   1cb1a:	e8 21 5b fe ff       	call   2640 <clock_gettime@plt>
   1cb1f:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   1cb23:	4c 8b 7c 24 08       	mov    0x8(%rsp),%r15
   1cb28:	4c 69 34 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%r14
   1cb2f:	3b 
   1cb30:	48 85 f6             	test   %rsi,%rsi
   1cb33:	7e 7e                	jle    1cbb3 <task+0x253>
   1cb35:	8b 53 08             	mov    0x8(%rbx),%edx
   1cb38:	48 8b 7b 20          	mov    0x20(%rbx),%rdi
   1cb3c:	31 c9                	xor    %ecx,%ecx
   1cb3e:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
   1cb42:	c5 fb 10 35 96 8d 00 	vmovsd 0x8d96(%rip),%xmm6        # 258e0 <__PRETTY_FUNCTION__.1+0x10>
   1cb49:	00 
   1cb4a:	c5 fb 10 2d 96 8d 00 	vmovsd 0x8d96(%rip),%xmm5        # 258e8 <__PRETTY_FUNCTION__.1+0x18>
   1cb51:	00 
   1cb52:	c5 7b 10 05 7e 8d 00 	vmovsd 0x8d7e(%rip),%xmm8        # 258d8 <__PRETTY_FUNCTION__.1+0x8>
   1cb59:	00 
   1cb5a:	c5 fb 10 3d 8e 8d 00 	vmovsd 0x8d8e(%rip),%xmm7        # 258f0 <__PRETTY_FUNCTION__.1+0x20>
   1cb61:	00 
   1cb62:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
   1cb68:	48 83 c1 01          	add    $0x1,%rcx
   1cb6c:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
   1cb71:	c5 f3 59 ce          	vmulsd %xmm6,%xmm1,%xmm1
   1cb75:	c5 f3 59 d5          	vmulsd %xmm5,%xmm1,%xmm2
   1cb79:	85 d2                	test   %edx,%edx
   1cb7b:	7e 2b                	jle    1cba8 <task+0x248>
   1cb7d:	31 c0                	xor    %eax,%eax
   1cb7f:	90                   	nop
   1cb80:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
   1cb84:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
   1cb88:	83 c0 01             	add    $0x1,%eax
   1cb8b:	c4 c1 7b 58 c0       	vaddsd %xmm8,%xmm0,%xmm0
   1cb90:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
   1cb94:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
   1cb98:	c5 f3 59 cf          	vmulsd %xmm7,%xmm1,%xmm1
   1cb9c:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
   1cba0:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
   1cba4:	39 c2                	cmp    %eax,%edx
   1cba6:	75 d8                	jne    1cb80 <task+0x220>
   1cba8:	c5 fb 11 54 cf f8    	vmovsd %xmm2,-0x8(%rdi,%rcx,8)
   1cbae:	48 39 ce             	cmp    %rcx,%rsi
   1cbb1:	75 b5                	jne    1cb68 <task+0x208>
   1cbb3:	48 89 ee             	mov    %rbp,%rsi
   1cbb6:	bf 01 00 00 00       	mov    $0x1,%edi
   1cbbb:	e8 80 5a fe ff       	call   2640 <clock_gettime@plt>
   1cbc0:	4c 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%r8
   1cbc7:	3b 
   1cbc8:	48 8b 53 18          	mov    0x18(%rbx),%rdx
   1cbcc:	4d 29 f8             	sub    %r15,%r8
   1cbcf:	4c 03 44 24 08       	add    0x8(%rsp),%r8
   1cbd4:	4d 29 f0             	sub    %r14,%r8
   1cbd7:	41 83 fc 01          	cmp    $0x1,%r12d
   1cbdb:	74 78                	je     1cc55 <task+0x2f5>
   1cbdd:	b9 01 00 00 00       	mov    $0x1,%ecx
   1cbe2:	31 f6                	xor    %esi,%esi
   1cbe4:	4c 89 ef             	mov    %r13,%rdi
   1cbe7:	e8 b4 c3 ff ff       	call   18fa0 <sgpl_record_doall_serial_sample>
   1cbec:	e9 8a fe ff ff       	jmp    1ca7b <task+0x11b>
   1cbf1:	8b 43 28             	mov    0x28(%rbx),%eax
   1cbf4:	45 31 c9             	xor    %r9d,%r9d
   1cbf7:	41 83 fc 01          	cmp    $0x1,%r12d
   1cbfb:	ba 01 00 00 00       	mov    $0x1,%edx
   1cc00:	41 0f 94 c1          	sete   %r9b
   1cc04:	48 83 ec 08          	sub    $0x8,%rsp
   1cc08:	49 89 d8             	mov    %rbx,%r8
   1cc0b:	31 ff                	xor    %edi,%edi
   1cc0d:	64 89 04 25 e4 db ff 	mov    %eax,%fs:0xffffffffffffdbe4
   1cc14:	ff 
   1cc15:	48 8d 05 f4 9f fe ff 	lea    -0x1600c(%rip),%rax        # 6c10 <range>
   1cc1c:	48 8d 0d 7d 7b fe ff 	lea    -0x18483(%rip),%rcx        # 47a0 <body>
   1cc23:	64 48 89 04 25 98 ff 	mov    %rax,%fs:0xffffffffffffff98
   1cc2a:	ff ff 
   1cc2c:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   1cc30:	41 51                	push   %r9
   1cc32:	e8 c9 f4 ff ff       	call   1c100 <parallel_for_runtime>
   1cc37:	58                   	pop    %rax
   1cc38:	5a                   	pop    %rdx
   1cc39:	e9 3d fe ff ff       	jmp    1ca7b <task+0x11b>
   1cc3e:	e8 5d e4 ff ff       	call   1b0a0 <sgpl_should_parallelize_doacross>
   1cc43:	e9 bb fe ff ff       	jmp    1cb03 <task+0x1a3>
   1cc48:	f0 83 05 74 d4 00 00 	lock addl $0x1,0xd474(%rip)        # 2a0c4 <early_loops>
   1cc4f:	01 
   1cc50:	e9 7e fe ff ff       	jmp    1cad3 <task+0x173>
   1cc55:	4d 89 c1             	mov    %r8,%r9
   1cc58:	b9 01 00 00 00       	mov    $0x1,%ecx
   1cc5d:	45 31 c0             	xor    %r8d,%r8d
   1cc60:	31 f6                	xor    %esi,%esi
   1cc62:	4c 89 ef             	mov    %r13,%rdi
   1cc65:	e8 06 cf ff ff       	call   19b70 <sgpl_record_doacross_serial_sample>
   1cc6a:	e9 0c fe ff ff       	jmp    1ca7b <task+0x11b>
   1cc6f:	e8 6c 5a fe ff       	call   26e0 <__stack_chk_fail@plt>

Disassembly of section .fini:
