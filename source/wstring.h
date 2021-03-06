/****************************************************************************
 * wstring Class
 * by Hibernatus
 ***************************************************************************/
#ifndef __WSTRING_HPP
#define __WSTRING_HPP

#include <string>

class wString: public std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >
{
	public:
		wString(void)
		{
		}
		wString(const wchar_t *s);
		wString(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > &ws);
		wString(const std::string &s);
		wString &operator=(const std::string &s);
		void fromUTF8(const char *s);
		std::string toUTF8(void) const;

		static wString FromUTF8( const std::string &utf8 );
};

size_t utf8Len(const char *s);
int wcsnicmp(const wchar_t *s1, const wchar_t *s2, int len);

#endif // !defined(__WSTRING_HPP)
