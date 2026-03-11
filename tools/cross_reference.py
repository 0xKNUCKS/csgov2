#!/usr/bin/env python3
"""
Cross-reference static analysis (from DLL files) with runtime dumps (from injected cheat).
Compares netvar offsets, vtable sizes, interface addresses, and module bases.

Usage:
    python tools/cross_reference.py                # Compare all available data
    python tools/cross_reference.py --netvars      # Focus on netvar offset comparison
    python tools/cross_reference.py --vtables      # Focus on vtable comparison
"""

import argparse
import json
import os
import sys
from pathlib import Path

# Paths
PROJECT_ROOT = Path(__file__).parent.parent
STATIC_DIR = PROJECT_ROOT / "external_sources" / "analysis" / "static"
DYNAMIC_DIR = Path(os.path.expanduser("~")) / "Documents" / "CSGO_v2_Analysis" / "dynamic"

def load_json(path):
    """Load a JSON file, return None if not found."""
    if not path.exists():
        return None
    with open(path) as f:
        return json.load(f)


def compare_netvars():
    """Compare static netvar string positions vs runtime resolved offsets."""
    print("\n" + "=" * 60)
    print("  NetVar Cross-Reference (Static vs Runtime)")
    print("=" * 60)

    # Runtime data
    runtime = load_json(DYNAMIC_DIR / "netvars.json")
    entity_info = load_json(DYNAMIC_DIR / "entity_info.json")

    if not runtime:
        print("  [!] No runtime netvar dump found. Run analysis::DumpAll() in-game first.")
        print(f"      Expected at: {DYNAMIC_DIR / 'netvars.json'}")
        return

    # Build a flat map of table.prop -> offset from runtime data
    runtime_offsets = {}
    for cls in runtime.get("classes", []):
        table = cls.get("table", "")
        for prop in cls.get("props", []):
            key = f"{table}.{prop['name']}"
            runtime_offsets[key] = prop["offset"]
            # Also flatten nested props
            for nested in prop.get("props", []):
                nested_key = f"{table}.{nested['name']}"
                # Nested offset = parent offset + nested offset
                runtime_offsets[nested_key] = prop["offset"] + nested["offset"]

    print(f"\n  Runtime: {len(runtime_offsets)} resolved offsets from {runtime.get('total_classes', '?')} classes")

    # Show our SDK's resolved offsets from entity_info
    if entity_info and "resolved_offsets" in entity_info:
        print("\n  SDK Resolved Offsets (from runtime):")
        print(f"  {'Netvar':<30} {'Offset':<12} {'Hex':<12}")
        print(f"  {'-'*30} {'-'*12} {'-'*12}")
        for entry in entity_info["resolved_offsets"]:
            name = entry["name"]
            dec = entry["decimal"]
            hex_val = entry["offset"]
            print(f"  {name:<30} {dec:<12} {hex_val:<12}")

    # Key netvars we care about for features
    key_netvars = {
        "DT_BaseEntity.m_bSpotted": "Radar Hack",
        "DT_BaseEntity.m_iTeamNum": "Team check",
        "DT_BasePlayer.m_fFlags": "Bunny hop",
        "DT_CSPlayer.m_bIsScoped": "Scope check",
        "DT_BasePlayer.deadflag": "Alive check",
        "DT_BasePlayer.m_vecVelocity[0]": "Auto-stop",
        "DT_BasePlayer.m_aimPunchAngle": "RCS",
        "DT_BaseCombatCharacter.m_hActiveWeapon": "Weapon detection",
        "DT_BaseCombatWeapon.m_flNextPrimaryAttack": "Fire rate",
        "DT_BaseAttributableItem.m_iItemDefinitionIndex": "Weapon ID",
        "DT_CSPlayer.m_ArmorValue": "Damage calc",
        "DT_CSPlayer.m_bHasHelmet": "Damage calc",
        "DT_BaseEntity.m_flSimulationTime": "Backtracking",
        "DT_BaseCombatWeapon.m_iClip1": "Ammo check",
        "DT_BasePlayer.m_nTickBase": "Fire rate",
        "DT_CSPlayer.m_angEyeAngles[0]": "AnimState",
        "DT_BaseAnimating.m_bClientSideAnimation": "3rd person",
        "DT_CSPlayer.m_flLowerBodyYawTarget": "Anti-aim",
    }

    print("\n  Key NetVar Offsets (runtime):")
    print(f"  {'Table.Property':<50} {'Offset':<10} {'Feature'}")
    print(f"  {'-'*50} {'-'*10} {'-'*20}")
    for key, feature in key_netvars.items():
        offset = runtime_offsets.get(key)
        if offset is not None:
            print(f"  {key:<50} {offset:<10} {feature}")
        else:
            print(f"  {key:<50} {'NOT FOUND':<10} {feature}")


