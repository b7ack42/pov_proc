#include "tinyxml2.h"

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iterator>
#include <set>

#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <assert.h>

using namespace tinyxml2;
using namespace std;

void test(int a, int b = 0)
{
	printf("@test(): %d\n", a + b);
}

int main()
{
	vector<char *> sv;
	sv.push_back((char*)"123");
	sv.push_back((char*)"abc");
	char str1[] = "xyz";

	const char* str2 = "123";
	sv.push_back(str1);
	//sv.push_back(str2);

	vector<char*>::iterator it;
	for(it = sv.begin(); it != sv.end(); it++)
	{
		printf("%s\n", (*it));
	}

	vector<void*> v0;
	int i = 1;
	v0.push_back(&i);
	v0.push_back(str1);

	printf("%d\n", *((int*)v0.at(0)));
	printf("%s\n", (char*)v0.at(1));

	test(1, 2);
	test(4);
	if(!NULL)
		printf("null\n");
	//assert(NULL);

	string s1 = string(str2);
	string s2 = "1234";

	assert(s1 == s2.substr(0,3));	


}