# UnsupportedInstructionsLiftingToLLVM

Using Zydis and LLVM to lift assembly instructions to LLVM-IR as inline assembly calls. This is meant to be used when lifting a virtual machine protection or obfuscated code, in a context where the lifter doesn't provide the semantic for a specific instruction and supports only a subset of the machine registers.

This PoC shows how to lift some assembly instructions assuming that the lifter supports only the general purpose registers. The general purpose registers (implicitly or explicitly) read by the instruction are loaded from the virtual registers context and fed as arguments to the inline assembly call. The general purpose registers (implicitly or explicitly) written by the instruction are obtained by the inline assembly call result and stored on the virtual registers context.

The support to the clobber constraints is only sketched (e.g. `~{memory}` is currently naïvely supported detecting if the instruction is executing an implicit or hidden memory access). By default the inline assembly calls are marked as having `sideeffect`, but ideally that should be used only when the constraints list is not explicitly mentioning some effects of the assembly instruction. Changes to the stack pointer are currently unsupported as they mess with the local stack frame.

# Sample output (unoptimized)

```llvm
; ModuleID = 'Module'
source_filename = "Module"

%ContextTy = type { %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR }
%RegisterR = type { %RegisterW }
%RegisterW = type { i64 }
%RegisterB = type { i8, i8, i8, i8, i8, i8, i8, i8 }
%IAOutTy = type { i32, i32, i32, i32 }
%IAOutTy.0 = type { i32, i32 }
%IAOutTy.1 = type { i64, i64 }
%IAOutTy.2 = type { i64, i64 }

; Function Attrs: alwaysinline
define void @Unsupported_pop(%ContextTy* %0) #0 {
  %2 = call i64 asm sideeffect inteldialect "pop rsp", "={rsp},~{memory}"() #1
  %3 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0
  %4 = bitcast %RegisterW* %3 to %RegisterB*
  %5 = getelementptr inbounds %RegisterB, %RegisterB* %4, i64 0, i32 0
  %6 = bitcast i8* %5 to i64*
  store i64 %2, i64* %6, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_std(%ContextTy* %0) #0 {
  call void asm sideeffect inteldialect "std", "~{flags},~{dirflag}"() #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i8*
  %6 = load i8, i8* %5, align 1
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 1
  %10 = bitcast i8* %9 to i8*
  %11 = load i8, i8* %10, align 1
  %12 = call i8 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i8 %6, i8 %11) #1
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %14 = bitcast %RegisterW* %13 to %RegisterB*
  %15 = getelementptr inbounds %RegisterB, %RegisterB* %14, i64 0, i32 1
  %16 = bitcast i8* %15 to i8*
  store i8 %12, i8* %16, align 1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_fsqrt(%ContextTy* %0) #0 {
  call void asm sideeffect inteldialect "fsqrt", "~{fpsr}"() #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_fsincos(%ContextTy* %0) #0 {
  call void asm sideeffect inteldialect "fsincos", "~{fpsr}"() #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_fstp(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  call void asm sideeffect inteldialect "fstp qword ptr ds:[$0], st0", "r,~{fpsr}"(i64 %6) #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add.1(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i32*
  %6 = load i32, i32* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i32*
  %11 = load i32, i32* %10, align 4
  %12 = call i32 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i32 %6, i32 %11) #1
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %14 = bitcast %RegisterW* %13 to %RegisterB*
  %15 = getelementptr inbounds %RegisterB, %RegisterB* %14, i64 0, i32 0
  %16 = bitcast i8* %15 to i32*
  store i32 %12, i32* %16, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_cpuid(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i32*
  %6 = load i32, i32* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i32*
  %11 = load i32, i32* %10, align 4
  %12 = call %IAOutTy asm sideeffect inteldialect "cpuid", "={eax},={ecx},={edx},={ebx},{eax},{ecx}"(i32 %6, i32 %11) #1
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %14 = bitcast %RegisterW* %13 to %RegisterB*
  %15 = getelementptr inbounds %RegisterB, %RegisterB* %14, i64 0, i32 0
  %16 = bitcast i8* %15 to i32*
  %17 = extractvalue %IAOutTy %12, 0
  store i32 %17, i32* %16, align 4
  %18 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2, i32 0
  %19 = bitcast %RegisterW* %18 to %RegisterB*
  %20 = getelementptr inbounds %RegisterB, %RegisterB* %19, i64 0, i32 0
  %21 = bitcast i8* %20 to i32*
  %22 = extractvalue %IAOutTy %12, 1
  store i32 %22, i32* %21, align 4
  %23 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %24 = bitcast %RegisterW* %23 to %RegisterB*
  %25 = getelementptr inbounds %RegisterB, %RegisterB* %24, i64 0, i32 0
  %26 = bitcast i8* %25 to i32*
  %27 = extractvalue %IAOutTy %12, 2
  store i32 %27, i32* %26, align 4
  %28 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %29 = bitcast %RegisterW* %28 to %RegisterB*
  %30 = getelementptr inbounds %RegisterB, %RegisterB* %29, i64 0, i32 0
  %31 = bitcast i8* %30 to i32*
  %32 = extractvalue %IAOutTy %12, 3
  store i32 %32, i32* %31, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_rdtsc(%ContextTy* %0) #0 {
  %2 = call %IAOutTy.0 asm sideeffect inteldialect "rdtsc", "={eax},={edx}"() #1
  %3 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %4 = bitcast %RegisterW* %3 to %RegisterB*
  %5 = getelementptr inbounds %RegisterB, %RegisterB* %4, i64 0, i32 0
  %6 = bitcast i8* %5 to i32*
  %7 = extractvalue %IAOutTy.0 %2, 0
  store i32 %7, i32* %6, align 4
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %9 = bitcast %RegisterW* %8 to %RegisterB*
  %10 = getelementptr inbounds %RegisterB, %RegisterB* %9, i64 0, i32 0
  %11 = bitcast i8* %10 to i32*
  %12 = extractvalue %IAOutTy.0 %2, 1
  store i32 %12, i32* %11, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_div(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i64*
  %11 = load i64, i64* %10, align 4
  %12 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2, i32 0
  %13 = bitcast %RegisterW* %12 to %RegisterB*
  %14 = getelementptr inbounds %RegisterB, %RegisterB* %13, i64 0, i32 0
  %15 = bitcast i8* %14 to i64*
  %16 = load i64, i64* %15, align 4
  %17 = call %IAOutTy.1 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %6, i64 %11, i64 %16) #1
  %18 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %19 = bitcast %RegisterW* %18 to %RegisterB*
  %20 = getelementptr inbounds %RegisterB, %RegisterB* %19, i64 0, i32 0
  %21 = bitcast i8* %20 to i64*
  %22 = extractvalue %IAOutTy.1 %17, 0
  store i64 %22, i64* %21, align 4
  %23 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %24 = bitcast %RegisterW* %23 to %RegisterB*
  %25 = getelementptr inbounds %RegisterB, %RegisterB* %24, i64 0, i32 0
  %26 = bitcast i8* %25 to i64*
  %27 = extractvalue %IAOutTy.1 %17, 1
  store i64 %27, i64* %26, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add.2(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i64*
  %11 = load i64, i64* %10, align 4
  %12 = call i64 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i64 %6, i64 %11) #1
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %14 = bitcast %RegisterW* %13 to %RegisterB*
  %15 = getelementptr inbounds %RegisterB, %RegisterB* %14, i64 0, i32 0
  %16 = bitcast i8* %15 to i64*
  store i64 %12, i64* %16, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_bswap(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = call i64 asm sideeffect inteldialect "bswap $0", "=r,0"(i64 %6) #1
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %9 = bitcast %RegisterW* %8 to %RegisterB*
  %10 = getelementptr inbounds %RegisterB, %RegisterB* %9, i64 0, i32 0
  %11 = bitcast i8* %10 to i64*
  store i64 %7, i64* %11, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_div.3(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i64*
  %11 = load i64, i64* %10, align 4
  %12 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %13 = bitcast %RegisterW* %12 to %RegisterB*
  %14 = getelementptr inbounds %RegisterB, %RegisterB* %13, i64 0, i32 0
  %15 = bitcast i8* %14 to i64*
  %16 = load i64, i64* %15, align 4
  %17 = call %IAOutTy.2 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %6, i64 %11, i64 %16) #1
  %18 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0
  %19 = bitcast %RegisterW* %18 to %RegisterB*
  %20 = getelementptr inbounds %RegisterB, %RegisterB* %19, i64 0, i32 0
  %21 = bitcast i8* %20 to i64*
  %22 = extractvalue %IAOutTy.2 %17, 0
  store i64 %22, i64* %21, align 4
  %23 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %24 = bitcast %RegisterW* %23 to %RegisterB*
  %25 = getelementptr inbounds %RegisterB, %RegisterB* %24, i64 0, i32 0
  %26 = bitcast i8* %25 to i64*
  %27 = extractvalue %IAOutTy.2 %17, 1
  store i64 %27, i64* %26, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_mov(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4, i32 0
  %8 = bitcast %RegisterW* %7 to %RegisterB*
  %9 = getelementptr inbounds %RegisterB, %RegisterB* %8, i64 0, i32 0
  %10 = bitcast i8* %9 to i64*
  %11 = load i64, i64* %10, align 4
  %12 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 5, i32 0
  %13 = bitcast %RegisterW* %12 to %RegisterB*
  %14 = getelementptr inbounds %RegisterB, %RegisterB* %13, i64 0, i32 0
  %15 = bitcast i8* %14 to i64*
  %16 = load i64, i64* %15, align 4
  call void asm sideeffect inteldialect "mov qword ptr ds:[$1+$0*2+0x08], $2", "r,r,r"(i64 %6, i64 %11, i64 %16) #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_mov.4(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = call i64 asm sideeffect inteldialect "mov $0, qword ptr ds:[$0+$0*2+0x08]", "=r,0"(i64 %6) #1
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4, i32 0
  %9 = bitcast %RegisterW* %8 to %RegisterB*
  %10 = getelementptr inbounds %RegisterB, %RegisterB* %9, i64 0, i32 0
  %11 = bitcast i8* %10 to i64*
  store i64 %7, i64* %11, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_call(%ContextTy* %0) #0 {
  call void asm sideeffect "call 0x0000000140001E60", "~{memory}"() #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_mov.5(%ContextTy* %0) #0 {
  %2 = call i64 asm sideeffect inteldialect "mov rsp, 0x1000", "={rsp}"() #1
  %3 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0
  %4 = bitcast %RegisterW* %3 to %RegisterB*
  %5 = getelementptr inbounds %RegisterB, %RegisterB* %4, i64 0, i32 0
  %6 = bitcast i8* %5 to i64*
  store i64 %2, i64* %6, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add.6(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0
  %3 = bitcast %RegisterW* %2 to %RegisterB*
  %4 = getelementptr inbounds %RegisterB, %RegisterB* %3, i64 0, i32 0
  %5 = bitcast i8* %4 to i64*
  %6 = load i64, i64* %5, align 4
  %7 = call i64 asm sideeffect inteldialect "add $0, 0x1000", "=r,0,~{flags}"(i64 %6) #1
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0
  %9 = bitcast %RegisterW* %8 to %RegisterB*
  %10 = getelementptr inbounds %RegisterB, %RegisterB* %9, i64 0, i32 0
  %11 = bitcast i8* %10 to i64*
  store i64 %7, i64* %11, align 4
  ret void
}

attributes #0 = { alwaysinline }
attributes #1 = { nounwind }
```

