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

## How to merge
1. Setup remotes
```
git remote add qemu-ASP git@github.com:pascalharp/qemu.git
git remote add qemu-LibAFL-fork git@github.com:TeumessianFox/qemu-libafl-bridge.git
```

2. Fetch remotes
```
git fetch qemu-LibAFL-fork
git fetch qemu-ASP
```

3. Merge LibAFL QEMU fork into repo
```
git merge remotes/qemu-LibAFL-fork/fix_configure --allow-unrelated-histories
```
Solve merge conflicts!

4. Merge ASP QEMU into repo
```
git merge remotes/qemu-ASP/psp_refactor
```
Solve merge conflicts!

## License

<sup>
This project extends the QEMU emulator, and our contributions to previously existing files adopt those files' respective licenses; the files that we have added are made available under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
</sup>

<br>
