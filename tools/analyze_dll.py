#!/usr/bin/env python3
"""
CS:GO DLL Static Analysis Toolkit
Extracts RTTI class hierarchies, vtable layouts, interface registrations,
string tables, and cross-references against our SDK.

Usage:
    python tools/analyze_dll.py                    # Analyze all key DLLs
    python tools/analyze_dll.py --dll client       # Analyze specific DLL
    python tools/analyze_dll.py --vtable CCSPlayer # Find vtable for class
    python tools/analyze_dll.py --interfaces       # List all CreateInterface exports
    python tools/analyze_dll.py --netvars          # Find netvar-related strings
    python tools/analyze_dll.py --strings client   # Dump interesting strings
    python tools/analyze_dll.py --xref             # Cross-reference SDK vtable indices
    python tools/analyze_dll.py --find-class CCSGOPlayerAnimState  # Find class RTTI
    python tools/analyze_dll.py --all              # Full analysis (slow, comprehensive)
"""

import argparse
import os
import struct
import sys
import json
import re
from pathlib import Path
from collections import defaultdict

try:
    import pefile
except ImportError:
    print("ERROR: pefile not installed. Run: pip install pefile")
    sys.exit(1)

# ============================================================================
# Configuration
# ============================================================================

CSGO_BIN = Path(r"D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive\bin")
CSGO_GAME_BIN = Path(r"D:\SteamLibrary\steamapps\common\Counter-Strike Global Offensive\csgo\bin")

DLL_PATHS = {
    "engine":          CSGO_BIN / "engine.dll",
    "vstdlib":         CSGO_BIN / "vstdlib.dll",
    "materialsystem":  CSGO_BIN / "materialsystem.dll",
    "vguimatsurface":  CSGO_BIN / "vguimatsurface.dll",
    "inputsystem":     CSGO_BIN / "inputsystem.dll",
    "studiorender":    CSGO_BIN / "studiorender.dll",
    "datacache":       CSGO_BIN / "datacache.dll",
    "client":          CSGO_GAME_BIN / "client.dll",
    "server":          CSGO_GAME_BIN / "server.dll",
}

# Our SDK's known vtable indices (from entity.h VIRTUAL_METHOD calls)
SDK_VTABLE_INDICES = {
    "gEntity": {
        # ClientNetworkable vtable (this + 2*sizeof(uintptr_t))
        "release": 1,
        "getClientClass": 2,
        "preDataUpdate": 6,
        "postDataUpdate": 7,
        "isDormant": 9,
        "index": 10,
        "setDestroyedOnRecreateEntities": 13,
        # Renderable vtable (this + sizeof(uintptr_t))
        "getRenderOrigin": 1,
        "shouldDraw": 3,
        "getModel": 8,
        "SetupBones": 13,
        "toWorldTransform": 32,
        # Entity vtable (this + 0)
        "setModelIndex": 75,
        "health": 122,
        "isAlive": 156,
        "isPlayer": 158,
        "isWeapon": 166,
        "updateClientSideAnimation": 224,
        "getEyePosition": 285,
        "getWeaponSubType": 282,
        "getObserverMode": 294,
        "getSpread": 453,
        "getWeaponType": 455,
        "getWeaponData": 461,
        "getMuzzleAttachmentIndex1stPerson": 468,
        "getMuzzleAttachmentIndex3rdPerson": 469,
        "getInaccuracy": 483,
        "updateInaccuracyPenalty": 484,
    },
    "IVEngineClient": {
        # From EngineClient.h
        "GetScreenSize": 5,
        "GetPlayerInfo": 8,
        "GetLocalPlayer": 12,
        "GetViewAngles": 18,
        "SetViewAngles": 19,
        "GetMaxClients": 20,
        "IsInGame": 26,
        "IsConnected": 27,
        "WorldToScreenMatrix": 37,
    },
    "IEngineTrace": {
        "TraceRay": 5,
    },
    "IVModelInfo": {
        "GetModel": 1,
        "GetModelIndex": 2,
        "GetModelName": 3,
        "GetStudioModel": 32,
    },
}

# Known interface version strings
KNOWN_INTERFACES = {
    "VClientEntityList003": ("client.dll", "IClientEntityList"),
    "VClient018": ("client.dll", "IBaseClientDLL"),
    "VEngineClient014": ("engine.dll", "IVEngineClient"),
    "InputSystemVersion001": ("inputsystem.dll", "CInputSystem"),
    "VModelInfoClient004": ("engine.dll", "IVModelInfo"),
    "EngineTraceClient004": ("engine.dll", "IEngineTrace"),
    "VEngineCvar007": ("vstdlib.dll", "ICvar"),
    "VGUI_Surface031": ("vguimatsurface.dll", "ISurface"),
    # Interfaces we might want to add
    "VMaterialSystem080": ("materialsystem.dll", "IMaterialSystem"),
    "VEngineModel016": ("engine.dll", "IVModelRender"),
    "GAMEEVENTSMANAGER002": ("engine.dll", "IGameEventManager2"),
    "IEngineSoundClient003": ("engine.dll", "IEngineSound"),
    "VStudioRender026": ("studiorender.dll", "IStudioRender"),
}

# Netvar table names we care about
NETVAR_TABLES = [
    "DT_BaseEntity", "DT_BasePlayer", "DT_CSPlayer", "DT_BaseCombatWeapon",
    "DT_BaseCombatCharacter", "DT_BaseAttributableItem", "DT_BaseAnimating",
    "DT_PlayerResource", "DT_CSPlayerResource",
]

# ============================================================================
# PE Analysis Helpers
# ============================================================================

