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

//type of interaction

XMLDocument doc;

enum iType
  {
    DECLARE,
    SLEEP,
    READ,
    WRITE    
  };

class POV
{

public:

   string m_filename;
   string m_name;
   vector< pair<iType, char*> > m_steps;
   vector<string> m_variables;
   string m_xml;
   //XMLDocument m_doc;


public:
  //POV(): filename(NULL), name(NULL) {};


  void compile_hex_match(string &data)
  {
    //{'\n', ' ', '\r', '\t'};
    for(string::iterator it = data.begin(); it!=data.end(); it++)
    {
    	bool is_in = ((*it == '\n')||(*it == ' ')||(*it == '\r')||(*it == '\t'));
    	printf("hello\n");
    	if(is_in)
      {
    	    data.erase(it);
          it--;
          //printf("%c\n", (*it));
      }
    }
  }

  regex_t* compile_pcre(string data)
  {
    regex_t* preg;
    //regmatch_t pmatch[1];
    bzero(preg, sizeof(regex_t));
    regcomp(preg, data.c_str(), REG_EXTENDED);
    return preg;

  }

  void parse_decl(const XMLElement* ele)
  {
    cout << "decl: " << ele->Name() << endl;
  }

  void parse_write(const XMLElement* ele)
  {
    cout << "write: " << ele->Name() << endl;
  }

  void parse_read(const XMLElement* ele)
  {
    cout << "read: " << ele->Name() << endl;
  }

  void parse_delay(const XMLElement* ele)
  {
    cout << "delay: " << ele->Name() << endl;
  }


  void parse(string filename)
  {

    //m_filename = filename;
    //m_xml = raw_data;


    doc.LoadFile(filename.c_str());
    XMLPrinter printer;
    doc.Print( &printer);
    m_xml = printer.CStr();

    XMLElement* root = doc.FirstChildElement("pov");
    assert(root);
    XMLElement* cbid = root->FirstChildElement("cbid");
    assert(cbid);
    m_name = cbid->GetText();
    //assert(m_name!=NULL);
    XMLElement* replay = cbid->NextSiblingElement("replay");
    assert(replay);

    // cout <<"replay: " << replay->Name() << endl;

    // cout << "m_xml: " << endl << m_xml << endl;
    // cout << "m_name: " << m_name << endl;
    XMLElement* iter = replay->FirstChildElement();
    while (iter)
    {
      char* tag = strdup(iter->Name());

      if(strcmp(tag, "decl") == 0)
      {
        parse_decl(iter);
      }
      else if(strcmp(tag, "read") == 0)
      {
        parse_read(iter);
      }
      else if(strcmp(tag, "write") == 0)
      {
        parse_write(iter);
      }
      else if(strcmp(tag, "delay") == 0)
      {
        parse_delay(iter);
      }
      else
      {
        perror("error of tag@parse() \n");
      }

      iter = iter->NextSiblingElement();

    }


  }




  
};

int main()
{
  XMLDocument doc;

  // printf("hello world\n");
  // string str("abc ef\t\r\n123");
  // printf("str: %s\n", str.c_str());

  POV pov = POV();
  // pov.compile_hex_match(str);
  // printf("str: %s\n", str.c_str());
  pov.parse("/home/b7ack/hack/pov_proc/pov.xml");
  
  return 0;
}
