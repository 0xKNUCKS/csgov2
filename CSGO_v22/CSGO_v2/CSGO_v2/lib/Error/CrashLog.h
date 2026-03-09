#pragma once
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <Windows.h>

// ============================================================
// CrashLog — SEH-safe file logger with timestamps.
//            No C++ objects (std::string, etc.) so it works
//            inside __try/__except and vectored handlers.
//            Writes to C:\Users\adama\Documents\CSGO_v2_Logs\
// ============================================================
namespace CrashLog
{
	inline constexpr const char* LOG_DIR  = "C:\\Users\\adama\\Documents\\CSGO_v2_Logs";
	inline constexpr const char* LOG_FILE = "C:\\Users\\adama\\Documents\\CSGO_v2_Logs\\csgo_v2.log";

	// Ensure the log directory exists
	inline void EnsureDir()
	{
		CreateDirectoryA(LOG_DIR, nullptr);
	}

	// Format a timestamp into buf: "[HH:MM:SS.mmm]"
	inline void Timestamp(char* buf, size_t bufSize)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf_s(buf, bufSize, "[%02d:%02d:%02d.%03d]",
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	}

	// Write a timestamped line to the log file
	inline void Write(const char* msg)
	{
		EnsureDir();

		FILE* f = nullptr;
		fopen_s(&f, LOG_FILE, "a");
		if (!f) return;

		char ts[32];
		Timestamp(ts, sizeof(ts));
		fprintf(f, "%s %s\n", ts, msg);
		fclose(f);
	}

	// Write a formatted timestamped line (printf-style)
	inline void Writef(const char* fmt, ...)
	{
		char msg[512];
		va_list args;
		va_start(args, fmt);
		vsprintf_s(msg, fmt, args);
		va_end(args);
		Write(msg);
	}

	// Write a session separator with date/time
	inline void NewSession()
	{
		EnsureDir();

		FILE* f = nullptr;
		fopen_s(&f, LOG_FILE, "a");
		if (!f) return;

		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(f, "\n==================== Session %04d-%02d-%02d %02d:%02d:%02d ====================\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		fclose(f);
	}
}
