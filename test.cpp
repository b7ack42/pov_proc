#include "tinyxml2.h"

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iterator>
#include <set>
#include <iomanip>
#include <sstream>

#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <assert.h>
#include <stdint.h>

using namespace tinyxml2;
using namespace std;

string hex_to_string(const string& in) 
{
    std::string output;

    assert((in.length() % 2) == 0);

    size_t cnt = in.length() / 2;

    for (size_t i = 0; cnt > i; ++i) {
        uint32_t s = 0;
        stringstream ss;
        cout << hex << in.substr(i * 2, 2) << " | " << endl;
        ss << hex << in.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

    return output;
}

string hex_to_string2(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();
    // if (len & 1) throw std::invalid_argument("odd length");

    string output;
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        // if (*p != a) throw std::invalid_argument("not a hex digit");

        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        // if (*q != b) throw std::invalid_argument("not a hex digit");

        output.push_back(((p - lut) << 4) | (q - lut));
    }
    return output;
}

void test(int a, int b = 0)
{
	printf("@test(): %d\n", a + b);
}

void test1()
{
		// vector<char *> sv;
	// sv.push_back((char*)"123");
	// sv.push_back((char*)"abc");
	// char str1[] = "xyz";

	// const char* str2 = "123";
	// str2 = "456";
	// cout << "str2: " << str2 << endl;
	// sv.push_back(str1);
	// //sv.push_back(str2);

	// vector<char*>::iterator it;
	// for(it = sv.begin(); it != sv.end(); it++)
	// {
	// 	printf("%s\n", (*it));
	// }

	// vector<void*> v0;
	// int i = 1;
	// v0.push_back(&i);
	// v0.push_back(str1);

	// printf("%d\n", *((int*)v0.at(0)));
	// printf("%s\n", (char*)v0.at(1));

	// test(1, 2);
	// test(4);
	// if(!NULL)
	// 	printf("null\n");
	// //assert(NULL);

	// string s1 = string(str2);
	// string s2 = "1234";

	// assert(s1 == s2.substr(0,3));
}

void test2()
{
	string str_h = "48656C6C6F20576F726C64";
	string str_a = hex_to_string(str_h);
	cout << str_a << endl;
	cout << dec << "len: " << str_a.length() << endl;
}

void test3()
{
	char* buf = strdup("\\x48\\x65\\x6c\\x6c");
	string str = string(buf);
	assert(str[0] == '\\');
	cout << str << " | " << str.length() << endl;
}

void test4()
{
	string i = "123456";
	string* j = new string();
	*j = i;
	//i = "456";
	cout << "i, j  " << i << ", " << *j << endl;
}

int test5(int v)
{
	int i = v;
	return i;
}

int* test6(int v)
{
	int* i = new int();
	*i = test5(v);
	return i;
}

void test7(char c)
{
	switch(c)
	{
		case 'a':
			cout << 1;
			break;
		case 'b':
			cout << 2;
			break;
		default:
			cout << 3;
			break;
	}
}

void test8()
{
	int i = 0;
	if(i == 0)
		i = 1;
	else if(i == 1)
		i = 2;
	else if(i ==2 )
		i = 3;

	cout << (i++) << endl;
}

void test9()
{
	string str = string("123\x00\x6c\x65", 6);
	cout << str.length() << endl << hex << str;
}

int main()
{

	//test2();
	//test3();
	// int* i = test6(3);
	// test6(4);
	// cout << *i << endl;
	test2();
	test9();
	// else
	// 	printf("%d", i);

}