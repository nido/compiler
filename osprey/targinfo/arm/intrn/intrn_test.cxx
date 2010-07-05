/* Test of intrinsics generated from MDS. */
/* This is not a functional test (auto check)
 * but just to test the compiler configuration
 * i.e. is the front-end recognized intrinsics, ...
 */

/* Note on dedicated registers: as these registers are not allocated by the compiler,
 * it is necessary to pass the index of such register. When it is passed as builtin
 * operand, an immediate of the form 'u[0..n]' is expected in place of dedicated register.
 * 'n' is the size of the register file minus one.
 * When dedicated register is the result, one additional operand of the same form 'u[0..n]'
 * is placed as first parameter to select the result register index.
 *
 * Note on immediates: some builtins may accept constant value as parameter. Programmer must
 * respect the immediate range given by the bits number and signess.
 * Immediates are given under the form '[su][0-9]+'
 * - [su] for signed and unsigned respectively.
 * - [0-9]+ the bits number associated to the immediate.
 */


#include "builtins_header.h"