class DLLAnalyzer:
    """Analyzes a single DLL file."""

    def __init__(self, name, path):
        self.name = name
        self.path = Path(path)
        self.pe = None
        self.data = None
        self.image_base = 0
        self.rdata_section = None
        self.text_section = None

    def load(self):
        if not self.path.exists():
            print(f"  [!] {self.path} not found")
            return False
        self.pe = pefile.PE(str(self.path), fast_load=False)
        self.data = self.pe.__data__
        self.image_base = self.pe.OPTIONAL_HEADER.ImageBase
        for section in self.pe.sections:
            name = section.Name.decode('utf-8', errors='ignore').strip('\x00')
            if name == '.rdata':
                self.rdata_section = section
            elif name == '.text':
                self.text_section = section
        return True

    def close(self):
        if self.pe:
            self.pe.close()

    def rva_to_offset(self, rva):
        """Convert RVA to file offset."""
        for section in self.pe.sections:
            if section.VirtualAddress <= rva < section.VirtualAddress + section.Misc_VirtualSize:
                return rva - section.VirtualAddress + section.PointerToRawData
        return None

    def read_dword(self, offset):
        """Read a 32-bit value at file offset."""
        if offset is None or offset + 4 > len(self.data):
            return None
        return struct.unpack_from('<I', self.data, offset)[0]

    def read_string_at_rva(self, rva, max_len=256):
        """Read a null-terminated string at RVA."""
        offset = self.rva_to_offset(rva)
        if offset is None or offset >= len(self.data):
            return None
        end = self.data.find(b'\x00', offset, offset + max_len)
        if end == -1:
            return None
        s = self.data[offset:end]
        try:
            return s.decode('utf-8', errors='strict')
        except:
            return None

    def read_string_at_offset(self, offset, max_len=256):
        """Read a null-terminated string at file offset."""
        if offset is None or offset >= len(self.data):
            return None
        end = self.data.find(b'\x00', offset, offset + max_len)
        if end == -1:
            return None
        s = self.data[offset:end]
        try:
            return s.decode('utf-8', errors='strict')
        except:
            return None

    # -----------------------------------------------------------------------
    # Section info
    # -----------------------------------------------------------------------
    def get_sections_info(self):
        """Return section information."""
        sections = []
        for s in self.pe.sections:
            name = s.Name.decode('utf-8', errors='ignore').strip('\x00')
            sections.append({
                "name": name,
                "virtual_address": hex(s.VirtualAddress),
                "virtual_size": hex(s.Misc_VirtualSize),
                "raw_size": hex(s.SizeOfRawData),
                "characteristics": hex(s.Characteristics),
            })
        return sections

    # -----------------------------------------------------------------------
    # Export table
    # -----------------------------------------------------------------------
    def get_exports(self):
        """List exported functions."""
        exports = []
        if hasattr(self.pe, 'DIRECTORY_ENTRY_EXPORT'):
            for exp in self.pe.DIRECTORY_ENTRY_EXPORT.symbols:
                name = exp.name.decode('utf-8', errors='ignore') if exp.name else f"Ordinal_{exp.ordinal}"
                exports.append({
                    "name": name,
                    "ordinal": exp.ordinal,
                    "rva": hex(exp.address),
                })
        return exports

    # -----------------------------------------------------------------------
    # RTTI extraction (MSVC x86)
    # -----------------------------------------------------------------------
    def extract_rtti_classes(self):
        """
        Extract RTTI class names from MSVC type descriptors.
        MSVC RTTI TypeDescriptor layout (x86):
            +0x00: pVFTable (pointer to type_info vftable)
            +0x04: spare (null)
            +0x08: name[] (mangled name string, e.g., ".?AVCCSPlayer@@")
        We search .rdata for the pattern ".?AV" which starts MSVC class names.
        """
        classes = []
        search = b".?AV"
        pos = 0
        while True:
            pos = self.data.find(search, pos)
            if pos == -1:
                break
            # Read the mangled name
            end = self.data.find(b'\x00', pos, pos + 512)
            if end == -1:
                pos += 4
                continue
            mangled = self.data[pos:end].decode('utf-8', errors='ignore')
            # Demangle: .?AVClassName@@namespace@@ -> ClassName (namespace)
            demangled = self._demangle_msvc(mangled)
            if demangled:
                # The TypeDescriptor starts 8 bytes before the name
                type_desc_offset = pos - 8
                # Compute RVA
                type_desc_rva = None
                for section in self.pe.sections:
                    raw_start = section.PointerToRawData
                    raw_end = raw_start + section.SizeOfRawData
                    if raw_start <= type_desc_offset < raw_end:
                        type_desc_rva = type_desc_offset - raw_start + section.VirtualAddress
                        break
                classes.append({
                    "mangled": mangled,
                    "demangled": demangled,
                    "file_offset": hex(type_desc_offset),
                    "rva": hex(type_desc_rva) if type_desc_rva else "unknown",
                })
            pos = end + 1
        return classes

    def _demangle_msvc(self, mangled):
        """Simple MSVC name demangling for .?AV names."""
        if not mangled.startswith(".?AV") and not mangled.startswith(".?AU"):
            return None
        # Remove .?AV or .?AU prefix and @@ suffix
        name = mangled[4:]
        # Remove trailing @@
        name = name.rstrip('@')
        # Split by @ for namespaces (reversed)
        parts = [p for p in name.split('@') if p]
        if not parts:
            return None
        # First part is class name, rest are namespaces (inner to outer)
        if len(parts) == 1:
            return parts[0]
        return f"{parts[0]} ({' :: '.join(reversed(parts[1:]))})"

    # -----------------------------------------------------------------------
    # Interface registration strings
    # -----------------------------------------------------------------------
    def find_interface_strings(self):
        """Find CreateInterface-style version strings in the binary."""
        results = []
        # Pattern: alphanumeric + version number, typically ending in 3 digits
        # e.g., "VClient018", "VEngineClient014", "VGUI_Surface031"
        pattern = re.compile(
            rb'(V[A-Za-z_]+\d{3}|'          # V-prefixed with 3-digit version
            rb'[A-Za-z]+Version\d{3}|'       # *Version### pattern
            rb'[A-Za-z_]+Client\d{3}|'       # *Client### pattern
            rb'GAMEEVENTSMANAGER\d{3}|'       # Specific known patterns
            rb'IEngine[A-Za-z]+\d{3}|'
            rb'Source2[A-Za-z]+\d{3})'
        )
        for match in pattern.finditer(self.data):
            s = match.group(0).decode('utf-8', errors='ignore')
            # Verify it's a real string (preceded by null or start)
            if match.start() > 0 and self.data[match.start() - 1] not in (0, 0x20, 0x0A):
                continue
            results.append({
                "interface": s,
                "file_offset": hex(match.start()),
                "known": s in KNOWN_INTERFACES,
                "sdk_name": KNOWN_INTERFACES.get(s, (None, None))[1],
            })
        return results

    # -----------------------------------------------------------------------
    # String extraction (interesting game strings)
    # -----------------------------------------------------------------------
    def extract_strings(self, min_len=6, patterns=None):
        """
        Extract readable strings, optionally filtered by patterns.
        patterns: list of regex patterns to match against.
        """
        results = []
        if patterns:
            compiled = [re.compile(p, re.IGNORECASE) for p in patterns]
        else:
            compiled = None

        # Find all printable ASCII strings
        current = []
        start_offset = 0
        for i, byte in enumerate(self.data):
            if 0x20 <= byte < 0x7F:
                if not current:
                    start_offset = i
                current.append(chr(byte))
            else:
                if len(current) >= min_len:
                    s = ''.join(current)
                    if compiled:
                        if any(p.search(s) for p in compiled):
                            results.append({"string": s, "offset": hex(start_offset)})
                    else:
                        results.append({"string": s, "offset": hex(start_offset)})
                current = []

        return results

    # -----------------------------------------------------------------------
    # Netvar/DataTable string search
    # -----------------------------------------------------------------------
    def find_netvar_strings(self):
        """Find netvar property names and datatable names in the binary."""
        results = {"datatables": [], "properties": []}
        # Search for DT_ prefixed datatable names
        dt_pattern = re.compile(rb'DT_[A-Za-z]+')
        for match in dt_pattern.finditer(self.data):
            s = match.group(0).decode('utf-8')
            # Verify null-terminated
            end_pos = match.end()
            if end_pos < len(self.data) and self.data[end_pos] == 0:
                if s not in [d["name"] for d in results["datatables"]]:
                    results["datatables"].append({
                        "name": s,
                        "offset": hex(match.start()),
                    })

        # Search for m_ prefixed property names
        prop_pattern = re.compile(rb'm_[a-zA-Z][a-zA-Z0-9_\[\]]{2,50}')
        seen = set()
        for match in prop_pattern.finditer(self.data):
            s = match.group(0).decode('utf-8', errors='ignore')
            end_pos = match.end()
            if end_pos < len(self.data) and self.data[end_pos] == 0:
                if s not in seen:
                    seen.add(s)
                    results["properties"].append({
                        "name": s,
                        "offset": hex(match.start()),
                    })

        return results

    # -----------------------------------------------------------------------
    # RTTI-based vtable discovery
    # -----------------------------------------------------------------------
    def find_vtables_for_class(self, class_name):
        """
        Find vtable(s) for a given class name by searching for its RTTI
        Complete Object Locator, then finding references to it.

        MSVC x86 vtable layout:
            vtable[-1] = pointer to RTTICompleteObjectLocator
            vtable[0]  = first virtual function
            vtable[1]  = second virtual function
            ...
        """
        # Step 1: Find the TypeDescriptor for this class
        search_name = f".?AV{class_name}@@".encode('utf-8')
        type_desc_offsets = []
        pos = 0
        while True:
            pos = self.data.find(search_name, pos)
            if pos == -1:
                break
            type_desc_offsets.append(pos - 8)  # TypeDescriptor starts 8 bytes before name
            pos += len(search_name)

        if not type_desc_offsets:
            # Try partial match
            search_partial = f".?AV{class_name}".encode('utf-8')
            pos = 0
            while True:
                pos = self.data.find(search_partial, pos)
                if pos == -1:
                    break
                end = self.data.find(b'\x00', pos, pos + 256)
                if end != -1:
                    full_name = self.data[pos:end].decode('utf-8', errors='ignore')
                    type_desc_offsets.append(pos - 8)
                pos += len(search_partial)

        results = []
        for td_offset in type_desc_offsets:
            # Convert TypeDescriptor offset to VA
            td_rva = None
            for section in self.pe.sections:
                raw_start = section.PointerToRawData
                raw_end = raw_start + section.SizeOfRawData
                if raw_start <= td_offset < raw_end:
                    td_rva = td_offset - raw_start + section.VirtualAddress
                    break
            if td_rva is None:
                continue

            td_va = self.image_base + td_rva
            # Read the mangled name for reporting
            name_bytes = self.data[td_offset + 8:td_offset + 8 + 128]
            name_end = name_bytes.find(b'\x00')
            mangled_name = name_bytes[:name_end].decode('utf-8', errors='ignore') if name_end > 0 else "?"

            # Step 2: Find CompleteObjectLocator that references this TypeDescriptor
            # COL layout (x86):
            #   +0x00: signature (0)
            #   +0x04: offset (0 for primary vtable)
            #   +0x08: cdOffset (0)
            #   +0x0C: pTypeDescriptor (VA of TypeDescriptor)
            #   +0x10: pClassDescriptor (VA of ClassHierarchyDescriptor)
            td_va_bytes = struct.pack('<I', td_va)
            col_search_pos = 0
            while True:
                col_search_pos = self.data.find(td_va_bytes, col_search_pos)
                if col_search_pos == -1:
                    break
                # Check if this looks like a COL (the TD pointer is at offset +0x0C)
                col_offset = col_search_pos - 0x0C
                if col_offset < 0:
                    col_search_pos += 4
                    continue
                # Read signature at col_offset
                sig = self.read_dword(col_offset)
                if sig != 0:
                    col_search_pos += 4
                    continue

                # This looks like a valid COL
                col_rva = None
                for section in self.pe.sections:
                    raw_start = section.PointerToRawData
                    raw_end = raw_start + section.SizeOfRawData
                    if raw_start <= col_offset < raw_end:
                        col_rva = col_offset - raw_start + section.VirtualAddress
                        break

                if col_rva is None:
                    col_search_pos += 4
                    continue

                col_va = self.image_base + col_rva

                # Step 3: Find vtable[-1] that points to this COL
                col_va_bytes = struct.pack('<I', col_va)
                vtable_search_pos = 0
                while True:
                    vtable_search_pos = self.data.find(col_va_bytes, vtable_search_pos)
                    if vtable_search_pos == -1:
                        break

                    # vtable[0] starts right after this pointer
                    vtable_start_offset = vtable_search_pos + 4
                    vtable_rva = None
                    for section in self.pe.sections:
                        raw_start = section.PointerToRawData
                        raw_end = raw_start + section.SizeOfRawData
                        if raw_start <= vtable_start_offset < raw_end:
                            vtable_rva = vtable_start_offset - raw_start + section.VirtualAddress
                            break

                    if vtable_rva is not None:
                        # Count vtable entries (consecutive valid .text pointers)
                        vfunc_count = self._count_vtable_entries(vtable_start_offset)
                        vfuncs = self._read_vtable_entries(vtable_start_offset, min(vfunc_count, 500))

                        vt_offset_in_class = self.read_dword(col_offset + 4)  # offset field in COL

                        results.append({
                            "class": mangled_name,
                            "vtable_rva": hex(vtable_rva),
                            "vtable_va": hex(self.image_base + vtable_rva),
                            "vfunc_count": vfunc_count,
                            "offset_in_class": vt_offset_in_class,
                            "col_rva": hex(col_rva),
                            "vfuncs": vfuncs,
                        })

                    vtable_search_pos += 4

                col_search_pos += 4

        return results

    def _count_vtable_entries(self, file_offset):
        """Count consecutive valid function pointers in a vtable."""
        if not self.text_section:
            return 0
        text_start = self.image_base + self.text_section.VirtualAddress
        text_end = text_start + self.text_section.Misc_VirtualSize
        count = 0
        pos = file_offset
        while pos + 4 <= len(self.data):
            val = struct.unpack_from('<I', self.data, pos)[0]
            # Check if it's a valid .text address
            if text_start <= val < text_end:
                count += 1
                pos += 4
            else:
                break
        return count

    def _read_vtable_entries(self, file_offset, count):
        """Read vtable function pointers."""
        entries = []
        for i in range(count):
            val = self.read_dword(file_offset + i * 4)
            if val:
                entries.append({
                    "index": i,
                    "va": hex(val),
                    "rva": hex(val - self.image_base),
                })
        return entries

    # -----------------------------------------------------------------------
    # ConVar search
    # -----------------------------------------------------------------------
    def find_convars(self):
        """Find ConVar name strings (command-line variables)."""
        results = []
        # ConVars are typically short lowercase strings with underscores
        # Look for known prefixes
        prefixes = [b'sv_', b'cl_', b'mp_', b'mat_', b'r_', b'net_', b'bot_',
                    b'weapon_', b'spec_', b'hud_']
        seen = set()
        for prefix in prefixes:
            pos = 0
            while True:
                pos = self.data.find(prefix, pos)
                if pos == -1:
                    break
                # Read the full string
                end = self.data.find(b'\x00', pos, pos + 128)
                if end != -1:
                    s = self.data[pos:end].decode('utf-8', errors='ignore')
                    # Validate: should be alphanumeric with underscores
                    if re.match(r'^[a-z_][a-z0-9_]+$', s) and s not in seen:
                        seen.add(s)
                        results.append({"name": s, "offset": hex(pos)})
                pos += len(prefix)
        return sorted(results, key=lambda x: x["name"])


