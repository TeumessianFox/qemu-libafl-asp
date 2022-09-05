# qemu-LibAFL-ASP
Combining LibAFL's QEMU patch with AMD SP's QEMU patch.

## Structure
```mermaid
flowchart LR;

VanillaQemu["Vanilla QEMU"];
ASPQemu["AMD SP QEMU"];
LibAFLQemu["LibAFL QEMU"];
ForkLibAFLQemu(["LibAFL QEMU fork\n(with patches)"]);
LibAFLPSPQemu{{"LibAFL + ASP QEMU\n(qemu-LibAFL-ASP)"}};

VanillaQemu --> ASPQemu;
ASPQemu --> LibAFLPSPQemu;
VanillaQemu --> LibAFLQemu;
LibAFLQemu --> ForkLibAFLQemu;
ForkLibAFLQemu --> LibAFLPSPQemu;
```
### Repos
- [Vanilla QEMU](https://github.com/qemu/qemu)
- [AMD SP QEMU](https://github.com/pascalharp/qemu)
- [LibAFL QEMU](https://github.com/AFLplusplus/qemu-libafl-bridge)
- [LibAFL QEMU fork (with patches)](https://github.com/TeumessianFox/qemu-libafl-bridge)
