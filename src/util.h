#ifndef _UTIL_H_
#define _UTIL_H_
#include <string>

using namespace std;

string convBase(unsigned long v, long base)
{
	string		digits = "0123456789abcdef";
	string		result;
	if ((base < 2) || (base > 16)) {
		result = "Error: base out of range.";
	} else {
		do {
			result = digits[v % base] + result;
			v /= base;
		}
		while (v);
	}

	if(base == 2){
	  if(result.size() < 32){
	    for(int i = (32 - result.size()); i > 0; --i){
	      result.insert(0, "0");
	    }
	  }

	  result.insert(8, " ");
	  result.insert(17, " ");
	  result.insert(26, " ");
	
	}

	return result;
}

#endif