# ============================================================================
# SDK Cross-Reference
# ============================================================================

def cross_reference_vtables(analyzer, class_name, expected_indices):
    """Compare our SDK's vtable indices against the actual binary."""
    vtables = analyzer.find_vtables_for_class(class_name)
    if not vtables:
        return {"class": class_name, "status": "NOT FOUND", "vtables": []}

    results = {"class": class_name, "status": "FOUND", "vtables": []}
    for vt in vtables:
        vt_info = {
            "rva": vt["vtable_rva"],
            "offset_in_class": vt["offset_in_class"],
            "total_vfuncs": vt["vfunc_count"],
            "checks": [],
        }
        for func_name, expected_idx in sorted(expected_indices.items(), key=lambda x: x[1]):
            if expected_idx < vt["vfunc_count"]:
                vfunc = vt["vfuncs"][expected_idx] if expected_idx < len(vt["vfuncs"]) else None
                vt_info["checks"].append({
                    "function": func_name,
                    "expected_index": expected_idx,
                    "valid": vfunc is not None,
                    "address": vfunc["va"] if vfunc else "N/A",
                })
            else:
                vt_info["checks"].append({
                    "function": func_name,
                    "expected_index": expected_idx,
                    "valid": False,
                    "address": f"OUT OF RANGE (vtable has {vt['vfunc_count']} entries)",
                })
        results["vtables"].append(vt_info)
    return results


