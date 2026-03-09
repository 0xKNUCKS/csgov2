#pragma once
#include <cstdio>
#include <ctime>
#include <format>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>
#include <Windows.h>
#include "CrashLog.h"

// ============================================================
// Error — carries full context about what went wrong
// ============================================================
class Error
{
public:
	enum class Severity { Warning, Error, Fatal };

	Error() = default;

	Error(Severity severity, const std::string& system, const std::string& message, DWORD win32Error = 0)
		: m_severity(severity), m_system(system), m_message(message), m_win32Error(win32Error) {}

	// Quick constructors
	static Error Warn(const std::string& system, const std::string& message) {
		return Error(Severity::Warning, system, message);
	}
	static Error Err(const std::string& system, const std::string& message) {
		return Error(Severity::Error, system, message);
	}
	static Error Fatal(const std::string& system, const std::string& message) {
		return Error(Severity::Fatal, system, message);
	}
	static Error Win32(const std::string& system, const std::string& message) {
		return Error(Severity::Error, system, message, GetLastError());
	}
	static Error Win32Fatal(const std::string& system, const std::string& message) {
		return Error(Severity::Fatal, system, message, GetLastError());
	}

	Severity severity() const { return m_severity; }
	const std::string& system() const { return m_system; }
	const std::string& message() const { return m_message; }
	DWORD win32Error() const { return m_win32Error; }

	bool isFatal() const { return m_severity == Severity::Fatal; }

	std::string format() const
	{
		std::string sev;
		switch (m_severity) {
		case Severity::Warning: sev = "WARN"; break;
		case Severity::Error:   sev = "ERR";  break;
		case Severity::Fatal:   sev = "FATAL"; break;
		}

		std::string result = std::format("[{}] [{}] {}", sev, m_system, m_message);
		if (m_win32Error != 0)
			result += std::format(" (Win32: {})", m_win32Error);
		return result;
	}

private:
	Severity m_severity = Severity::Error;
	std::string m_system;
	std::string m_message;
	DWORD m_win32Error = 0;
};


// ============================================================
// Result — wraps a success/fail outcome with an Error on failure.
// Use: if (!result) { /* handle result.error() */ }
// ============================================================
class Result
{
public:
	Result() : m_ok(true) {} // success
	Result(const Error& err) : m_ok(false), m_error(err) {} // failure

	static Result Ok() { return Result(); }
	static Result Fail(const Error& err) { return Result(err); }

	bool ok() const { return m_ok; }
	explicit operator bool() const { return m_ok; }

	const Error& error() const { return m_error; }

private:
	bool m_ok;
	Error m_error;
};


// ============================================================
// Log — writes to debug console + collects errors for review.
//       In Release, logging still works but goes to file only.
// ============================================================
namespace Log
{
	// All errors encountered during this session
	inline std::vector<Error> g_errors;
	inline std::mutex g_mutex;

	inline std::string Timestamp()
	{
		time_t now = time(nullptr);
		struct tm ts;
		localtime_s(&ts, &now);
		char buf[32];
		strftime(buf, sizeof(buf), "%H:%M:%S", &ts);
		return std::string(buf);
	}

	// Core: record and print an Error object
	inline void Record(const Error& err)
	{
		std::lock_guard lock(g_mutex);
		g_errors.push_back(err);

		std::string line = std::format("[{}] {}", Timestamp(), err.format());

#ifdef _DEBUG
		printf("%s\n", line.c_str());
#endif
	}

	// Print an info message (not an error, no recording)
	template<typename... Args>
	void Info(const char* system, std::format_string<Args...> fmt, Args&&... args) {
#ifdef _DEBUG
		printf("[%s] [INFO] [%s] %s\n", Timestamp().c_str(), system,
			std::format(fmt, std::forward<Args>(args)...).c_str());
#endif
	}

	// Log + record a warning
	template<typename... Args>
	void Warn(const char* system, std::format_string<Args...> fmt, Args&&... args) {
		Record(Error::Warn(system, std::format(fmt, std::forward<Args>(args)...)));
	}

	// Log + record an error
	template<typename... Args>
	void Err(const char* system, std::format_string<Args...> fmt, Args&&... args) {
		Record(Error::Err(system, std::format(fmt, std::forward<Args>(args)...)));
	}

	// Log + record a fatal error
	template<typename... Args>
	void Fatal(const char* system, std::format_string<Args...> fmt, Args&&... args) {
		Record(Error::Fatal(system, std::format(fmt, std::forward<Args>(args)...)));
	}

	// Log + record a Win32 error (captures GetLastError automatically)
	template<typename... Args>
	void WinError(const char* system, std::format_string<Args...> fmt, Args&&... args) {
		Record(Error(Error::Severity::Error, system,
			std::format(fmt, std::forward<Args>(args)...), GetLastError()));
	}

	// Check if any fatal errors have been recorded
	inline bool HasFatalErrors()
	{
		std::lock_guard lock(g_mutex);
		for (const auto& e : g_errors)
			if (e.isFatal()) return true;
		return false;
	}

	// Check if any errors (non-warning) have been recorded
	inline bool HasErrors()
	{
		std::lock_guard lock(g_mutex);
		for (const auto& e : g_errors)
			if (e.severity() != Error::Severity::Warning) return true;
		return false;
	}

	// Get count of errors by severity
	inline int Count(Error::Severity sev)
	{
		std::lock_guard lock(g_mutex);
		int n = 0;
		for (const auto& e : g_errors)
			if (e.severity() == sev) n++;
		return n;
	}

	// Log directory (shared with CrashLog)
	inline std::string GetLogDirectory()
	{
		CrashLog::EnsureDir();
		return std::string(CrashLog::LOG_DIR) + "\\";
	}

	// Dump all errors to a log file
	inline void DumpToFile(const std::string& filename)
	{
		std::string path = GetLogDirectory() + filename;

		std::lock_guard lock(g_mutex);
		std::ofstream ofs(path, std::ios::app);
		if (!ofs.is_open()) return;

		time_t now = time(nullptr);
		struct tm ts;
		localtime_s(&ts, &now);
		char dateBuf[64];
		strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", &ts);

		ofs << "========== Error Dump: " << dateBuf << " ==========\n";
		for (const auto& e : g_errors)
			ofs << e.format() << "\n";
		ofs << "========== End Dump (" << g_errors.size() << " entries) ==========\n\n";
	}

	// Clear all recorded errors
	inline void Clear()
	{
		std::lock_guard lock(g_mutex);
		g_errors.clear();
	}
}
