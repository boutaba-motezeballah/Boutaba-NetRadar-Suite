#!/usr/bin/env python3
"""
==============================================================================
Technical Blueprint: Boutaba NetRadar Asynchronous Logic Analyst (v3.0)
Chief Architect: Motezeballah Boutaba
Design: Stream-driven byte-dissector with execution boundary enforcement.
==============================================================================
"""

import sys
import asyncio
import struct

async def read_exact(stream, n):
    """دالة حماية صارمة تضمن قراءة عدد البايتات المطلوب بالظبط دون نقص"""
    data = b''
    while len(data) < n:
        packet = await stream.read(n - len(data))
        if not packet:
            return None
        data += packet
    return data

async def main():
    print("[*] Asynchronous Logic Analyst Core Active. Awaiting Stream...")
    sys.stdout.flush()

    # فتح بث لامتزامن لقراءة الـ standard input (stdin)
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await loop.connect_read_pipe(lambda: protocol, sys.stdin)

    while True:
        # 1. قراءة حجم الحزمة القادمة (8 بايت المخصصة للـ ssize_t في الـ C)
        size_header = await read_exact(reader, 8)
        if not size_header:
            break

        # فك تشفير حجم الحزمة بشكل آمن
        packet_size = struct.unpack('Q', size_header)[0]

        # 2. جدار حماية لمنع الـ Buffer Overflow: التحقق من الحدود الصارمة لحجم الحزمة
        if packet_size <= 0 or packet_size > 65536:
            continue

        # 3. سحب بايتات الحزمة كاملة ديركت بناءً على الحجم المحدد
        packet_bytes = await read_exact(reader, packet_size)
        if not packet_bytes:
            break

        # 4. تفكيك الـ Ethernet Header (أول 14 بايت: Mac Dest, Mac Src, Type)
        if len(packet_bytes) >= 14:
            eth_header = struct.unpack('!6s6sH', packet_bytes[:14])
            dest_mac = ':'.join(f'{b:02x}' for b in eth_header[0])
            src_mac = ':'.join(f'{b:02x}' for b in eth_header[1])
            eth_proto = eth_header[2]

            # 5. إذا كانت الحزمة IP (0x0800)، نفكك الـ IP Header لقراءة الـ IPs ديركت
            if eth_proto == 0x0800 and len(packet_bytes) >= 34:
                ip_header = struct.unpack('!BBHHHBBH4s4s', packet_bytes[14:34])
                ttl = ip_header[5]
                proto = ip_header[6]
                src_ip = '.'.join(str(b) for b in ip_header[8])
                dest_ip = '.'.join(str(b) for b in ip_header[9])

                # عرض المصفوفة الرقمية شاعلة ونظيفة في الـ Terminal في الوقت الفعلي
                print(f"[RADAR] SRC: {src_ip} -> DST: {dest_ip} | TTL: {ttl} | PROTO: {proto} | Size: {packet_size}B")
                sys.stdout.flush()

if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
