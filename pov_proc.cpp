#include "tinyxml2.h"

#include <list>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>
#include <sstream>
#include <iterator>
#include <set>

#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <assert.h>
#include <stdint.h>

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

enum ValueType
{
  VAR,
  DATA,
  vPCRE
};

enum AssignType
{
  aPCRE,
  SLICE
};

typedef struct
{
  StepType type;
  void* the_step;
}MetaStep;

typedef struct
{
  float stime;
}StepDelay;

void dump_delay(StepDelay* sdy)
{
  printf("(SLEEP, { %f })\n", sdy->stime);
}

typedef struct 
{
  char* key;
  char* value;
}StepDecl;

void dump_decl(StepDecl* sdl)
{
  printf("(DECL, {%s : %s})\n", sdl->key, sdl->value);
}

typedef struct
{
  int group;
  char* pattern;
}Regex;

typedef struct
{
  bool invert; 
  vector<ValueType>* types;
  vector<void*>* values;
}Match;

typedef struct
{
  AssignType type;
  char* var;
  void* expr;
}Assign;

typedef struct
{
  int begin;
  int end;
}Slice;

string string_to_hex(const string& in) 
{
    stringstream ss;

    ss << hex ;//<< setw(2) << setfill('0');
    for (size_t i = 0; in.length() > i; ++i) {
        ss << "\\x" << setw(2) << setfill('0') << static_cast<unsigned int>(static_cast<unsigned char>(in[i]));
    }

    return ss.str(); 
}

string hex_to_string(const string& in) 
{
    string output;

    assert((in.length() % 2) == 0);

    size_t cnt = in.length() / 2;

    for (size_t i = 0; cnt > i; ++i) {
        uint32_t s = 0;
        stringstream ss;
        //cout << hex << in.substr(i * 2, 2) << " | " << endl;
        ss << hex << in.substr(i * 2, 2);
        ss >> s;

        output.push_back(static_cast<unsigned char>(s));
    }

    return output;
}

void dump_match(Match* ma)
{
  cout << "{\'invert\': " << ma->invert << ", ";
  cout << "\'values\': [";
  int cnt = ma->types->size();
  ValueType *pt = ma->types->data();
  void** pv = ma->values->data();

  const string types[2] = {"var", "data"};
  int i, t;
  for(i = 0; i < cnt; i++)
  {
    t = (int)pt[i];
    if(t < 3)
    {
      cout << "(\'" << types[t] << "\', " << string_to_hex(*(string*)(pv[i])) << ")";
    }
    else
    {
      Regex* re = (Regex*) pv[i];
      cout <<  "(\'pcre\', " << "{\'group\': " << re->group << ", " << "\'Pattern\': " << "\"" << re->pattern <<"\"}";
    }

    if(i < cnt - 1)
    {
      cout << ", ";
    }
    else
    {
      cout << "]";
    }

  }

}


void dump_assign(Assign* asn)
{
  cout << "{\'var\': " << asn->var << ", ";
  if(asn->type == aPCRE)
  {
    Regex* re = (Regex*)(asn->expr);
    cout << "\'expr(regex)\': " << "{\'group\': " << re->group << ", " << "\'Pattern\': " << "\"" << re->pattern <<"\"}";
  }
  else
  {
    assert(asn->type == SLICE);
    Slice *s = (Slice*)(asn->expr);
    cout << "\'expr(slice)\': " << "{\'bgein\': " << s->begin << ", " << "\'end\': " << s->end << "}";
  }
  cout << "}";
}

typedef struct
{ 
  int timeout;
  char* echo;
  int length;
  string* delim;
  Match* match;
  Assign* assign;
}StepRead;



typedef struct
{
  vector<ValueType> *type;
  vector<string*> *value;
  char* echo;
}StepWrite;



void dump_read(StepRead* sr)
{
  //printf("(READ, {'timeout': %d, 'echo': %s, 'length': %d, 'delim': %s, 'match': ")
  cout << "(READ, ";
  cout << "{\'timeout\': " << sr->timeout << ", ";
  cout << "\'echo\': " << sr->echo << ", ";
  if(sr->length != -1)
  {
    cout << "\'length\': " << sr->length;
  }
  else
  {
    assert(sr->delim);
    cout << "\'delim\': " << string_to_hex(*(sr->delim));
  }

  if(sr->match)
  {
    cout << ", \'match\': ";// << " "; 
    dump_match(sr->match);

  }

  if(sr->assign)
  {
    cout << ", \'assign': ";// << " "; 
    dump_assign(sr->assign);
  }

  cout << "})" << endl;


}

void dump_write(StepWrite* sw)
{
  cout << "(WRITE, ";
  cout << "{\'echo\': " <<  sw->echo << ", ";
  cout << "\'values\': [";
  int i;
  int cnt  = sw->type->size();
  const string types[2] = {"var", "data"};

  ValueType *pt = sw->type->data();
  string** pv = sw->value->data();
  for(i = 0; i < cnt - 1; i++)
  {
    cout << "(\'" << types[pt[i]] << "\', " << string_to_hex(*(pv[i])) << "), ";
  }
  cout << "(\'" << types[pt[i]] << "\', " << string_to_hex(*(pv[i])) << ")]})" << endl;
  //int j = (int)(sw->type[0]);
}

class POV
{

public:

   const char* m_filename;
   const char* m_name;
   vector< MetaStep > m_steps;
   vector<char*> m_variables;
   const char* m_xml;
   //XMLDocument m_doc;


public:
  
  

  
  string* compile_hex_match(const char* text)
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
    string* ret = new string();
    *ret = hex_to_string(data);
    return ret;
  }

  //TODO
  Regex* compile_pcre(const char* text)
  {
    // regex_t* preg;
    // //regmatch_t pmatch[1];
    // bzero(preg, sizeof(regex_t));
    // regcomp(preg, text, REG_EXTENDED);
    // //TODO
    // return preg;
    //return strdup("pcre");
    Regex* re = new Regex();
    re->group = 0;
    re->pattern = strdup(text);
    return re;
  }

  Slice* compile_slice(const XMLElement* elm)
  {
    assert(strcmp(elm->Name(), "slice") ==0);
    assert(!elm->GetText());
    int begin = atoi(get_attribute(elm, "begin", "0")); 
    char* end_c; 
    get_attribute(elm, "end");
    int end;
    if(end_c)
    {
      end = atoi(end_c);
    }
    else
    {
      end = (int)NULL;
    }
    Slice* slice = new Slice();
    slice->begin = begin;
    slice->end = end;
    
    return slice;
  }

  string* compile_string_match(const char* text)
  {
    //return strdup("hello@compile_string_match()");
    string data = string(text);
    int state = 0;
    const char* const hex_chars = "0123456789abcdef";
    string* out = new string();

    for(string::iterator it = data.begin(); it!=data.end(); it++)
    {
      if(state == 0)
      {
        if(*it != '\\')
        {
          out->push_back(*it);
          continue;
        }
        state = 1;
      }
      else if(state == 1)
      {
        switch (*it)
        {
          case 'n':
          {
            out->push_back('\n');
            state = 0;
            break;
          }
          case 'r':
          {
            out->push_back('\r');
            state = 0;
            break;
          }
          case 't':
          {
            out->push_back('\t');
            state = 0;
            break;
          }
          case '\\':
          {
            out->push_back('\\');
            state = 0;
            break;
          }
          case 'x':
          {
            state  = 2;
            break;
          }
          default:
          {
            perror("error char in state 1 @compile_string_match()\n");
          }
        }
      }
      else if(state == 2)
      {
        char c1 = tolower(*it);
        //string::iterator p = lower_bound(hex_chars.begin(), hex_chars.end(), c1);
        const char* const p = lower_bound(hex_chars, hex_chars + 16, c1);
        assert(*p == c1);
        it++;
        char c2 = tolower(*it);
        const char* const q = lower_bound(hex_chars, hex_chars + 16, c2);
        assert(*q == c2);
        out->push_back((p - hex_chars) << 4 | (q - hex_chars));
        state = 0;
      }
      else
      {
        perror("error state @compile_string_match()\n");
      }
    }
    return out;

    //return data;
  }

  void* compile_string(const char* type, const char* data)
  {
    //printf("(type, data)@compile_string(): %s, %s\n", type, data);
    if(strcmp(type, "pcre") == 0)
      return compile_pcre(data);
    else if(strcmp(type, "asciic") == 0)
      return compile_string_match(data);
    else if(strcmp(type, "hex") ==0)
      return compile_hex_match(data);
    else
      perror("wrong type @compile_string()\n");
  }

  char* get_attribute(const XMLElement* elm, const char* name, const char* default_ret = NULL, vector<const char*>* allowed_ret = NULL)
  {
    const char* ret = elm->Attribute(name);
    if(!ret)
      ret = default_ret;
    if(!ret)
      return NULL;
    
    if(allowed_ret)
    {
      vector<const char*>::const_iterator it;
      for(it = allowed_ret->begin(); it != allowed_ret->end(); it++)
      {
        if(strcmp(*it, ret) == 0)
          return strdup(ret);
      }
      perror("ret not allowed @get_attribute()\n");
    }
    else
    {
      return strdup(ret);  
    }
  }


  void add_variable(char* name)
  {
    vector<char*>::iterator it;
    for(it = m_variables.begin(); it != m_variables.end(); it++)
    {
      if(strcmp(*it, name) == 0)
        break;
    }
    if(it == m_variables.end())
      m_variables.push_back(name);
  }


  bool has_variable(const char* var)
  {
    //return true;
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
    // if(type == WRITE)
    // {
    //   cout << "  StepWrite.value@add_step(): " << ((StepWrite*)value)->value->at(0) << endl;
    //   cout << "  StepWrite.echo@add_step(): " << ((StepWrite*)value)->echo << endl;
    // }
    MetaStep ms = 
    {
      .type = type,
      .the_step = value
    };
    m_steps.push_back(ms);
    //m_steps.push_back(make_pair(type, value));
  }


 


  void parse_decl(const XMLElement* elm)
  {
    cout << "decl: " << elm->Name() << endl;
    const XMLElement* child = elm->FirstChildElement();
    assert(strcmp(child->Name(), "var") == 0);
    char* key = strdup(child->GetText());

    child = child->NextSiblingElement();
    assert(strcmp(child->Name(), "value") == 0);
    char* value = strdup(child->GetText());

    StepDecl* sdl = new StepDecl();
    sdl->key = key;
    sdl->value = value;

    add_variable(key);
    add_step(DECLARE, sdl);
  }



  void parse_delay(const XMLElement* elm)
  {
    cout << "delay: " << elm->Name() << endl;
    StepDelay* sdy = new StepDelay();
    float t = 0;
    elm->QueryFloatText(&t);
    sdy->stime = t / 1000;
    add_step(SLEEP, sdy);
  }

  Assign* parse_assign(const XMLElement* elm)
  {
    //TODO
    Assign* asn = new Assign();
    assert(strcmp(elm->Name(), "assign") == 0);
    const XMLElement* child  = elm->FirstChildElement();
    assert(strcmp(child->Name(), "var") == 0);

    char* var = strdup(child->GetText());
    add_variable(var);
    asn->var = var;


    child = child->NextSiblingElement();
    if(strcmp(child->Name(), "pcre") == 0)
    {
      Regex* re = compile_pcre(child->GetText());
      re->group = atoi(get_attribute(child, "group", "0"));
      asn->expr = re;
      asn->type = aPCRE;
    }
    else if(strcmp(child->Name(), "slice") == 0)
    {
      Slice* slice = compile_slice(child);
      asn->type = SLICE;
      asn->expr = slice;
    }
    else
    {
      printf("unknown expr tag %s ", child->Name());
      perror("@parse_assign()\n");
    }
    return asn;
  }

  string* parse_data(const XMLElement* elm, vector<const char*>* allowed_formats = NULL, const char* default_format = NULL)
  {
    int mark = 0;
    if(!allowed_formats)
    {
      allowed_formats = new vector<const char*>();
      allowed_formats->push_back("asciic");
      allowed_formats->push_back("hex");
      mark = 1;
    }

    if(!default_format)
        default_format = "asciic";

    char* data_format = get_attribute(elm, "format", default_format, allowed_formats);
    //cout << "elm->text@parse_data: " << elm->GetText() << endl;
    if(mark == 1)
    {
      delete(allowed_formats);
    }
    return (string*)compile_string(data_format, elm->GetText());
  }


  void parse_read(const XMLElement* elm)
  {
    //cout << endl << "read: " << elm->Name() << endl;
    StepRead* sr = new StepRead();
    add_step(READ, sr);
    vector<const char*>* allowed = new vector<const char*>();
    allowed->push_back("yes");
    allowed->push_back("no");
    allowed->push_back("ascii");
    sr->echo = get_attribute(elm, "echo", "no", allowed);
    sr->match = NULL;
    sr->assign = NULL;
    sr->length = -1;
    sr->delim = NULL;

    const XMLElement* child = elm->FirstChildElement();
    if(strcmp(child->Name(), "length") == 0)
    {
      int len;
      child->QueryIntText(&len);
      sr->length = len;
    }
    else if(strcmp(child->Name(), "delim") == 0)
    {
      sr->delim = parse_data(child);
    }
    else
    {
      perror("invalid first argument @parse_read()\n");
    }

    child = child->NextSiblingElement();
    if(!child)
      return;


    if(strcmp(child->Name(), "match") == 0)
    {
      allowed->clear();
      allowed->push_back("false");
      allowed->push_back("true");
      bool invert = (strcmp(get_attribute(child, "invert", "false", allowed), "true") == 0);
      
      const XMLElement* gchild = child->FirstChildElement();
      assert(gchild);
      vector<void*>* values = new vector<void*>();
      vector<ValueType>* types = new vector<ValueType>();
      const char* tag = gchild->Name();
      if(strcmp(tag, "data") == 0)
      {
        types->push_back(DATA);
        values->push_back(parse_data(gchild));
      }
      else if(strcmp(tag, "pcre") == 0)
      {
        types->push_back(vPCRE);
        values->push_back(compile_string("pcre", gchild->GetText()));
      }
      else if(strcmp(tag, "var") == 0)
      {
        types->push_back(VAR);
        string* text = new string(gchild->GetText());
        values->push_back(text);
      }
      else
      {
        printf("invalid data.match element name: %s ", tag);
        perror("@parse_read()");
      }

      Match* match = new Match();
      match->invert = invert;
      match->types = types;
      match->values = values;
      sr->match = match;

      child = child->NextSiblingElement();
      if(!child)
      {
        delete(allowed);
        return;
      }
    }
    delete(allowed);

    if(strcmp(child->Name(), "assign") == 0)
    {
      Assign* asn = parse_assign(child);
      sr->assign = asn;
      child = child->NextSiblingElement();
      if(!child)
        return;
    }

    
    assert(strcmp(child->Name(), "timeout") == 0);
    //TODO
    int _t = 0;
    child->QueryIntText(&_t);
    sr->timeout = _t;
  }
 



  void parse_write(const XMLElement* elm)
  {
    //cout << endl << "write: ---" << elm->Name() << endl;
    //cout << -2;
    assert(elm);
    vector<string*>* values = new vector<string*>();
    vector<ValueType>* types = new vector<ValueType>();
    //cout << -1;
    const XMLElement* child = elm->FirstChildElement();
    //cout << 0;
    while(child)
    {
      const char* tag = child->Name();
      if(strcmp(tag, "data") ==0)
      {
        //vector<const char*> formats;
        values->push_back(parse_data(child));
        types->push_back(DATA);
      }
      else
      {
        assert(strcmp(tag, "var") == 0);
        assert(has_variable(child->GetText()));
        string* text  = new string(child->GetText());
        values->push_back(text);
        types->push_back(VAR);
      }

      child = child->NextSiblingElement();
      
    }
    //cout << 1;
    // StepWrite st = 
    // {
    //   .value = values,
    //   .echo = get_attribute(elm, "echo", allowed, "no")
    // };

    StepWrite* sw = (StepWrite*)malloc(sizeof(StepWrite));  
    sw->type = types;
    sw->value = values;

    vector<const char*>* allowed = new vector<const char*>();;
    allowed->push_back("yes");
    allowed->push_back("no");
    allowed->push_back("ascii");
    sw->echo = get_attribute(elm, "echo", "no", allowed);
    delete(allowed);
    // cout << 2;
    //cout << ".value@parse_write(): " << st->value->at(0) << endl;


    add_step(WRITE, sw);
    //cout << 3;
  }

  void parse(const char* filename)
  {

    m_filename = filename;
    //m_xml = raw_data;


    doc.LoadFile(filename);
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

  void dump_step(MetaStep ms)
  {
    StepType type = ms.type;

    switch (type)
    {
      case WRITE:
      {
        dump_write((StepWrite*) ms.the_step);
        break;
      }
      case READ:
      {
        dump_read((StepRead*) ms.the_step);
        break;
      }
      case DECLARE:
      {
        dump_decl((StepDecl*) ms.the_step);
        break;
      }
      case SLEEP:
      {
        dump_delay((StepDelay*) ms.the_step);
        break;
      }
    }
  }


  void dump()
  {
    cout << "dump ------------------------" << endl;
    static const char* StepTypes[] = {"declare", "sleep", "read", "write"};
    int vars_size = m_variables.size();
    int i;
    cout << "variables: [" ;
    for(i = 0; i < vars_size - 1; i++)
    {
      cout << m_variables[i]<< ", "; 
    }
    cout << m_variables[i] << "]" << endl << endl;

    vector< MetaStep > ::iterator sit;
    for(sit = m_steps.begin(); sit != m_steps.end(); sit++)
    {
      dump_step(*sit);
    }

  }




  
};

int main()
{
  //XMLDocument doc;

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