def compare_vtables():
    """Compare static vtable sizes vs runtime vtable sizes."""
    print("\n" + "=" * 60)
    print("  VTable Cross-Reference (Static vs Runtime)")
    print("=" * 60)

    static_vt = load_json(STATIC_DIR / "vtables.json")
    runtime_ifaces = load_json(DYNAMIC_DIR / "interfaces.json")

    if static_vt:
        print("\n  Static VTable Analysis:")
        print(f"  {'Class':<30} {'VFuncs':<10} {'RVA'}")
        print(f"  {'-'*30} {'-'*10} {'-'*15}")
        for cls, info in sorted(static_vt.items()):
            print(f"  {cls:<30} {info['vfunc_count']:<10} {info['rva']}")
    else:
        print("  [!] No static vtable data. Run: python tools/analyze_dll.py --export")

    if runtime_ifaces:
        print("\n  Runtime Interface VTables:")
        print(f"  {'Interface':<30} {'Address':<14} {'VTable':<14} {'VFuncs'}")
        print(f"  {'-'*30} {'-'*14} {'-'*14} {'-'*8}")
        for iface in runtime_ifaces.get("interfaces", []):
            name = iface["name"]
            addr = iface["address"]
            vt = iface.get("vtable", "?")
            count = iface.get("vfunc_count", "?")
            print(f"  {name:<30} {addr:<14} {vt:<14} {count}")
    else:
        print("  [!] No runtime interface data. Run analysis::DumpAll() in-game first.")


def compare_modules():
    """Show module bases from runtime for RVA calculation."""
    print("\n" + "=" * 60)
    print("  Module Base Addresses (Runtime)")
    print("=" * 60)

    entity_info = load_json(DYNAMIC_DIR / "entity_info.json")
    if not entity_info or "modules" not in entity_info:
        print("  [!] No runtime module data. Run analysis::DumpAll() in-game first.")
        return

    print(f"\n  {'Module':<30} {'Base Address'}")
    print(f"  {'-'*30} {'-'*14}")
    for mod in entity_info["modules"]:
        print(f"  {mod['name']:<30} {mod['base']}")

    print("\n  Use these bases to convert static RVAs to runtime addresses:")
    print("  runtime_addr = module_base + static_rva")


def full_comparison():
    """Run all comparisons."""
    compare_modules()
    compare_netvars()
    compare_vtables()

    # Summary
    print("\n" + "=" * 60)
    print("  Data Availability Summary")
    print("=" * 60)

    static_files = [
        ("rtti_classes.json", "RTTI class hierarchy"),
        ("interfaces.json", "Interface version strings"),
        ("netvars_client.json", "NetVar property names (static)"),
        ("vtables.json", "VTable maps"),
        ("convars.json", "ConVar names"),
        ("sdk_validation.json", "SDK validation results"),
    ]

    print(f"\n  Static Analysis ({STATIC_DIR}):")
    for fname, desc in static_files:
        exists = (STATIC_DIR / fname).exists()
        status = "OK" if exists else "MISSING"
        print(f"  [{status:>7}] {fname:<30} {desc}")

    dynamic_files = [
        ("netvars.json", "Runtime netvar offsets"),
        ("interfaces.json", "Live interface pointers"),
        ("entity_info.json", "Entity structure info"),
        ("convars.json", "ConVar values"),
    ]

    print(f"\n  Dynamic Analysis ({DYNAMIC_DIR}):")
    for fname, desc in dynamic_files:
        exists = (DYNAMIC_DIR / fname).exists()
        status = "OK" if exists else "MISSING"
        print(f"  [{status:>7}] {fname:<30} {desc}")


def main():
    parser = argparse.ArgumentParser(description="Cross-reference static and runtime analysis")
    parser.add_argument("--netvars", action="store_true", help="Compare netvar offsets")
    parser.add_argument("--vtables", action="store_true", help="Compare vtable sizes")
    parser.add_argument("--modules", action="store_true", help="Show module bases")

    args = parser.parse_args()

    if args.netvars:
        compare_netvars()
    elif args.vtables:
        compare_vtables()
    elif args.modules:
        compare_modules()
    else:
        full_comparison()


if __name__ == "__main__":
    main()