# Sample output (optimized)

```llvm
%ContextTy = type { %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR, %RegisterR }
%RegisterR = type { %RegisterW }
%RegisterW = type { i64 }
%RegisterB = type { i8, i8, i8, i8, i8, i8, i8, i8 }
%IAOutTy = type { i32, i32, i32, i32 }
%IAOutTy.0 = type { i32, i32 }
%IAOutTy.1 = type { i64, i64 }
%IAOutTy.2 = type { i64, i64 }

define void @Unsupported_pop(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = tail call i64 asm sideeffect inteldialect "pop rsp", "={rsp},~{memory}"() #1
  %3 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0, i32 0
  store i64 %2, i64* %3, align 4
  ret void
}

define void @Unsupported_std(%ContextTy* nocapture readnone %0) local_unnamed_addr #0 {
  tail call void asm sideeffect inteldialect "std", "~{flags},~{dirflag}"() #1
  ret void
}

define void @Unsupported_add(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to i8*
  %4 = load i8, i8* %3, align 1
  %5 = bitcast %ContextTy* %0 to %RegisterB*
  %6 = getelementptr inbounds %RegisterB, %RegisterB* %5, i64 0, i32 1
  %7 = load i8, i8* %6, align 1
  %8 = tail call i8 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i8 %4, i8 %7) #1
  store i8 %8, i8* %6, align 1
  ret void
}

define void @Unsupported_fsqrt(%ContextTy* nocapture readnone %0) local_unnamed_addr #0 {
  tail call void asm sideeffect inteldialect "fsqrt", "~{fpsr}"() #1
  ret void
}

define void @Unsupported_fsincos(%ContextTy* nocapture readnone %0) local_unnamed_addr #0 {
  tail call void asm sideeffect inteldialect "fsincos", "~{fpsr}"() #1
  ret void
}

define void @Unsupported_fstp(%ContextTy* nocapture readonly %0) local_unnamed_addr #0 {
  %2 = getelementptr %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  tail call void asm sideeffect inteldialect "fstp qword ptr ds:[$0], st0", "r,~{fpsr}"(i64 %3) #1
  ret void
}

define void @Unsupported_add.1(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %3 = bitcast %RegisterW* %2 to i32*
  %4 = load i32, i32* %3, align 4
  %5 = bitcast %ContextTy* %0 to i32*
  %6 = load i32, i32* %5, align 4
  %7 = tail call i32 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i32 %4, i32 %6) #1
  store i32 %7, i32* %5, align 4
  ret void
}

define void @Unsupported_cpuid(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = bitcast %ContextTy* %0 to i32*
  %3 = load i32, i32* %2, align 4
  %4 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2, i32 0
  %5 = bitcast %RegisterW* %4 to i32*
  %6 = load i32, i32* %5, align 4
  %7 = tail call %IAOutTy asm sideeffect inteldialect "cpuid", "={eax},={ecx},={edx},={ebx},{eax},{ecx}"(i32 %3, i32 %6) #1
  %8 = extractvalue %IAOutTy %7, 0
  store i32 %8, i32* %2, align 4
  %9 = extractvalue %IAOutTy %7, 1
  store i32 %9, i32* %5, align 4
  %10 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %11 = bitcast %RegisterW* %10 to i32*
  %12 = extractvalue %IAOutTy %7, 2
  store i32 %12, i32* %11, align 4
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0
  %14 = bitcast %RegisterW* %13 to i32*
  %15 = extractvalue %IAOutTy %7, 3
  store i32 %15, i32* %14, align 4
  ret void
}

define void @Unsupported_rdtsc(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = tail call %IAOutTy.0 asm sideeffect inteldialect "rdtsc", "={eax},={edx}"() #1
  %3 = bitcast %ContextTy* %0 to i32*
  %4 = extractvalue %IAOutTy.0 %2, 0
  store i32 %4, i32* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0
  %6 = bitcast %RegisterW* %5 to i32*
  %7 = extractvalue %IAOutTy.0 %2, 1
  store i32 %7, i32* %6, align 4
  ret void
}

define void @Unsupported_div(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0, i32 0
  %5 = load i64, i64* %4, align 4
  %6 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2, i32 0, i32 0
  %7 = load i64, i64* %6, align 4
  %8 = tail call %IAOutTy.1 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %3, i64 %5, i64 %7) #1
  %9 = extractvalue %IAOutTy.1 %8, 0
  store i64 %9, i64* %2, align 4
  %10 = extractvalue %IAOutTy.1 %8, 1
  store i64 %10, i64* %4, align 4
  ret void
}

define void @Unsupported_add.2(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = getelementptr %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0, i32 0
  %5 = load i64, i64* %4, align 4
  %6 = tail call i64 asm sideeffect inteldialect "add $0, $1", "=r,r,0,~{flags}"(i64 %3, i64 %5) #1
  store i64 %6, i64* %4, align 4
  ret void
}

define void @Unsupported_bswap(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = tail call i64 asm sideeffect inteldialect "bswap $0", "=r,0"(i64 %3) #1
  store i64 %4, i64* %2, align 4
  ret void
}

define void @Unsupported_div.3(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr %ContextTy, %ContextTy* %0, i64 0, i32 0, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0, i32 0
  %5 = load i64, i64* %4, align 4
  %6 = tail call %IAOutTy.2 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %3, i64 %5, i64 %3) #1
  %7 = extractvalue %IAOutTy.2 %6, 0
  store i64 %7, i64* %2, align 4
  %8 = extractvalue %IAOutTy.2 %6, 1
  store i64 %8, i64* %4, align 4
  ret void
}

define void @Unsupported_mov(%ContextTy* nocapture readonly %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4, i32 0, i32 0
  %5 = load i64, i64* %4, align 4
  %6 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 5, i32 0, i32 0
  %7 = load i64, i64* %6, align 4
  tail call void asm sideeffect inteldialect "mov qword ptr ds:[$1+$0*2+0x08], $2", "r,r,r"(i64 %3, i64 %5, i64 %7) #1
  ret void
}

define void @Unsupported_mov.4(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = tail call i64 asm sideeffect inteldialect "mov $0, qword ptr ds:[$0+$0*2+0x08]", "=r,0"(i64 %3) #1
  store i64 %4, i64* %2, align 4
  ret void
}

define void @Unsupported_call(%ContextTy* nocapture readnone %0) local_unnamed_addr #0 {
  tail call void asm sideeffect "call 0x0000000140001E60", "~{memory}"() #1
  ret void
}

define void @Unsupported_mov.5(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = tail call i64 asm sideeffect inteldialect "mov rsp, 0x1000", "={rsp}"() #1
  %3 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0, i32 0
  store i64 %2, i64* %3, align 4
  ret void
}

define void @Unsupported_add.6(%ContextTy* nocapture %0) local_unnamed_addr #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 6, i32 0, i32 0
  %3 = load i64, i64* %2, align 4
  %4 = tail call i64 asm sideeffect inteldialect "add $0, 0x1000", "=r,0,~{flags}"(i64 %3) #1
  store i64 %4, i64* %2, align 4
  ret void
}

attributes #0 = { alwaysinline }
attributes #1 = { nounwind }
```