// 64-bit checking

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
