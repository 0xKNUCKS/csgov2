#pragma once
#include <string>

// Runtime memory analysis — dumps live game data to JSON files
// for cross-referencing with static analysis results.
// Output dir: Documents/CSGO_v2_Analysis/dynamic/
namespace analysis {

    // Dump all data (netvars, interfaces, convars, classes, entity info)
    void DumpAll();

    // Individual dumps
    void DumpNetvars();      // All netvar tables + offsets from ClientClass chain
    void DumpInterfaces();   // Interface pointers + vtable info
    void DumpEntityInfo();   // Local player entity structure, weapon, animstate
    void DumpConVars();      // All ConVars with current values

    // Get output directory path
    std::string GetOutputDir();
}
