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

enum StepType
  {
    DECLARE,
    SLEEP,
    READ,
    WRITE    
  };

typedef struct
{
  vector<char*> *value;
  char* echo;

}step_write;

class POV
{

public:

   string m_filename;
   string m_name;
   vector< pair<StepType, void*> > m_steps;
   vector<char*> m_variables;
   string m_xml;
   //XMLDocument m_doc;


public:
  //POV(): filename(NULL), name(NULL) {};


  char* compile_hex_match(char* text)
  {
    //{'\n', ' ', '\r', '\t'};
    string data = string(text);
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
    return strdup(data.c_str());
  }

  //TODO
  char* compile_pcre(char* text)
  {
    regex_t* preg;
    //regmatch_t pmatch[1];
    bzero(preg, sizeof(regex_t));
    regcomp(preg, text, REG_EXTENDED);
    return strdup("pcre");

  }

  char* compile_string_match(char* data)
  {
    //return strdup("hello@compile_string_match()");
    return data;
  }

  char* compile_string(char* type, char* data)
  {
    printf("(type, data)@compile_string(): %s, %s\n", type, data);
    if(strcmp(type, "pcre") == 0)
      return compile_pcre(data);
    else if(strcmp(type, "asciic") == 0)
      return compile_string_match(data);
    else if(strcmp(type, "hex") ==0)
      return compile_hex_match(data);
    else
      perror("wrong type @compile_string()\n");
  }

  char* get_attribute(const XMLElement* elm, const char* name, vector<const char*> allowed_ret, const char* default_ret = NULL)
  {
    const char* ret = elm->Attribute(name);
    if(!ret)
      ret = default_ret;
 
    vector<const char*>::const_iterator it;
    for(it = allowed_ret.begin(); it != allowed_ret.end(); it++)
    {
      if(strcmp(*it, ret) == 0)
        return strdup(ret);
    }
    perror("ret not allowed @get_attribute()\n");
    
  }

  bool has_variable(const char* var)
  {
    return true;
    vector<char*>::iterator it;
    for(it = m_variables.begin(); it != m_variables.end(); it++)
    {
      if(strcmp(*it, var) == 0)
        return true;
    }
    return false;
  }

  void add_step(StepType type,  void* value)
  {
    if(type == WRITE)
    {
      cout << "  step_write.value@add_step(): " << ((step_write*)value)->value->at(0) << endl;
      cout << "  step_write.echo@add_step(): " << ((step_write*)value)->echo << endl;
    }
    m_steps.push_back(make_pair(type, value));
  }


 


  void parse_decl(const XMLElement* elm)
  {
    cout << "decl: " << elm->Name() << endl;
  }


  void parse_read(const XMLElement* elm)
  {
    cout << endl << "read: " << elm->Name() << endl;
  }

  void parse_delay(const XMLElement* elm)
  {
    cout << "delay: " << elm->Name() << endl;
  }
 
  char* parse_data(const XMLElement* elm, vector<const char*> allowed_formats, const char* default_format = NULL)
  {
    if(allowed_formats.empty())
    {
      allowed_formats.push_back("asciic");
      allowed_formats.push_back("hex");
    }

    if(!default_format)
        default_format = "asciic";

    char* data_format = get_attribute(elm, "format", allowed_formats, default_format);
    cout << "elm->text@parse_data: " << elm->GetText() << endl;
    return compile_string(data_format, strdup(elm->GetText()));
  }


  void parse_write(const XMLElement* elm)
  {
    cout << endl << "write: ---" << elm->Name() << endl;
    //cout << -2;
    assert(elm);
    vector<char*>* values = new vector<char*>();
    //cout << -1;
    const XMLElement* child = elm->FirstChildElement();
    //cout << 0;
    while(child)
    {
      const char* tag = child->Name();
      if(strcmp(tag, "data") ==0)
      {
        vector<const char*> formats;
        values->push_back(parse_data(child, formats));
      }
      else
      {
        assert(strcmp(tag, "var") == 0);
        assert(has_variable(child->GetText()));
        values->push_back(strdup(child->GetText()));
      }

      child = child->NextSiblingElement();
      
    }
    //cout << 1;
    vector<const char*> allowed;
    allowed.push_back("yes");
    allowed.push_back("no");
    allowed.push_back("ascii");

    // step_write st = 
    // {
    //   .value = &values,
    //   .echo = get_attribute(elm, "echo", allowed, "no")
    // };

    step_write* st = (step_write*)malloc(sizeof(step_write));  
    
    st->value = values,
    st->echo = get_attribute(elm, "echo", allowed, "no");
    // cout << 2;
    cout << ".value@parse_write(): " << st->value->at(0) << endl;


    add_step(WRITE, st);
    //cout << 3;
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
    const XMLElement* child = replay->FirstChildElement();
    while (child)
    {
      char* tag = strdup(child->Name());

      if(strcmp(tag, "decl") == 0)
      {
        parse_decl(child);
      }
      else if(strcmp(tag, "read") == 0)
      {
        parse_read(child);
      }
      else if(strcmp(tag, "write") == 0)
      {
        parse_write(child);
      }
      else if(strcmp(tag, "delay") == 0)
      {
        parse_delay(child);
      }
      else
      {
        perror("error of tag@parse() \n");
      }

      child = child->NextSiblingElement();

    }

  }

  void dump()
  {
    cout << "dump ------------------------" << endl;
    static const char* StepTypes[] = {"declare", "sleep", "read", "write"};
    vector< pair<StepType, void*> > ::iterator it;
    for(it = m_steps.begin(); it != m_steps.end(); it++)
    {
      if(it->first == WRITE)
      {
        cout << "write: "  <<  endl;
        if(((step_write *)(it->second))->value->empty())
          cout << "empty" << endl;
        cout  << ((step_write*)(it->second))->value->at(0) << endl;
      }
      else
        cout << StepTypes[it->first] << endl;

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
  pov.parse("/home/b7ack/hack/pov_proc/pov-1.xml");
  pov.dump();
  
  return 0;
}
