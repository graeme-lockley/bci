# bci

This project is an implementation of a byte code interpreter.  The byte code has the following characteristics:

- Values are typed - boolean, 8-bit character, signed 32 bit integer, float, string, pointer and closure
- Two stacks - a frame stack and calculation stack

This byte code interpreter is helpful for running code for a typed language.

## Instruction Set

| Instruction | Semantics | Type Rules | Commentary |
|-|-|-|-|
| RET | Returns a void value | `SP` == 0 | This instruction must appear as the last instruction in a block |
| RET_S32 | Returns `Stack[SP-1]` | `SP == 1` and `typeOf(Stack[SP-1]) == S32` | This instruction must appear as the last instruction in a block |

