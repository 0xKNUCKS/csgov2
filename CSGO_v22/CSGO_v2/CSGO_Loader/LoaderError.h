#pragma once
#include <string>
#include <format>
#include <iostream>
#include <Windows.h>

// Error class for the loader — carries Win32 error details and descriptive messages.
class LoaderError
{
public:
	enum class Code {
		OK = 0,
		FileNotFound,
		ProcessNotFound,
		SnapshotFailed,
		OpenProcessFailed,
		AllocFailed,
		WriteFailed,
		ThreadCreationFailed,
		ThreadExecutionFailed,
		ThreadExitCodeFailed,
		TerminateFailed,
	};

	LoaderError() : m_code(Code::OK), m_win32(0) {}

	LoaderError(Code code, const std::string& message, DWORD win32Error = 0)
		: m_code(code), m_message(message), m_win32(win32Error) {}

	// Convenience: create from current GetLastError()
	static LoaderError FromLastError(Code code, const std::string& message) {
		return LoaderError(code, message, GetLastError());
	}

	bool ok() const { return m_code == Code::OK; }
	explicit operator bool() const { return ok(); }

	Code code() const { return m_code; }
	const std::string& message() const { return m_message; }
	DWORD win32Error() const { return m_win32; }

	std::string format() const
	{
		if (ok()) return "OK";
		std::string result = std::format("[Error {}] {}", (int)m_code, m_message);
		if (m_win32 != 0)
			result += std::format(" (Win32: {})", m_win32);
		return result;
	}

	// Print the error to console (use with dye::red() in caller if desired)
	void print() const
	{
		if (!ok())
			std::cerr << format() << "\n";
	}

private:
	Code m_code;
	std::string m_message;
	DWORD m_win32;
};