# ============================================================================
# Output Formatting
# ============================================================================

def print_header(text):
    print(f"\n{'='*70}")
    print(f"  {text}")
    print(f"{'='*70}")

def print_subheader(text):
    print(f"\n--- {text} ---")

def print_table(headers, rows, widths=None):
    """Simple ASCII table printer."""
    if not widths:
        widths = [max(len(str(h)), max((len(str(r[i])) for r in rows), default=0))
                  for i, h in enumerate(headers)]
    fmt = "  " + " | ".join(f"{{:<{w}}}" for w in widths)
    print(fmt.format(*headers))
    print("  " + "-+-".join("-" * w for w in widths))
    for row in rows:
        print(fmt.format(*[str(r) for r in row]))


# ============================================================================
# Main Analysis Functions
# ============================================================================

def analyze_dll_overview(name):
    """Print overview of a DLL."""
    path = DLL_PATHS.get(name)
    if not path:
        print(f"Unknown DLL: {name}")
        return

    print_header(f"{name}.dll — {path}")
    analyzer = DLLAnalyzer(name, path)
    if not analyzer.load():
        return

    # Basic info
    print(f"  Image Base:    {hex(analyzer.image_base)}")
    print(f"  Entry Point:   {hex(analyzer.pe.OPTIONAL_HEADER.AddressOfEntryPoint)}")
    print(f"  File Size:     {os.path.getsize(path):,} bytes")
    print(f"  Sections:      {len(analyzer.pe.sections)}")

    # Sections
    print_subheader("Sections")
    for s in analyzer.get_sections_info():
        print(f"  {s['name']:<10} VA={s['virtual_address']:<10} VSize={s['virtual_size']:<10} RawSize={s['raw_size']}")

    # Exports
    exports = analyzer.get_exports()
    if exports:
        print_subheader(f"Exports ({len(exports)})")
        for e in exports[:20]:
            print(f"  [{e['ordinal']:>3}] {e['name']:<40} RVA={e['rva']}")
        if len(exports) > 20:
            print(f"  ... and {len(exports) - 20} more")

    # RTTI classes
    classes = analyzer.extract_rtti_classes()
    print_subheader(f"RTTI Classes ({len(classes)})")
    for c in classes[:30]:
        print(f"  {c['demangled']:<60} RVA={c['rva']}")
    if len(classes) > 30:
        print(f"  ... and {len(classes) - 30} more")

    # Interfaces
    interfaces = analyzer.find_interface_strings()
    if interfaces:
        print_subheader(f"Interface Strings ({len(interfaces)})")
        for iface in interfaces:
            status = f"[SDK: {iface['sdk_name']}]" if iface['known'] else "[NOT IN SDK]"
            print(f"  {iface['interface']:<35} {status}")

    analyzer.close()


