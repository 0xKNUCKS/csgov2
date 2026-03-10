#pragma once
#ifdef _DEBUG

#include <string>
#include <vector>
#include <mutex>
#include <ctime>
#include <fstream>
#include "CrashLog.h"

// ============================================================
// AuditLog — Debug-only in-memory log with deduplication.
//            Repeated consecutive messages get a counter instead
//            of spamming the list. Also writes to file.
// ============================================================

struct AuditEntry
{
	std::string timestamp;
	std::string severity;   // INFO, WARN, ERR, FATAL
	std::string message;
	int count = 1;
};

namespace AuditLog
{
	inline std::vector<AuditEntry> g_entries;
	inline std::mutex g_mutex;
	inline constexpr int MAX_ENTRIES = 500;
	inline constexpr const char* AUDIT_FILE = "C:\\Users\\adama\\Documents\\CSGO_v2_Logs\\audit.log";

	inline std::string Now()
	{
		time_t now = time(nullptr);
		struct tm ts;
		localtime_s(&ts, &now);
		char buf[32];
		strftime(buf, sizeof(buf), "%H:%M:%S", &ts);
		return std::string(buf);
	}

	inline void WriteEntryToFile(const AuditEntry& e)
	{
		CrashLog::EnsureDir();
		std::ofstream ofs(AUDIT_FILE, std::ios::app);
		if (!ofs.is_open()) return;
		if (e.count > 1)
			ofs << "[" << e.timestamp << "] [" << e.severity << "] " << e.message << " (x" << e.count << ")\n";
		else
			ofs << "[" << e.timestamp << "] [" << e.severity << "] " << e.message << "\n";
	}

	inline void Add(const char* severity, const char* msg)
	{
		std::lock_guard lock(g_mutex);

		// Deduplicate: if last entry has the same message and severity, increment counter
		if (!g_entries.empty()) {
			auto& last = g_entries.back();
			if (last.message == msg && last.severity == severity) {
				last.count++;
				// Update file periodically (every 10th duplicate to avoid excessive IO)
				if (last.count % 10 == 0)
					WriteEntryToFile(last);
				return;
			}
			// Flush the previous deduplicated entry to file if it had repeats
			if (last.count > 1)
				WriteEntryToFile(last);
		}

		AuditEntry e;
		e.timestamp = Now();
		e.severity = severity;
		e.message = msg;
		g_entries.push_back(e);

		// Write single entries immediately
		WriteEntryToFile(e);

		// Cap size — remove oldest
		if ((int)g_entries.size() > MAX_ENTRIES)
			g_entries.erase(g_entries.begin());
	}

	inline void Info(const char* msg)    { Add("INFO", msg); }
	inline void Warn(const char* msg)    { Add("WARN", msg); }
	inline void Error(const char* msg)   { Add("ERR",  msg); }
	inline void Fatal(const char* msg)   { Add("FATAL", msg); }

	// Format all entries into a single string for clipboard copy
	inline std::string FormatAll()
	{
		std::lock_guard lock(g_mutex);
		std::string result;
		result.reserve(g_entries.size() * 80);
		for (const auto& e : g_entries) {
			result += "[" + e.timestamp + "] [" + e.severity + "] " + e.message;
			if (e.count > 1)
				result += " (x" + std::to_string(e.count) + ")";
			result += "\n";
		}
		return result;
	}

	inline void Clear()
	{
		std::lock_guard lock(g_mutex);
		g_entries.clear();
	}

	inline int Size()
	{
		std::lock_guard lock(g_mutex);
		return (int)g_entries.size();
	}
}

#endif // _DEBUG
