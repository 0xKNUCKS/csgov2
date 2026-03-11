#!/usr/bin/env python3
"""Find m_bCustomMaterialInitialized offset in client.dll"""
import struct
import sys

dll_path = r'D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive\csgo\bin\client.dll'

with open(dll_path, 'rb') as f:
    data = f.read()

print(f"Loaded client.dll: {len(data)} bytes")

# 1. Search for the string
s = b'm_bCustomMaterialInitialized'
idx = data.find(s)
if idx >= 0:
    print(f"\n[+] String 'm_bCustomMaterialInitialized' found at file offset 0x{idx:X}")
else:
    print("\n[-] String 'm_bCustomMaterialInitialized' not found")

# Also search related strings
for name in [b'CustomMaterial', b'CCustomMaterialOwner', b'CCSCustomMaterialSwapManager']:
    idx = data.find(name)
    if idx >= 0:
        print(f"[+] String '{name.decode()}' found at file offset 0x{idx:X}")

# 2. Pattern scan: C6 86 ?? ?? 00 00 ?? FF 50 04
# mov byte ptr [esi+OFFSET], VALUE; then FF 50 04 = call [eax+04]
print("\n--- Pattern: C6 86 XX XX 00 00 XX FF 50 04 ---")
i = 0
while i < len(data) - 10:
    if (data[i] == 0xC6 and data[i+1] == 0x86
        and data[i+4] == 0x00 and data[i+5] == 0x00
        and data[i+7] == 0xFF and data[i+8] == 0x50 and data[i+9] == 0x04):
        offset = struct.unpack_from('<I', data, i+2)[0]
        val = data[i+6]
        print(f"  File 0x{i:X}: mov byte ptr [esi+0x{offset:X}], {val}; call [eax+04]")
    i += 1

# 3. Broader pattern: C6 86 ?? ?? 00 00 01 (setting bool to true via esi)
# Focus on offsets in 0x3000-0x3500 range
print("\n--- Pattern: C6 86 XX XX 00 00 01 (offset in 0x3000-0x3500) ---")
i = 0
while i < len(data) - 7:
    if (data[i] == 0xC6 and data[i+1] == 0x86
        and data[i+4] == 0x00 and data[i+5] == 0x00
        and data[i+6] == 0x01):
        offset = struct.unpack_from('<I', data, i+2)[0]
        if 0x3000 <= offset <= 0x3500:
            # Show surrounding context
            ctx_before = data[max(0,i-4):i].hex(' ')
            ctx_after = data[i+7:i+15].hex(' ')
            print(f"  File 0x{i:X}: mov byte ptr [esi+0x{offset:X}], 1  [{ctx_before} | {ctx_after}]")
    i += 1

# 4. Pattern: C6 81 ?? ?? 00 00 01 (setting bool to true via ecx)
print("\n--- Pattern: C6 81 XX XX 00 00 01 (offset in 0x3000-0x3500) ---")
i = 0
while i < len(data) - 7:
    if (data[i] == 0xC6 and data[i+1] == 0x81
        and data[i+4] == 0x00 and data[i+5] == 0x00
        and data[i+6] == 0x01):
        offset = struct.unpack_from('<I', data, i+2)[0]
        if 0x3000 <= offset <= 0x3500:
            ctx_after = data[i+7:i+15].hex(' ')
            print(f"  File 0x{i:X}: mov byte ptr [ecx+0x{offset:X}], 1  [after: {ctx_after}]")
    i += 1

# 5. Pattern: 80 BE ?? ?? 00 00 00 (cmp byte ptr [esi+OFFSET], 0)
print("\n--- Pattern: 80 BE XX XX 00 00 00 (offset in 0x3000-0x3500) ---")
i = 0
while i < len(data) - 7:
    if (data[i] == 0x80 and data[i+1] == 0xBE
        and data[i+4] == 0x00 and data[i+5] == 0x00
        and data[i+6] == 0x00):
        offset = struct.unpack_from('<I', data, i+2)[0]
        if 0x3000 <= offset <= 0x3500:
            ctx_after = data[i+7:i+15].hex(' ')
            print(f"  File 0x{i:X}: cmp byte ptr [esi+0x{offset:X}], 0  [after: {ctx_after}]")
    i += 1

# 6. Search for 0x32DD specifically to see if it appears
print("\n--- Searching for 0x32DD as a 32-bit LE value ---")
needle = struct.pack('<I', 0x32DD)
pos = 0
while True:
    pos = data.find(needle, pos)
    if pos < 0:
        break
    ctx = data[max(0,pos-4):pos+8].hex(' ')
    print(f"  File 0x{pos:X}: {ctx}")
    pos += 1

# 7. CSGOSimple pattern: "83 BE ? ? ? ? ? 7F 67" for m_CustomMaterials
print("\n--- Pattern: 83 BE XX XX 00 00 XX 7F 67 (m_CustomMaterials check) ---")
i = 0
while i < len(data) - 9:
    if (data[i] == 0x83 and data[i+1] == 0xBE
        and data[i+4] == 0x00 and data[i+5] == 0x00
        and data[i+7] == 0x7F and data[i+8] == 0x67):
        offset = struct.unpack_from('<I', data, i+2)[0]
        val = data[i+6]
        print(f"  File 0x{i:X}: cmp dword ptr [esi+0x{offset:X}], {val}; jg +0x67")
    i += 1

print("\nDone.")