def analyze_interfaces_all():
    """Find all interface registrations across all DLLs."""
    print_header("Interface Registration Scan")

    all_interfaces = {}
    for name, path in DLL_PATHS.items():
        analyzer = DLLAnalyzer(name, path)
        if not analyzer.load():
            continue
        interfaces = analyzer.find_interface_strings()
        for iface in interfaces:
            iface["dll"] = name
            all_interfaces[iface["interface"]] = iface
        analyzer.close()

    # Print organized by status
    print_subheader("Interfaces Used by Our SDK")
    rows = []
    for version, (dll, sdk_name) in sorted(KNOWN_INTERFACES.items()):
        found = version in all_interfaces
        in_sdk = sdk_name in ["IClientEntityList", "IBaseClientDLL", "IVEngineClient",
                              "CInputSystem", "IVModelInfo", "IEngineTrace", "ICvar", "ISurface"]
        status = "ACTIVE" if in_sdk else "NOT YET"
        rows.append([version, dll, sdk_name, "FOUND" if found else "MISSING", status])
    print_table(["Interface Version", "DLL", "SDK Class", "In Binary", "In Our SDK"], rows)

    print_subheader("Other Interfaces Found (not in our SDK)")
    for iface_name, info in sorted(all_interfaces.items()):
        if iface_name not in KNOWN_INTERFACES:
            print(f"  {info['dll']:<20} {iface_name}")


def analyze_netvars(dll_name="client"):
    """Find netvar-related strings in the specified DLL."""
    print_header(f"NetVar Analysis — {dll_name}.dll")
    path = DLL_PATHS.get(dll_name)
    analyzer = DLLAnalyzer(dll_name, path)
    if not analyzer.load():
        return

    netvar_data = analyzer.find_netvar_strings()

    print_subheader(f"DataTables ({len(netvar_data['datatables'])})")
    for dt in sorted(netvar_data["datatables"], key=lambda x: x["name"]):
        known = "***" if dt["name"] in NETVAR_TABLES else "   "
        print(f"  {known} {dt['name']:<35} offset={dt['offset']}")

    print_subheader(f"NetVar Properties ({len(netvar_data['properties'])} unique)")
    # Group by interesting categories
    interesting = [
        "m_hActiveWeapon", "m_flNextPrimaryAttack", "m_iItemDefinitionIndex",
        "m_ArmorValue", "m_bHasHelmet", "m_flSimulationTime", "m_iClip1",
        "m_nTickBase", "m_bSpotted", "m_iTeamNum", "m_fFlags", "m_bIsScoped",
        "m_vecVelocity", "m_vecViewOffset", "m_aimPunchAngle", "m_angEyeAngles",
        "m_bClientSideAnimation", "m_flPoseParameter", "m_iHealth",
        "m_flLowerBodyYawTarget", "m_nFallbackPaintKit", "m_nFallbackSeed",
        "m_flFallbackWear", "m_iItemIDHigh", "m_iEntityQuality",
        "m_bGunGameImmunity", "m_flFlashDuration", "m_flFlashMaxAlpha",
        "m_iObserverMode", "m_hObserverTarget", "m_bHasDefuser",
        "m_flNextAttack", "m_nModelIndex",
    ]

    print_subheader("Key Properties (for our features)")
    found_props = {p["name"]: p for p in netvar_data["properties"]}
    for prop_name in interesting:
        # Check exact or prefix match
        matches = [p for p in netvar_data["properties"]
                   if p["name"] == prop_name or p["name"].startswith(prop_name)]
        if matches:
            for m in matches:
                in_sdk = "(IN SDK)" if prop_name in ["m_bSpotted", "m_iTeamNum", "m_fFlags",
                    "m_bIsScoped", "m_vecVelocity", "m_vecViewOffset", "m_aimPunchAngle"] else "(NEEDED)"
                print(f"  [FOUND] {m['name']:<35} {in_sdk}")
        else:
            print(f"  [MISS]  {prop_name:<35}")

    analyzer.close()


