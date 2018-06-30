#include <ctype.h>
#include <string>
#include "Helpers.h"

bool DSLib::Helpers::is_number(const std::string& str)
{
	int num_read = 0;
	int num_radix = 0;
	auto it = str.begin();
	for (; it != str.end(); ++it)
		if (!isspace(*it))
			break;
	for (; it != str.end(); ++it)
	{
		if (isspace(*it))
			break;
		if (!isdigit(*it) && *it != '.')
			return false;
		if (*it == '.')
		{
			++num_radix;
			if (num_radix > 1)
				return false;
		}
		++num_read;
	}
	for (; it != str.end(); ++it)
		if (!isspace(*it))
			return false;
	return num_read > 0;
}

bool DSLib::Helpers::is_integer(const std::string& str)
{
	int num_read = 0;
	auto it = str.begin();
	for (; it != str.end(); ++it)
		if (!isspace(*it))
			break;
	for (; it != str.end(); ++it)
	{
		if (isspace(*it))
			break;
		if (!isdigit(*it))
			return false;
		++num_read;
	}
	for (; it != str.end(); ++it)
		if (!isspace(*it))
			return false;
	return num_read > 0;
}