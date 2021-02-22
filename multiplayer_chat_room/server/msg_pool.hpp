#pragma once 
#include<queue>
#include<iostream>
#include<pthread.h>
#include<string>

#define MSG_POOL_CAPACITY 1024
class msg_pool
{
  public:
    msg_pool()
    {
      capacity_ = MSG_POOL_CAPACITY;
      pthread_mutex_init(&msg_queue_lock,NULL);
      pthread_cond_init(&product_queue,NULL);
      pthread_cond_init(&consumer_queue,NULL);
    }

    ~msg_pool()
    {
      pthread_mutex_destroy(&msg_queue_lock);
      pthread_cond_destroy(&product_queue);
      pthread_cond_destroy(&consumer_queue);
    }

    void push_msg_pool(std::string& msg)
   {
      pthread_mutex_lock(&msg_queue_lock);
      while(is_full())
      {
        pthread_cond_wait(&product_queue,&msg_queue_lock);
      }
      msg_queue.push(msg);
      pthread_mutex_unlock(&msg_queue_lock);
      pthread_cond_signal(&consumer_queue);
    }
    void pop_msg_pool(std::string* msg)
    {
      pthread_mutex_lock(&msg_queue_lock);
      while(msg_queue.empty())
      {
        pthread_cond_wait(&consumer_queue,&msg_queue_lock);
      }
      *msg = msg_queue.front();
      msg_queue.pop();
      pthread_mutex_unlock(&msg_queue_lock);
      pthread_cond_signal(&product_queue);
    }
  private:
    bool is_full()
    {
      if(msg_queue.size() == capacity_)
      {
        return true;
      }
      return false;
    }
  private:
    std::queue<std::string> msg_queue;
    //约束队列大小，防止队列无限扩容，导致内存过大，被操作系统强杀
    size_t capacity_;
    //互斥锁
    pthread_mutex_t msg_queue_lock;
    //生产者条件变量
    pthread_cond_t product_queue;
    //消费者条件变量
    pthread_cond_t consumer_queue;
};
