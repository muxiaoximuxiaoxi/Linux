#pragma  once 
#include<iostream>
#include<arpa/inet.h>
#include<string>
#include<cstring>
#include<unordered_map>
#include<vector>
#include<pthread.h>


#include"log_svr.hpp"

#define OFF_LINE 0
#define REGISTERED 1
#define LOGINED 2
#define ON_LINE 3
class user_info
{
  public:
    //在注册和登录阶段使用的是tcp，所有不可以保存tcp地址信息，而是等到当前登录
    //上来的用户第一次使用udp协议发送消息时，将udp的地址信息保存下来
    //保存下来之后，进行群发时，就可以找到有效的udp地址信息
    user_info(const std::string& name,const std::string& from,const uint32_t userid,const std::string& passwd)
    {
      name_ = name;
      from_ = from;
      user_id = userid;
      passwd_ = passwd;
      memset(&cli_addr,'0',sizeof(struct sockaddr_in));
      cli_addr_len = -1;
      user_status = OFF_LINE;
    }

    void set_user_status(int Status)
    {
      user_status = Status;
    }

    std::string& get_passwd()
    {
      return passwd_;
    }
    int& get_user_status()
    {
      return user_status;
    }
    void set_cli_addr_info(const struct sockaddr_in& cli_addr_info)
    {
      memcpy(&cli_addr,&cli_addr_info,sizeof(cli_addr_info));
    }
    void set_cli_addr_len(const socklen_t& clen)
    {
      cli_addr_len = clen;
    }
    struct sockaddr_in& get_cli_addr()
    {
      return cli_addr;
    }

    socklen_t& get_cli_addr_len()
    {
      return cli_addr_len;
    }
  private:
    std::string name_;
    std::string from_;
    //用户id
    uint32_t user_id;
    std::string passwd_;
    
    //保存udp客户端地址信息
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len;

    //保存当前用户状态
    int user_status;
};


class user_manager
{
  public:
    user_manager()
    {
      user_map.clear();
      online_user_vectot.clear();
      pthread_mutex_init(&lock_,NULL);
      p_user_id = 0;
    }

    ~user_manager()
    {
      pthread_mutex_destroy(&lock_);
    }

    //当注册新用户时，需要输入：name from passwd  返回 uesrid
    int Register(const std::string& name,const std::string& from,const std::string& passwd,uint32_t* userid)
    {
      if(name.size() == 0 || from.size() == 0 || passwd.size() == 0 )
      {
        return -1;
      }
      pthread_mutex_lock(&lock_);
      user_info User_Info(name,from,p_user_id,passwd);
      //需要更改当前用户的状态，改为已注册状态
      User_Info.set_user_status(REGISTERED);
      //插入到map当中去
      user_map.insert(std::make_pair(p_user_id,User_Info));
      *userid = p_user_id;
      ++p_user_id;
      pthread_mutex_unlock(&lock_);
     return 1;
    }
    
    int Login(const uint32_t& ui,const std::string& p)//传入用户id，密码
    {
      if(p.size() < 0)
      {
        return -1;
      }

      //pthread_mutex_lock(&lock_);
      //用于标记当前的登录状态
      int flag_status = -1;

      //防止迭代器失效，所以需要加锁
      pthread_mutex_lock(&lock_);
      //返回登录的状态
      //1.先在map中查找是否存在该id
      //  ①不存在
      //  ②存在
      //    a.密码正确
      //    b.密码不正确
      //std::unordered_map<uint32_t, user_info>::iterator it;
      auto it = user_map.find(ui);
      if(it != user_map.end())
      {
        //查找到
        if(p == it->second.get_passwd()) 
        {
          //密码正确
          //
          it->second.get_user_status() = LOGINED; 

          flag_status = 1;
        }
        else 
        {
          //密码不正确
          LOG(ERROR,"user passwd is not correct passwd is ")<<it->second.get_passwd()<<std::endl;
          flag_status = -1;
        }
      }
      else 
      {
        //未找到
        LOG(ERROR,"user_id is not found  ")<<std::endl;
        flag_status = -1;
      }
      pthread_mutex_unlock(&lock_);

      return flag_status;
    }
    int LoginOut()
    {
    }

    bool is_login(uint32_t ui,const struct sockaddr_in& cli_addr,const socklen_t& cli_addr_len)
    {
      if(sizeof(cli_addr) < 0 || cli_addr_len)
      {
        return false;
      }
      pthread_mutex_lock(&lock_);
      //1.校验当前用户是否存在
      auto it = user_map.find(ui);
      if(it == user_map.end())
     {
        
        pthread_mutex_unlock(&lock_);
        LOG(ERROR,"user not exist");
        return false;
      }
      
      //2.判断当前用户状态，来判断是否完成注册和登录
      if(it->second.get_user_status() == OFF_LINE || it->second.get_user_status() == REGISTERED)
      {
        pthread_mutex_unlock(&lock_);
        LOG(ERROR,"user status error");
        return false;
      }

      //3.判断用户是否是第一次发送消息
      if(it->second.get_user_status() == ON_LINE)
      {
        pthread_mutex_unlock(&lock_);
        return true;
      }
      
      //第一次发送消息
      if(it->second.get_user_status() == LOGINED)
      {
        //增加地址信息、地址信息长度、改变状态为ON_LINE
        it->second.set_cli_addr_info(cli_addr);
        it->second.set_cli_addr_len(cli_addr_len);
        it->second.set_user_status(ON_LINE);
       
        online_user_vectot.push_back(it->second);

      }
        pthread_mutex_unlock(&lock_);
      return true;
    }
    void get_user_vectot_info(std::vector<user_info>* v)
    {
      *v = online_user_vectot;
    }

  private:
    //map 红黑树 log(N)
    //unordered_map 哈希表 可以是常数
    
    //保存所有注册用户的信息---tcp
    std::unordered_map<uint32_t,user_info> user_map; 
    
    pthread_mutex_t lock_;
    //保存在线用户的信息--udp，判断在线的标准，是否使用udp协议发送消息
    std::vector<user_info> online_user_vectot;
    
    //预分配的用户id
    uint32_t p_user_id;

};

