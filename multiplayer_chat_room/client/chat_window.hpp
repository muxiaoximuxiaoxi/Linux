#pragma  once 
#include<stdio.h>
#include<unistd.h>
#include<ncurses.h>
#include<vector>
#include<pthread.h>
#include<stdlib.h>

#include"chat_client.hpp"
#include"../server/message.hpp"
class chat_window;
class pthread_num
{
  public:
    pthread_num(chat_window* winp,int pn,chat_client* cc)
    {
      win_p = winp;
      p_n = pn;
      c_c = cc;
    }

  public:
      chat_window* win_p;
      int p_n;
      chat_client* c_c;
};
class chat_window
{
  private:
    WINDOW* head_;
    WINDOW* out_put;
    WINDOW* user_list;
    WINDOW* in_put;

    std::vector<pthread_t> v_p_s;
    pthread_mutex_t lock_;
  public:
    chat_window()
    {
      head_ = NULL;
      out_put = NULL;
      in_put = NULL;
      user_list = NULL;
      v_p_s.clear();
      pthread_mutex_init(&lock_,NULL);

      //初始化窗口
      initscr();
      //不显示光标
      curs_set(0);
    }

    ~chat_window()
    {
      if(head_)
      {
        delwin(head_);
      }
      if(out_put)
      {
        delwin(out_put);
      }
      if(user_list)
      {
        delwin(user_list);
      }
      if(in_put)
      {
        delwin(in_put);
      }
      //关闭窗口
      endwin();
      pthread_mutex_destroy(&lock_);
    }
    void Draw_head()
    {
      //行数
      int y = LINES/5;
      //列数
      int x = COLS;
      int start_y = 0;
      int start_x = 0;

      //WINDOW* header_;
      head_ = newwin(y,x,start_y,start_x);
      box(head_,0,0);
      pthread_mutex_lock(&lock_);
      //刷新窗口
      wrefresh(head_);
      pthread_mutex_unlock(&lock_);
    }

    void Draw_out_put()
    {

      //行数
      int y = (LINES*3)/5;
      //列数
      int x = (COLS*3)/4;
      int start_y = LINES/5;
      int start_x = 0;

      out_put = newwin(y,x,start_y,start_x);
      box(out_put,0,0);
      pthread_mutex_lock(&lock_);
      //刷新窗口
      wrefresh(out_put);
      pthread_mutex_unlock(&lock_);
    }

    void Draw_user_list()
    {

      //行数
      int y = (LINES*3)/5;
      //列数
      int x = COLS/4;
      int start_y = LINES/5;
      int start_x = (COLS*3)/4;

      user_list = newwin(y,x,start_y,start_x);
      box(user_list,0,0);
      pthread_mutex_lock(&lock_);
      //刷新窗口
      wrefresh(user_list);
      pthread_mutex_unlock(&lock_);
    }

    void Draw_in_put()
    {

      //行数
      int y = LINES/5;
      //列数
      int x = COLS;
      int start_y = (LINES*4)/5;
      int start_x = 0;

      in_put = newwin(y,x,start_y,start_x);
      box(in_put,0,0);
      pthread_mutex_lock(&lock_);
      //刷新窗口
      wrefresh(in_put);
      pthread_mutex_unlock(&lock_);
    }

    void put_string_to_window(WINDOW* win,int y,int x,std::string& s)
    {
      //mvwaddstr(win,y,x,s.c_str());
      
      pthread_mutex_lock(&lock_);
      mvwaddstr(win,y,x,s.c_str());
      wrefresh(win);
      pthread_mutex_unlock(&lock_);
    }

