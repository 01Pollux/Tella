#include "Format.hpp"


void FixEscapeCharacters(std::string& str)
{
	char* buf = str.data();
	size_t size = str.size(), step = 0;

	for (size_t i = 0; i < size; i++)
	{
		if (str[i] == '\\' && i + 1 < size)
		{
			char& c2 = str[++i];
			switch (c2)
			{
			case 'n':
			{
				c2 = '\n';
				step++;
				break;
			}
			case 't':
			{
				c2 = '\t';
				step++;
				break;
			}
			case 'r':
			{
				c2 = '\r';
				step++;
				break;
			}
			case '\\':
			case '"':
			{
				step++;
				break;
			}
			default:
			{
				i--;
			}
			}
		}
		*buf++ = str[i];
	}

	str.resize(size - step);
}
