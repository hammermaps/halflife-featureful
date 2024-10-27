#include "error_collector.h"
#include <cstdarg>
#include <cstdio>

#ifndef CLIENT_DLL
#include "extdll.h"
#include "enginecallback.h"
#endif

void ErrorCollector::AddError(const char *str)
{
	if (str)
	{
		_errors.push_back(str);
#ifndef CLIENT_DLL
		ALERT(at_error, "%s\n", str);
#endif
	}
}

void ErrorCollector::AddFormattedError(const char *format, ...)
{
	va_list	argptr;
	static char buf[1024];

	va_start(argptr, format);
	_vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);

	AddError(buf);
}

bool ErrorCollector::HasErrors() const
{
	return _errors.size() > 0;
}

std::string ErrorCollector::GetFullString() const
{
	std::string fullString;
	for (const std::string& str : _errors)
	{
		fullString += str;
		fullString += '\n';
	}
	return fullString;
}

void ErrorCollector::Clear()
{
	_errors.clear();
}

ErrorCollector g_errorCollector;