def analyze_vtable_xref():
    """Cross-reference our SDK vtable indices against the binary."""
    print_header("SDK VTable Cross-Reference")

    # CCSPlayer (entity vtable) is in client.dll
    analyzer = DLLAnalyzer("client", DLL_PATHS["client"])
    if not analyzer.load():
        return

    # Check key classes
    classes_to_check = [
        ("CCSPlayer", "gEntity"),
        ("C_CSPlayer", "gEntity"),  # Client-side class name
    ]

    for class_name, sdk_key in classes_to_check:
        print_subheader(f"Searching for {class_name} vtable...")
        vtables = analyzer.find_vtables_for_class(class_name)
        if vtables:
            for vt in vtables:
                print(f"  Found: RVA={vt['vtable_rva']}, {vt['vfunc_count']} entries, "
                      f"class offset={vt['offset_in_class']}")

                # Check our SDK indices
                if sdk_key in SDK_VTABLE_INDICES:
                    expected = SDK_VTABLE_INDICES[sdk_key]
                    max_idx = max(expected.values())
                    if max_idx >= vt["vfunc_count"]:
                        print(f"  WARNING: Our max vtable index ({max_idx}) >= "
                              f"actual vtable size ({vt['vfunc_count']})")
                    else:
                        print(f"  OK: All SDK indices ({max_idx} max) within "
                              f"vtable bounds ({vt['vfunc_count']})")

                    # Show specific function addresses
                    for func_name, idx in sorted(expected.items(), key=lambda x: x[1]):
                        if idx < len(vt["vfuncs"]):
                            addr = vt["vfuncs"][idx]["va"]
                            print(f"    [{idx:>3}] {func_name:<40} -> {addr}")
                        else:
                            print(f"    [{idx:>3}] {func_name:<40} -> OUT OF BOUNDS!")
        else:
            print(f"  Not found in binary")

    # Check engine interfaces
    analyzer.close()
    analyzer = DLLAnalyzer("engine", DLL_PATHS["engine"])
    if not analyzer.load():
        return

    for class_name in ["IVEngineClient", "CEngineClient", "IEngineTrace", "CEngineTraceClient"]:
        print_subheader(f"Searching for {class_name} vtable...")
        vtables = analyzer.find_vtables_for_class(class_name)
        if vtables:
            for vt in vtables:
                print(f"  Found: RVA={vt['vtable_rva']}, {vt['vfunc_count']} entries")

    analyzer.close()


def find_class_info(class_name):
    """Search all DLLs for a specific class's RTTI and vtable."""
    print_header(f"Class Search: {class_name}")

    for dll_name, path in DLL_PATHS.items():
        analyzer = DLLAnalyzer(dll_name, path)
        if not analyzer.load():
            continue

        # Search RTTI
        classes = analyzer.extract_rtti_classes()
        matches = [c for c in classes if class_name.lower() in c["demangled"].lower()]
        if matches:
            print_subheader(f"RTTI matches in {dll_name}.dll")
            for c in matches:
                print(f"  {c['demangled']:<50} RVA={c['rva']}")

        # Search vtables
        vtables = analyzer.find_vtables_for_class(class_name)
        if vtables:
            print_subheader(f"VTables in {dll_name}.dll")
            for vt in vtables:
                print(f"  Class: {vt['class']}")
                print(f"  VTable RVA: {vt['vtable_rva']}")
                print(f"  VFunc Count: {vt['vfunc_count']}")
                print(f"  Offset in class: {vt['offset_in_class']}")
                if vt['vfunc_count'] <= 50:
                    for vf in vt["vfuncs"]:
                        print(f"    [{vf['index']:>3}] {vf['va']}")
                else:
                    print(f"    (showing first 20 of {vt['vfunc_count']})")
                    for vf in vt["vfuncs"][:20]:
                        print(f"    [{vf['index']:>3}] {vf['va']}")

        analyzer.close()


def analyze_strings(dll_name, patterns=None):
    """Extract interesting strings from a DLL."""
    path = DLL_PATHS.get(dll_name)
    if not path:
        print(f"Unknown DLL: {dll_name}")
        return

    print_header(f"String Analysis — {dll_name}.dll")
    analyzer = DLLAnalyzer(dll_name, path)
    if not analyzer.load():
        return

    if not patterns:
        # Default interesting patterns
        patterns = [
            r'CreateInterface',
            r'CCSPlayer',
            r'AnimState',
            r'SetupBones',
            r'm_fl[A-Z]',
            r'Weapon_',
            r'models/player',
            r'materials/',
            r'player_hurt',
            r'player_death',
            r'round_start',
            r'UpdateClientSideAnimation',
            r'GetEyePosition',
            r'CalcView',
            r'thirdperson',
            r'sv_cheats',
            r'cam_ideal',
        ]

    strings = analyzer.extract_strings(min_len=8, patterns=patterns)
    print(f"  Found {len(strings)} matching strings\n")
    for s in strings[:200]:
        print(f"  {s['offset']}: {s['string'][:100]}")
    if len(strings) > 200:
        print(f"\n  ... and {len(strings) - 200} more")

    analyzer.close()


