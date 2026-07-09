/* ==============================================================================
 * Technical Blueprint: Boutaba NetRadar Inline Assembly Timers (v3.0)
 * Chief Architect: Motezeballah Boutaba
 * Design: High-precision x86_64 TSC reader for nanosecond latency auditing.
 * ==============================================================================
 */

#ifndef INLINE_ASM_H
#define INLINE_ASM_H

#include <stdint.h>

/**
 * دالة نينجا برمجية تقرأ مسجلات المعالج مباشرة في الوقت الفعلي
 * تستخدم rdtsc لقراءة الـ Time Stamp Counter و lfence لمنع الـ Out-of-Order Execution
 */
static inline uint64_t read_hardware_timestamp(void) {
    uint32_t low, high;
    
    /* جدار حماية عتادي (Memory Barrier) يضمن عدم تنفيذ المعالج لأي أوامر قبل قياس الوقت */
    __asm__ __volatile__ (
        "lfence\n\t"
        "rdtsc\n\t"
        : "=a" (low), "=d" (high)
        :
        : "memory"
    );
    
    /* دمج مسجل rdx (high) ومسجل rax (low) للحصول على القيمة كاملة في 64-بت */
    return ((uint64_t)high << 32) | low;
}

#endif /* INLINE_ASM_H */
