# UnsupportedInstructionsLiftingToLLVM

Using Zydis and LLVM to lift assembly instructions to LLVM-IR as inline assembly calls. This is meant to be used when lifting a virtual machine protection or obfuscated code, in a context where the lifter doesn't provide the semantic for a specific instruction and supports only a subset of the machine registers.

This PoC shows how to lift some assembly instructions assuming that the lifter supports only the general purpose registers. The general purpose registers (implicitly or explicitly) read by the instruction are loaded from the virtual registers context and fed as arguments to the inline assembly call. The general purpose registers (implicitly or explicitly) written by the instruction are obtained by the inline assembly call result and stored on the virtual registers context.

The support to the clobbered constraints is only sketched (e.g. `~{memory}` is currently unsupported). By default the inline assembly calls are marked as having `sideeffect`, but ideally that should be used only when the constraints list is not explicitly mentioning some effects of the assembly instruction.

# Sample output

```
; ModuleID = 'Module'
source_filename = "Module"

%ContextTy = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64 }
%IAOutTy = type { i32, i32, i32, i32 }
%IAOutTy.0 = type { i32, i32 }
%IAOutTy.1 = type { i64, i64 }
%IAOutTy.2 = type { i64, i64 }

; Function Attrs: alwaysinline
define void @Unsupported_std(%ContextTy* %0) #0 {
  call void asm sideeffect inteldialect "std", "~{flags},~{dirflag}"() #1
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
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  call void asm sideeffect inteldialect "fstp qword ptr ds:[$0], st0", "r,~{fpsr}"(i64 %4) #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1
  %3 = bitcast i64* %2 to i32*
  %4 = load i32, i32* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %6 = bitcast i64* %5 to i32*
  %7 = load i32, i32* %6, align 4
  %8 = call i32 asm sideeffect inteldialect "add $1, $0", "=r,r,0,~{flags}"(i32 %4, i32 %7) #1
  %9 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %10 = bitcast i64* %9 to i32*
  store i32 %8, i32* %10, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_cpuid(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %3 = bitcast i64* %2 to i32*
  %4 = load i32, i32* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2
  %6 = bitcast i64* %5 to i32*
  %7 = load i32, i32* %6, align 4
  %8 = call %IAOutTy asm sideeffect inteldialect "cpuid", "={eax},={ecx},={edx},={ebx},{eax},{ecx}"(i32 %4, i32 %7) #1
  %9 = extractvalue %IAOutTy %8, 0
  %10 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %11 = bitcast i64* %10 to i32*
  store i32 %9, i32* %11, align 4
  %12 = extractvalue %IAOutTy %8, 1
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2
  %14 = bitcast i64* %13 to i32*
  store i32 %12, i32* %14, align 4
  %15 = extractvalue %IAOutTy %8, 2
  %16 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %17 = bitcast i64* %16 to i32*
  store i32 %15, i32* %17, align 4
  %18 = extractvalue %IAOutTy %8, 3
  %19 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1
  %20 = bitcast i64* %19 to i32*
  store i32 %18, i32* %20, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_rdtsc(%ContextTy* %0) #0 {
  %2 = call %IAOutTy.0 asm sideeffect inteldialect "rdtsc", "={eax},={edx}"() #1
  %3 = extractvalue %IAOutTy.0 %2, 0
  %4 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %5 = bitcast i64* %4 to i32*
  store i32 %3, i32* %5, align 4
  %6 = extractvalue %IAOutTy.0 %2, 1
  %7 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %8 = bitcast i64* %7 to i32*
  store i32 %6, i32* %8, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_div(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %6 = bitcast i64* %5 to i64*
  %7 = load i64, i64* %6, align 4
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 2
  %9 = bitcast i64* %8 to i64*
  %10 = load i64, i64* %9, align 4
  %11 = call %IAOutTy.1 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %4, i64 %7, i64 %10) #1
  %12 = extractvalue %IAOutTy.1 %11, 0
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %14 = bitcast i64* %13 to i64*
  store i64 %12, i64* %14, align 4
  %15 = extractvalue %IAOutTy.1 %11, 1
  %16 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %17 = bitcast i64* %16 to i64*
  store i64 %15, i64* %17, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_add.1(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %6 = bitcast i64* %5 to i64*
  %7 = load i64, i64* %6, align 4
  %8 = call i64 asm sideeffect inteldialect "add $1, $0", "=r,r,0,~{flags}"(i64 %4, i64 %7) #1
  %9 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %10 = bitcast i64* %9 to i64*
  store i64 %8, i64* %10, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_bswap(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = call i64 asm sideeffect inteldialect "bswap $0", "=r,0"(i64 %4) #1
  %6 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 1
  %7 = bitcast i64* %6 to i64*
  store i64 %5, i64* %7, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_div.2(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %6 = bitcast i64* %5 to i64*
  %7 = load i64, i64* %6, align 4
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %9 = bitcast i64* %8 to i64*
  %10 = load i64, i64* %9, align 4
  %11 = call %IAOutTy.2 asm sideeffect inteldialect "div $4", "={rax},={rdx},{rax},{rdx},r,~{flags}"(i64 %4, i64 %7, i64 %10) #1
  %12 = extractvalue %IAOutTy.2 %11, 0
  %13 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 0
  %14 = bitcast i64* %13 to i64*
  store i64 %12, i64* %14, align 4
  %15 = extractvalue %IAOutTy.2 %11, 1
  %16 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %17 = bitcast i64* %16 to i64*
  store i64 %15, i64* %17, align 4
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_mov(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 3
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4
  %6 = bitcast i64* %5 to i64*
  %7 = load i64, i64* %6, align 4
  %8 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 5
  %9 = bitcast i64* %8 to i64*
  %10 = load i64, i64* %9, align 4
  call void asm sideeffect inteldialect "mov qword ptr ds:[$1+$0*2+0x08], $2", "r,r,r"(i64 %4, i64 %7, i64 %10) #1
  ret void
}

; Function Attrs: alwaysinline
define void @Unsupported_mov.3(%ContextTy* %0) #0 {
  %2 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4
  %3 = bitcast i64* %2 to i64*
  %4 = load i64, i64* %3, align 4
  %5 = call i64 asm sideeffect inteldialect "mov $0, qword ptr ds:[$0+$0*2+0x08]", "=r,0"(i64 %4) #1
  %6 = getelementptr inbounds %ContextTy, %ContextTy* %0, i64 0, i32 4
  %7 = bitcast i64* %6 to i64*
  store i64 %5, i64* %7, align 4
  ret void
}

attributes #0 = { alwaysinline }
attributes #1 = { nounwind }
```
