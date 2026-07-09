#!/usr/bin/env bash
# ==============================================================================
# Technical Blueprint: Boutaba NetRadar Orchestrator Core (v3.0)
# Chief Architect: Motezeballah Boutaba
# Design: Asynchronous parallel pipeline coupling with hardware compilation.
# ==============================================================================

set -e # الخروج الفوري عند حدوث أي خطأ في البنية التحتية للتنفيذ

echo "[*] Initializing Boutaba NetRadar Multi-Engine Hardened Suite..."

# 1. تخصيص مسار أنبوب نقل البيانات في الـ Transient RAM Space لمنع الـ Disk I/O
PIPE_PATH="/tmp/netradar_stream.fifo"
if [ ! -p "$PIPE_PATH" ]; then
    mkfifo "$PIPE_PATH"
fi

# 2. تجميع المحرك العتادي بلغة C مع تضمين ترويسة الـ Assembly المتقدمة
echo "[*] Compiling Hardware Core (packet_sniffer.c + inline_asm.h)..."
# تفعيل الـ Optimization level 3 لأقصى سرعة عتادية ممكنة
gcc -O3 -I./src src/packet_sniffer.c -o src/packet_sniffer -lpthread

# 3. تشغيل المحرك الأول (C Engine) في الخلفية ليوجه الحزم ديركت إلى الأنبوب
echo "[*] Injecting Low-Level Raw Socket Sniffer Engine (Ring 0 Level)..."
./src/packet_sniffer > "$PIPE_PATH" &
SNIFFER_PID=$!

# 4. تشغيل المحرك الثاني (Python Engine) ليمتص البيانات من الأنبوب ويحللها لامتناعياً
echo "[*] Injecting Asynchronous Logic Analyst Engine..."
python3 src/core_analyst.py < "$PIPE_PATH" &
ANALYST_PID=$!

# بروتوكول الإغلاق النظيف (Graceful Termination) عند الضغط على Ctrl+C لحماية السيرفر
cleanup() {
    echo -e "\n[*] Terminating Execution Streams and Hardening Memory Spaces..."
    kill $SNIFFER_PID 2>/dev/null || true
    kill $ANALYST_PID 2>/dev/null || true
    rm -f "$PIPE_PATH"
    echo "[*] Shared Pipeline Flushed. System Secured and Cleared."
}
trap cleanup EXIT

echo "[+] NetRadar Suite Framework is running dynamically. Capturing Telemetry..."
# إبقاء المنسق شغال لمراقبة تدفق القوة والـ PIDs
wait