def full_analysis():
    """Run comprehensive analysis on all DLLs."""
    # Overview of each DLL
    for name in ["client", "engine", "vstdlib", "materialsystem", "inputsystem"]:
        analyze_dll_overview(name)

    # Interface scan
    analyze_interfaces_all()

    # Netvars
    analyze_netvars("client")
    analyze_netvars("server")

    # VTable cross-reference
    analyze_vtable_xref()

    # Key class searches
    for cls in ["CCSGOPlayerAnimState", "CCSPlayer", "CBaseCombatWeapon", "CInput"]:
        find_class_info(cls)


def export_analysis(output_dir):
    """Export comprehensive analysis as JSON files for cross-referencing."""
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    print_header("Exporting Static Analysis")

    # 1. RTTI class lists per DLL
    all_rtti = {}
    for dll_name, path in DLL_PATHS.items():
        analyzer = DLLAnalyzer(dll_name, path)
        if not analyzer.load():
            continue
        classes = analyzer.extract_rtti_classes()
        all_rtti[dll_name] = [c["demangled"] for c in classes]
        analyzer.close()

    rtti_path = output_dir / "rtti_classes.json"
    with open(rtti_path, 'w') as f:
        json.dump(all_rtti, f, indent=2)
    total = sum(len(v) for v in all_rtti.values())
    print(f"  [+] RTTI classes: {total} classes across {len(all_rtti)} DLLs -> {rtti_path}")

    # 2. Interfaces
    all_interfaces = {}
    for dll_name, path in DLL_PATHS.items():
        analyzer = DLLAnalyzer(dll_name, path)
        if not analyzer.load():
            continue
        interfaces = analyzer.find_interface_strings()
        for iface in interfaces:
            all_interfaces[iface["interface"]] = {
                "dll": dll_name,
                "offset": iface["file_offset"],
                "in_sdk": iface["known"],
                "sdk_class": iface.get("sdk_name"),
            }
        analyzer.close()

    iface_path = output_dir / "interfaces.json"
    with open(iface_path, 'w') as f:
        json.dump(all_interfaces, f, indent=2)
    print(f"  [+] Interfaces: {len(all_interfaces)} found -> {iface_path}")

    # 3. NetVar properties from client.dll
    analyzer = DLLAnalyzer("client", DLL_PATHS["client"])
    if analyzer.load():
        netvar_data = analyzer.find_netvar_strings()
        netvars_export = {
            "datatables": [dt["name"] for dt in sorted(netvar_data["datatables"], key=lambda x: x["name"])],
            "properties": {p["name"]: p["offset"] for p in netvar_data["properties"]},
        }
        analyzer.close()
        netvar_path = output_dir / "netvars_client.json"
        with open(netvar_path, 'w') as f:
            json.dump(netvars_export, f, indent=2)
        print(f"  [+] NetVars: {len(netvars_export['datatables'])} tables, "
              f"{len(netvars_export['properties'])} properties -> {netvar_path}")

    # 4. VTable maps for key classes
    vtable_export = {}

    # Client classes
    analyzer = DLLAnalyzer("client", DLL_PATHS["client"])
    if analyzer.load():
        for cls in ["C_CSPlayer", "C_BaseCombatWeapon", "C_WeaponCSBase",
                     "C_WeaponCSBaseGun", "C_BaseAnimating", "CInput",
                     "C_BasePlayer", "C_BaseEntity"]:
            vtables = analyzer.find_vtables_for_class(cls)
            for vt in vtables:
                if vt["offset_in_class"] == 0 and vt["vfunc_count"] > 50:
                    vtable_export[cls] = {
                        "dll": "client",
                        "rva": vt["vtable_rva"],
                        "vfunc_count": vt["vfunc_count"],
                        "vfuncs": {str(vf["index"]): vf["va"] for vf in vt["vfuncs"]},
                    }
                    break
        analyzer.close()

    # Engine classes
    analyzer = DLLAnalyzer("engine", DLL_PATHS["engine"])
    if analyzer.load():
        for cls in ["CEngineClient", "CEngineTraceClient"]:
            vtables = analyzer.find_vtables_for_class(cls)
            for vt in vtables:
                if vt["vfunc_count"] > 10:
                    vtable_export[cls] = {
                        "dll": "engine",
                        "rva": vt["vtable_rva"],
                        "vfunc_count": vt["vfunc_count"],
                        "vfuncs": {str(vf["index"]): vf["va"] for vf in vt["vfuncs"]},
                    }
                    break
        analyzer.close()

    vtable_path = output_dir / "vtables.json"
    with open(vtable_path, 'w') as f:
        json.dump(vtable_export, f, indent=2)
    print(f"  [+] VTables: {len(vtable_export)} class vtables mapped -> {vtable_path}")

    # 5. ConVars from engine + client
    all_convars = {}
    for dll_name in ["engine", "client", "server"]:
        analyzer = DLLAnalyzer(dll_name, DLL_PATHS[dll_name])
        if analyzer.load():
            convars = analyzer.find_convars()
            for cv in convars:
                if cv["name"] not in all_convars:
                    all_convars[cv["name"]] = dll_name
            analyzer.close()

    convar_path = output_dir / "convars.json"
    with open(convar_path, 'w') as f:
        json.dump(all_convars, f, indent=2)
    print(f"  [+] ConVars: {len(all_convars)} unique -> {convar_path}")

    # 6. SDK validation summary
    validation = {
        "vtable_checks": {},
        "interfaces_status": {},
        "netvars_status": {},
    }

    # VTable validation
    sdk_checks = [
        ("C_CSPlayer", "client", {
            "setModelIndex[75]": 75, "health[122]": 122, "isAlive[156]": 156,
            "isPlayer[158]": 158, "updateClientSideAnimation[224]": 224,
            "getEyePosition[285]": 285, "getObserverMode[294]": 294,
        }),
        ("C_WeaponCSBase", "client", {
            "getSpread[453]": 453, "getWeaponType[455]": 455, "getWeaponData[461]": 461,
            "getInaccuracy[483]": 483, "updateInaccuracyPenalty[484]": 484,
        }),
        ("CEngineClient", "engine", {
            "GetScreenSize[5]": 5, "GetPlayerInfo[8]": 8, "GetLocalPlayer[12]": 12,
            "IsInGame[26]": 26, "WorldToScreenMatrix[37]": 37,
        }),
    ]

    for cls, dll, indices in sdk_checks:
        cls_key = cls
        if cls_key in vtable_export:
            vt = vtable_export[cls_key]
            checks = {}
            for name, idx in indices.items():
                checks[name] = "VALID" if idx < vt["vfunc_count"] else "OUT_OF_BOUNDS"
            validation["vtable_checks"][cls] = {
                "total_vfuncs": vt["vfunc_count"],
                "checks": checks,
            }

    # Interface validation
    for version, (dll, sdk_name) in KNOWN_INTERFACES.items():
        validation["interfaces_status"][version] = {
            "dll": dll,
            "sdk_class": sdk_name,
            "found": version in all_interfaces,
        }

    valid_path = output_dir / "sdk_validation.json"
    with open(valid_path, 'w') as f:
        json.dump(validation, f, indent=2)
    print(f"  [+] SDK validation summary -> {valid_path}")

    # 7. Summary index file
    summary = {
        "generated_by": "analyze_dll.py (static analysis)",
        "csgo_path": str(CSGO_BIN.parent),
        "files": {
            "rtti_classes.json": f"{total} RTTI classes from {len(all_rtti)} DLLs",
            "interfaces.json": f"{len(all_interfaces)} interface version strings",
            "netvars_client.json": f"{len(netvars_export['datatables'])} datatables, {len(netvars_export['properties'])} properties",
            "vtables.json": f"{len(vtable_export)} class vtable maps",
            "convars.json": f"{len(all_convars)} console variables",
            "sdk_validation.json": "SDK vtable/interface/netvar validation results",
        },
        "dll_sizes": {},
    }
    for name, path in DLL_PATHS.items():
        if path.exists():
            summary["dll_sizes"][name] = os.path.getsize(path)

    index_path = output_dir / "index.json"
    with open(index_path, 'w') as f:
        json.dump(summary, f, indent=2)
    print(f"  [+] Index -> {index_path}")
    print(f"\n  Export complete: {output_dir}")


