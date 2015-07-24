/*
The MIT License (MIT)
Copyright (c) 2015 Tayfun Kayhan
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Utils.h>

#include <base.h>

using std::string;
using std::vector;

string Utils::split(const string& source, const string& delimeter, bool byTail)
{
	assert(source.size() > 1 && "String source to split cannot be empty!");
	size_t delimeterIndex = -1;
	if (byTail)
	{
		delimeterIndex = source.rfind(delimeter);
	}
	else
	{
		delimeterIndex = source.find(delimeter);
	}

	return (source.substr(delimeterIndex + 1, source.size()));
}

StringVector Utils::split(string& source, const string& delimeter)
{
	StringVector result;
	result.reserve(source.size());
	while (!source.empty())
	{
		const int index = (int) source.find_first_of(delimeter.c_str());
		if (index > 0)
		{
			const string item = source.substr(0, index + 1);
			result.push_back(item);
			source.erase(0, index + 1);
		}
		else
		{
			result.push_back(source);
			source.clear();
		}
	}

	return result;
}