/*
 * Copyright (c) 2022 Miku AuahDark
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#if defined(__x86_64__) || defined(__ARM_ARCH_ISA_A64) || defined(_WIN64)

#ifdef __cplusplus
namespace
{
#endif
char _dummy[sizeof(void*) >= 8 ? 1 : -1]; // should fail to compile in 32-bit
#ifdef __cplusplus
}
#endif

#ifdef CMAKE_TRY_COMPILE
int main() {return 0;}
#endif

#else
#error "Only 64-bit target is supported"
#endif