# ============================================================================
# Entry Point
# ============================================================================

def main():
    parser = argparse.ArgumentParser(description="CS:GO DLL Static Analysis Toolkit")
    parser.add_argument("--dll", help="Analyze a specific DLL (client, engine, etc.)")
    parser.add_argument("--interfaces", action="store_true", help="Scan all DLLs for interface registrations")
    parser.add_argument("--netvars", nargs="?", const="client", help="Find netvar strings (default: client.dll)")
    parser.add_argument("--vtable", help="Find vtable for a class name")
    parser.add_argument("--xref", action="store_true", help="Cross-reference SDK vtable indices")
    parser.add_argument("--find-class", help="Search all DLLs for a class")
    parser.add_argument("--strings", help="Extract interesting strings from a DLL")
    parser.add_argument("--convars", help="Find ConVar strings in a DLL")
    parser.add_argument("--all", action="store_true", help="Full comprehensive analysis")
    parser.add_argument("--export", nargs="?", const="external_sources/analysis/static",
                        help="Export analysis as JSON to directory (default: external_sources/analysis/static)")
    parser.add_argument("--json", action="store_true", help="Output as JSON")

    args = parser.parse_args()

    if args.export:
        # Resolve path relative to script's project root
        export_path = Path(args.export)
        if not export_path.is_absolute():
            export_path = Path(__file__).parent.parent / export_path
        export_analysis(export_path)
    elif args.all:
        full_analysis()
    elif args.dll:
        analyze_dll_overview(args.dll)
    elif args.interfaces:
        analyze_interfaces_all()
    elif args.netvars is not None:
        analyze_netvars(args.netvars)
    elif args.xref:
        analyze_vtable_xref()
    elif args.find_class:
        find_class_info(args.find_class)
    elif args.strings:
        analyze_strings(args.strings)
    elif args.convars:
        path = DLL_PATHS.get(args.convars)
        if path:
            print_header(f"ConVars in {args.convars}.dll")
            analyzer = DLLAnalyzer(args.convars, path)
            if analyzer.load():
                convars = analyzer.find_convars()
                print(f"  Found {len(convars)} ConVars\n")
                for cv in convars:
                    print(f"  {cv['name']}")
                analyzer.close()
    elif args.vtable:
        # Search all DLLs
        find_class_info(args.vtable)
    else:
        # Default: quick summary
        print("CS:GO DLL Analysis Toolkit")
        print("Use --help for full options, or try:")
        print("  --interfaces      Scan for interface version strings")
        print("  --netvars         Find netvar properties in client.dll")
        print("  --xref            Validate SDK vtable indices")
        print("  --find-class X    Search for class RTTI + vtable")
        print("  --dll client      Overview of a specific DLL")
        print("  --all             Full comprehensive analysis")
        print()
        # Quick sanity check
        print("DLL Status:")
        for name, path in DLL_PATHS.items():
            exists = path.exists()
            size = f"{os.path.getsize(path):>12,} bytes" if exists else "NOT FOUND"
            print(f"  {name:<20} {size}   {path}")

if __name__ == "__main__":
    main()