    void get_string_from_win(WINDOW* win,std::string* s)
    {
      char buf[1024];
      memset(buf,'\0',sizeof(buf));
      wgetnstr(win,buf,sizeof(buf)-1);
      (*s).assign(buf,strlen(buf));
    }
    static void run_Draw_head(chat_window* cw)
    {
      //1.绘制窗口
      //2.展示欢迎语
      int y,x;

      std::string s = "welcom to many chat room";
      
      while(1)
      {
        cw->Draw_head();
        //获得窗口的最大行数和列数
        getmaxyx(cw->head_,y,x);
        cw->put_string_to_window(cw->head_,y/3,x/3,s); 
        sleep(1);
      }

    }

    static void run_out_put(chat_window* cw,chat_client* cc)
    {
      std::string recv_msg;
      message msg;

      //行数
      int y =1;
      //表示当前窗口的最大行数和最大列数
      int yy,xx;
      getmaxyx(cw->out_put,yy,xx);
        cw->Draw_out_put();
      while(1)
      {
        //cw->Draw_out_put();
        cc->recv_msg(&recv_msg);
        msg.deseruakuze(recv_msg);
        //展示数据格式如下： name-from:
        std::string show_msg;
        show_msg += msg.get_name();
        show_msg += "--";
        show_msg += msg.get_from();
        show_msg += ":";
        show_msg += msg.get_msg();

        if(y > yy-2)
        {
          y = 1;
          cw->Draw_out_put();
        }
        cw->put_string_to_window(cw->out_put,y,1,show_msg);
        ++y;

        std::string user_info;
        user_info += msg.get_name();
        user_info += "--";
        user_info +=msg.get_from();
        cc->push_user(user_info);
      }
    }

    static void run_in_put(chat_window* cw,chat_client* cc)
    {
      message msg;
      //设置name from msg userid
      msg.set_name(cc->get_my_info().Name_);
      msg.set_from(cc->get_my_info().From_);
      msg.set_user_id(cc->get_my_info().Uer_ID);
      std::string p_enter = "please enter:";
      //用户输入的消息
      std::string user_enter_msg;

      //序列化之后的消息
      std::string send_msg;
      while(1)
      {
        cw->Draw_in_put();
        cw->put_string_to_window(cw->in_put,3,3,p_enter);
        //从窗口当中获取数据，放到send_msg
        cw->get_string_from_win(cw->in_put,&user_enter_msg);
        msg.set_msg(user_enter_msg);
        msg.serialize(&send_msg); 
        
        cc->send_msg(send_msg);
      }
    }

    static void run_user_list(chat_window* cw,chat_client* cc)
    {

      //当前窗口的最大行数和最大列数
      int yy,xx;
      while(1)
      {
        cw->Draw_user_list();
        getmaxyx(cw->user_list,yy,xx);

        std::vector<std::string> u_list =  cc->get_online_user();
        for(auto it : u_list)
        {
          int y = 1;
          cw->put_string_to_window(cw->user_list,++y,1,it);
        }
      }
    }

    static void* Draw_window(void* arg)
    {
      pthread_num* pn = (pthread_num*) arg;
      chat_window* cw = pn->win_p;
      int num = pn->p_n;
      switch (num)
      {
        case 0:
          //即可以绘制窗口，也可以打出欢迎语
          run_Draw_head(cw);
          break;
        case 1:
          run_out_put(cw,pn->c_c);
          break;
        case 2:
          run_user_list(cw,pn->c_c);
          break;
        case 3:
          run_in_put(cw,pn->c_c);
          break;
        default:
          break;
      }
      //printf("i am %d  pthread\n",pn->p_n);
      delete pn;
      return NULL;
    }
    void Start(chat_client* cc)
    {
      int i=0;
      pthread_t tid;
      for(;i<4;++i)
      {
        pthread_num* pn = new pthread_num(this,i,cc);
        int ret = pthread_create(&tid,NULL,Draw_window,(void*)pn);
        if(ret < 0)
        {
          printf("create thread failed\n");
          exit(1);
        }
        v_p_s.push_back(tid);
          
      }
      for(i =0;i<4;++i)
      {
        pthread_join(v_p_s[i],NULL);
      }
    }

    

};
