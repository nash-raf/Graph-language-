
proof/tdg_joint_schedule/overhead_revision/build/probe_reference:     file format elf64-x86-64


Disassembly of section .init:

Disassembly of section .plt:

Disassembly of section .plt.got:

Disassembly of section .plt.sec:

Disassembly of section .text:

0000000000013880 <task>:
   13880:	f3 0f 1e fa          	endbr64
   13884:	41 57                	push   %r15
   13886:	41 56                	push   %r14
   13888:	41 55                	push   %r13
   1388a:	41 54                	push   %r12
   1388c:	55                   	push   %rbp
   1388d:	53                   	push   %rbx
   1388e:	48 89 fb             	mov    %rdi,%rbx
   13891:	48 83 ec 28          	sub    $0x28,%rsp
   13895:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
   1389c:	00 00 
   1389e:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
   138a3:	31 c0                	xor    %eax,%eax
   138a5:	48 89 e5             	mov    %rsp,%rbp
   138a8:	e8 23 ed fe ff       	call   25d0 <pthread_self@plt>
   138ad:	83 83 90 00 00 00 01 	addl   $0x1,0x90(%rbx)
   138b4:	48 89 ee             	mov    %rbp,%rsi
   138b7:	bf 01 00 00 00       	mov    $0x1,%edi
   138bc:	48 89 43 78          	mov    %rax,0x78(%rbx)
   138c0:	e8 2b ed fe ff       	call   25f0 <clock_gettime@plt>
   138c5:	48 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%rax
   138cc:	3b 
   138cd:	48 03 44 24 08       	add    0x8(%rsp),%rax
   138d2:	48 89 83 80 00 00 00 	mov    %rax,0x80(%rbx)
   138d9:	64 8b 04 25 a0 ff ff 	mov    %fs:0xffffffffffffffa0,%eax
   138e0:	ff 
   138e1:	89 43 10             	mov    %eax,0x10(%rbx)
   138e4:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
   138e9:	64 48 8b 14 25 58 ff 	mov    %fs:0xffffffffffffff58,%rdx
   138f0:	ff ff 
   138f2:	48 85 d2             	test   %rdx,%rdx
   138f5:	74 03                	je     138fa <task+0x7a>
   138f7:	8b 42 08             	mov    0x8(%rdx),%eax
   138fa:	8b 4b 04             	mov    0x4(%rbx),%ecx
   138fd:	89 43 14             	mov    %eax,0x14(%rbx)
   13900:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
   13904:	85 c9                	test   %ecx,%ecx
   13906:	0f 85 d5 00 00 00    	jne    139e1 <task+0x161>
   1390c:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   13910:	48 85 f6             	test   %rsi,%rsi
   13913:	7e 7e                	jle    13993 <task+0x113>
   13915:	8b 53 08             	mov    0x8(%rbx),%edx
   13918:	48 8b 7b 20          	mov    0x20(%rbx),%rdi
   1391c:	31 c9                	xor    %ecx,%ecx
   1391e:	c5 fb 10 35 1a 8e 00 	vmovsd 0x8e1a(%rip),%xmm6        # 1c740 <__PRETTY_FUNCTION__.1+0x10>
   13925:	00 
   13926:	c5 fb 10 2d 1a 8e 00 	vmovsd 0x8e1a(%rip),%xmm5        # 1c748 <__PRETTY_FUNCTION__.1+0x18>
   1392d:	00 
   1392e:	c5 7b 10 05 02 8e 00 	vmovsd 0x8e02(%rip),%xmm8        # 1c738 <__PRETTY_FUNCTION__.1+0x8>
   13935:	00 
   13936:	c5 fb 10 3d 12 8e 00 	vmovsd 0x8e12(%rip),%xmm7        # 1c750 <__PRETTY_FUNCTION__.1+0x20>
   1393d:	00 
   1393e:	66 90                	xchg   %ax,%ax
   13940:	48 83 c1 01          	add    $0x1,%rcx
   13944:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
   13949:	c5 f3 59 ce          	vmulsd %xmm6,%xmm1,%xmm1
   1394d:	c5 f3 59 d5          	vmulsd %xmm5,%xmm1,%xmm2
   13951:	85 d2                	test   %edx,%edx
   13953:	7e 33                	jle    13988 <task+0x108>
   13955:	31 c0                	xor    %eax,%eax
   13957:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
   1395e:	00 00 
   13960:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
   13964:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
   13968:	83 c0 01             	add    $0x1,%eax
   1396b:	c4 c1 7b 58 c0       	vaddsd %xmm8,%xmm0,%xmm0
   13970:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
   13974:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
   13978:	c5 f3 59 cf          	vmulsd %xmm7,%xmm1,%xmm1
   1397c:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
   13980:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
   13984:	39 d0                	cmp    %edx,%eax
   13986:	75 d8                	jne    13960 <task+0xe0>
   13988:	c5 fb 11 54 cf f8    	vmovsd %xmm2,-0x8(%rdi,%rcx,8)
   1398e:	48 39 ce             	cmp    %rcx,%rsi
   13991:	75 ad                	jne    13940 <task+0xc0>
   13993:	f0 83 05 2d c7 00 00 	lock addl $0x1,0xc72d(%rip)        # 200c8 <ordinary_done>
   1399a:	01 
   1399b:	48 89 ee             	mov    %rbp,%rsi
   1399e:	bf 01 00 00 00       	mov    $0x1,%edi
   139a3:	e8 48 ec fe ff       	call   25f0 <clock_gettime@plt>
   139a8:	48 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%rax
   139af:	3b 
   139b0:	48 03 44 24 08       	add    0x8(%rsp),%rax
   139b5:	48 89 83 88 00 00 00 	mov    %rax,0x88(%rbx)
   139bc:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
   139c1:	64 48 2b 04 25 28 00 	sub    %fs:0x28,%rax
   139c8:	00 00 
   139ca:	0f 85 bf 01 00 00    	jne    13b8f <task+0x30f>
   139d0:	48 83 c4 28          	add    $0x28,%rsp
   139d4:	31 c0                	xor    %eax,%eax
   139d6:	5b                   	pop    %rbx
   139d7:	5d                   	pop    %rbp
   139d8:	41 5c                	pop    %r12
   139da:	41 5d                	pop    %r13
   139dc:	41 5e                	pop    %r14
   139de:	41 5f                	pop    %r15
   139e0:	c3                   	ret
   139e1:	8b 05 e1 c6 00 00    	mov    0xc6e1(%rip),%eax        # 200c8 <ordinary_done>
   139e7:	3b 05 e3 c6 00 00    	cmp    0xc6e3(%rip),%eax        # 200d0 <ordinary_count>
   139ed:	0f 8c 75 01 00 00    	jl     13b68 <task+0x2e8>
   139f3:	8b 7b 28             	mov    0x28(%rbx),%edi
   139f6:	4c 8d 6b 28          	lea    0x28(%rbx),%r13
   139fa:	e8 51 1b ff ff       	call   5550 <sgpl_loop_effective_decision_threads_for_loop_id>
   139ff:	44 8b 63 2c          	mov    0x2c(%rbx),%r12d
   13a03:	48 8b 53 18          	mov    0x18(%rbx),%rdx
   13a07:	31 f6                	xor    %esi,%esi
   13a09:	89 43 0c             	mov    %eax,0xc(%rbx)
   13a0c:	b9 01 00 00 00       	mov    $0x1,%ecx
   13a11:	4c 89 ef             	mov    %r13,%rdi
   13a14:	41 83 fc 01          	cmp    $0x1,%r12d
   13a18:	0f 84 40 01 00 00    	je     13b5e <task+0x2de>
   13a1e:	e8 dd b7 ff ff       	call   f200 <sgpl_should_parallelize_doall>
   13a23:	85 c0                	test   %eax,%eax
   13a25:	0f 85 e6 00 00 00    	jne    13b11 <task+0x291>
   13a2b:	c7 43 0c 01 00 00 00 	movl   $0x1,0xc(%rbx)
   13a32:	48 89 ee             	mov    %rbp,%rsi
   13a35:	bf 01 00 00 00       	mov    $0x1,%edi
   13a3a:	e8 b1 eb fe ff       	call   25f0 <clock_gettime@plt>
   13a3f:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   13a43:	4c 8b 7c 24 08       	mov    0x8(%rsp),%r15
   13a48:	4c 69 34 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%r14
   13a4f:	3b 
   13a50:	48 85 f6             	test   %rsi,%rsi
   13a53:	7e 7e                	jle    13ad3 <task+0x253>
   13a55:	8b 53 08             	mov    0x8(%rbx),%edx
   13a58:	48 8b 7b 20          	mov    0x20(%rbx),%rdi
   13a5c:	31 c9                	xor    %ecx,%ecx
   13a5e:	c5 d8 57 e4          	vxorps %xmm4,%xmm4,%xmm4
   13a62:	c5 fb 10 35 d6 8c 00 	vmovsd 0x8cd6(%rip),%xmm6        # 1c740 <__PRETTY_FUNCTION__.1+0x10>
   13a69:	00 
   13a6a:	c5 fb 10 2d d6 8c 00 	vmovsd 0x8cd6(%rip),%xmm5        # 1c748 <__PRETTY_FUNCTION__.1+0x18>
   13a71:	00 
   13a72:	c5 7b 10 05 be 8c 00 	vmovsd 0x8cbe(%rip),%xmm8        # 1c738 <__PRETTY_FUNCTION__.1+0x8>
   13a79:	00 
   13a7a:	c5 fb 10 3d ce 8c 00 	vmovsd 0x8cce(%rip),%xmm7        # 1c750 <__PRETTY_FUNCTION__.1+0x20>
   13a81:	00 
   13a82:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
   13a88:	48 83 c1 01          	add    $0x1,%rcx
   13a8c:	c4 e1 db 2a c9       	vcvtsi2sd %rcx,%xmm4,%xmm1
   13a91:	c5 f3 59 ce          	vmulsd %xmm6,%xmm1,%xmm1
   13a95:	c5 f3 59 d5          	vmulsd %xmm5,%xmm1,%xmm2
   13a99:	85 d2                	test   %edx,%edx
   13a9b:	7e 2b                	jle    13ac8 <task+0x248>
   13a9d:	31 c0                	xor    %eax,%eax
   13a9f:	90                   	nop
   13aa0:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
   13aa4:	c5 db 2a d8          	vcvtsi2sd %eax,%xmm4,%xmm3
   13aa8:	83 c0 01             	add    $0x1,%eax
   13aab:	c4 c1 7b 58 c0       	vaddsd %xmm8,%xmm0,%xmm0
   13ab0:	c5 fb 58 c3          	vaddsd %xmm3,%xmm0,%xmm0
   13ab4:	c5 f3 5e c0          	vdivsd %xmm0,%xmm1,%xmm0
   13ab8:	c5 f3 59 cf          	vmulsd %xmm7,%xmm1,%xmm1
   13abc:	c5 eb 58 d0          	vaddsd %xmm0,%xmm2,%xmm2
   13ac0:	c5 eb 58 c9          	vaddsd %xmm1,%xmm2,%xmm1
   13ac4:	39 c2                	cmp    %eax,%edx
   13ac6:	75 d8                	jne    13aa0 <task+0x220>
   13ac8:	c5 fb 11 54 cf f8    	vmovsd %xmm2,-0x8(%rdi,%rcx,8)
   13ace:	48 39 ce             	cmp    %rcx,%rsi
   13ad1:	75 b5                	jne    13a88 <task+0x208>
   13ad3:	48 89 ee             	mov    %rbp,%rsi
   13ad6:	bf 01 00 00 00       	mov    $0x1,%edi
   13adb:	e8 10 eb fe ff       	call   25f0 <clock_gettime@plt>
   13ae0:	4c 69 04 24 00 ca 9a 	imul   $0x3b9aca00,(%rsp),%r8
   13ae7:	3b 
   13ae8:	48 8b 53 18          	mov    0x18(%rbx),%rdx
   13aec:	4d 29 f8             	sub    %r15,%r8
   13aef:	4c 03 44 24 08       	add    0x8(%rsp),%r8
   13af4:	4d 29 f0             	sub    %r14,%r8
   13af7:	41 83 fc 01          	cmp    $0x1,%r12d
   13afb:	74 78                	je     13b75 <task+0x2f5>
   13afd:	b9 01 00 00 00       	mov    $0x1,%ecx
   13b02:	31 f6                	xor    %esi,%esi
   13b04:	4c 89 ef             	mov    %r13,%rdi
   13b07:	e8 54 c9 ff ff       	call   10460 <sgpl_record_doall_serial_sample>
   13b0c:	e9 8a fe ff ff       	jmp    1399b <task+0x11b>
   13b11:	8b 43 28             	mov    0x28(%rbx),%eax
   13b14:	45 31 c9             	xor    %r9d,%r9d
   13b17:	41 83 fc 01          	cmp    $0x1,%r12d
   13b1b:	ba 01 00 00 00       	mov    $0x1,%edx
   13b20:	41 0f 94 c1          	sete   %r9b
   13b24:	48 83 ec 08          	sub    $0x8,%rsp
   13b28:	49 89 d8             	mov    %rbx,%r8
   13b2b:	31 ff                	xor    %edi,%edi
   13b2d:	64 89 04 25 44 ff ff 	mov    %eax,%fs:0xffffffffffffff44
   13b34:	ff 
   13b35:	48 8d 05 74 19 ff ff 	lea    -0xe68c(%rip),%rax        # 54b0 <range>
   13b3c:	48 8d 0d bd 0a ff ff 	lea    -0xf543(%rip),%rcx        # 4600 <body>
   13b43:	64 48 89 04 25 98 ff 	mov    %rax,%fs:0xffffffffffffff98
   13b4a:	ff ff 
   13b4c:	48 8b 73 18          	mov    0x18(%rbx),%rsi
   13b50:	41 51                	push   %r9
   13b52:	e8 19 f7 ff ff       	call   13270 <parallel_for_runtime>
   13b57:	58                   	pop    %rax
   13b58:	5a                   	pop    %rdx
   13b59:	e9 3d fe ff ff       	jmp    1399b <task+0x11b>
   13b5e:	e8 ad e6 ff ff       	call   12210 <sgpl_should_parallelize_doacross>
   13b63:	e9 bb fe ff ff       	jmp    13a23 <task+0x1a3>
   13b68:	f0 83 05 54 c5 00 00 	lock addl $0x1,0xc554(%rip)        # 200c4 <early_loops>
   13b6f:	01 
   13b70:	e9 7e fe ff ff       	jmp    139f3 <task+0x173>
   13b75:	4d 89 c1             	mov    %r8,%r9
   13b78:	b9 01 00 00 00       	mov    $0x1,%ecx
   13b7d:	45 31 c0             	xor    %r8d,%r8d
   13b80:	31 f6                	xor    %esi,%esi
   13b82:	4c 89 ef             	mov    %r13,%rdi
   13b85:	e8 06 d3 ff ff       	call   10e90 <sgpl_record_doacross_serial_sample>
   13b8a:	e9 0c fe ff ff       	jmp    1399b <task+0x11b>
   13b8f:	e8 fc ea fe ff       	call   2690 <__stack_chk_fail@plt>

Disassembly of section .fini:
