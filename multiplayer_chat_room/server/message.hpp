#pragma  once 
#include<iostream>
#include<string>
#include"./json/json.h"


#include "./json/autolink.h"
#include "./json/value.h"
#include "./json/reader.h"
#include "./json/writer.h"
#include "./json/features.h"
class message
{
  public:
  //反序列化
  //将客户端发送来的json数据反序列化
  void deseruakuze(std::string mes)
  {
    Json::Reader r;
    Json::Value v;
    r.parse(mes,v,false);

    name_ = v["name_"].asString();
    from_ = v["from_"].asString();
    msg_ = v["msg_"].asString();
    user_id = v["user_id"].asInt();
  }

  void serialize(std::string* msg)
  {
    Json::Value v;
    v["name_"] = name_;
    v["from_"] = from_;
    v["msg_"] = msg_;
    v["user_id"] = user_id;

    Json::FastWriter w;
    *msg = w.write(v);
  }
  uint32_t& get_user_id()
  {
    return user_id;
  }

  std::string& get_name()
  {
    return name_;
  }

  std::string& get_from()
  {
    return from_;
  }

  std::string& get_msg()
  {
    return msg_;
  }
  void set_name(std::string& name)
  {
    name_ = name;
  }
  
  void set_from(std::string& from)
  {
    from_ = from;
  }
  
  void set_msg(std::string& msg)
  {
    msg_ = msg;
  }
  
  void set_user_id(uint32_t ui)
  {
    user_id = ui;
  }
  
  private:
    std::string name_;
    std::string from_;
    std::string msg_;
    uint32_t user_id;

};
