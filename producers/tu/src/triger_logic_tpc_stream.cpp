#include<stdio.h>
#include<string.h>    //strlen
#include <stdlib.h>

#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include "triger_logic_tpc_stream.h"
//#define HOST_NAME "128.146.33.69"
#define STREAM_HOST_PORT 8080


   Triger_Logic_tpc_Stream::Triger_Logic_tpc_Stream()
   {
      socket_desc = -1;
      error = 0;
      is_socket_open =false;
      this->ip_adr = "192.168.1.120";
   }
   bool Triger_Logic_tpc_Stream::is_open()
   {
       return is_socket_open;
   }

   int Triger_Logic_tpc_Stream::open()
   {    
       struct sockaddr_in server;
       struct timeval tv;
       fd_set fdset;
       is_socket_open =false;
     
    //Create socket
//    puts("creating socket\n");
       socket_desc = socket(AF_INET , SOCK_STREAM , 0);
       if (socket_desc == -1)
       {
           printf("Could not create socket");
           error =1;
           return 1;
       }
    puts("socket created\n");
         
       server.sin_addr.s_addr = inet_addr(this->ip_adr.c_str());
       server.sin_family = AF_INET;
       server.sin_port = htons( STREAM_HOST_PORT);
 
   //Connect to remote server
       // make non blocking so it does not lockup
       fcntl(socket_desc, F_SETFL, O_NONBLOCK);

        connect(socket_desc , (struct sockaddr *)&server , sizeof(server));
       //Connect to remote server

       FD_ZERO(&fdset);
       FD_SET(socket_desc, &fdset);
       tv.tv_sec = 10;             /* 10 second timeout */
       tv.tv_usec = 0;
       int ret= select(socket_desc + 1, NULL, &fdset, NULL, &tv);
       if (ret <= 0)
       {
           printf("Error connecting\n");
           ::close(socket_desc);
           return 1;
           /*
            * Asume error conetining error is not therown on deade server
         int so_error;
         socklen_t len = sizeof so_error;

         getsockopt(socket_desc, SOL_SOCKET, SO_ERROR, &so_error, &len);
          //see if the socket actualy connected
         if (so_error != 0) {
             printf("Error connecting\n");
             ::close(socket_desc);
             return 1;
         }
         //
         return 1;
         */
       }
       //make blocking again
       int flags =0;
       if (-1 == (flags = fcntl(socket_desc, F_GETFL, 0)))
           flags = 0;
       fcntl(socket_desc, F_SETFL, flags & (!O_NONBLOCK));

       puts("connect sucess");
       is_socket_open =true;
       return 0;
    }

    int Triger_Logic_tpc_Stream::close()
    {
        is_socket_open =false;
        return shutdown(socket_desc,SHUT_RDWR);
    }
    void Triger_Logic_tpc_Stream::set_ip_adr(std::string ip_address)
    {
        this->ip_adr = ip_address;
    }

    std::string Triger_Logic_tpc_Stream::get_ip_adr()
    {
        return this->ip_adr;
    }

/*****************************************************************************
 * timer_handler (int signum)
 * can not pass the timer handler to the set timer directly
 * so set the external callback to call timer_handler to readout the data
 *****************************************************************************/
Readout_Data * Triger_Logic_tpc_Stream::timer_handler ()
{
    char  server_reply[2000];
   // static int count = 0;
    int size;
    if(!is_socket_open)
    return NULL;
    //Send some data
    //

        if( send(socket_desc , "R\n" , 2 , 0) < 0)
       {
            is_socket_open = false;
   	     puts("Send failed");
         return NULL;
	}
     
    //Receive a reply from the server
    if((size =recv(socket_desc, server_reply , 2000 , 0)) < 0)
    {
        is_socket_open = false;
        puts("recv failed");
        return NULL;
    }
    puts(server_reply);
   Readout_Data * readout;
   readout = pars_stream_ret(server_reply);
	   if(readout !=NULL)
           dump_readout(readout);
	    else
	        puts("pars failed");
        return readout;
    
}
Readout_Data *Triger_Logic_tpc_Stream::pars_stream_ret(char *stream)
{
    char *start;
    int i;
    unsigned int *iptr;
    Readout_Data *readout;
    if((start=strstr(stream,"RS #"))==NULL)
        return NULL;
    readout = (Readout_Data*)malloc(sizeof(Readout_Data));
    iptr = (unsigned int *) start+4;
    readout->id = *iptr;
    for(i=0;i<4;i++)
            printf("%d %c %x, \t",i,*(start+i) ,*(start+i));

            printf("\n");
    for(i=4;i<TRIGGER_LOGIC_READBACK_FILE_SIZE;i=i+4)
    {
        iptr = (unsigned int *) (start +i);
        printf("%d %d, \t",i,*iptr);
        if(i%10==0)
            printf("\n");
    }
    for(i=0;i<10;i++)
    {
        iptr = (unsigned int *) (start +TRIGGER_COUNT_0+4*i);
        readout->trigger_counts[i] =  *iptr;
    }
    iptr = (unsigned int *) (start +TRIGGER_LOGIC_COINCIDENCE_CNT_NO_SIN);
    readout->coincidence_count_no_sin =  *iptr;
    iptr = (unsigned int *) (start +TRIGGER_LOGIC_COINCIDENCE_CNT);
    readout->coincidence_count =  *iptr;
//    printf("coincidence_count %d, \n",*iptr);
    iptr = (unsigned int *) (start +TRIGGER_LOGIC_BEAM_CURRENT);
    readout->beam_curent= *iptr;
//    printf("beam_current %d, \n",*iptr);
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_PRESCALER_CNT);
    readout->prescaler_count = *iptr;
//    printf("prescaler_count %d, \n",*iptr);


    iptr = (unsigned int *) (start + TRIGGER_LOGIC_PRESCALER_XOR_PULSER_CNT);
    readout->prescaler_count_xor_pulser_count = *iptr;
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_PRESCALER_XOR_PULSER_AND_PRESCALER_DELAYED_CNT);
    readout->pulser_delay_and_xor_pulser_count = *iptr;
    iptr = (unsigned int *) (start +TRIGGER_LOGIC_HANDSHAKE_CNT);
    readout->handshake_count = *iptr;
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_COINCIDENCE_CNT);
    readout->coincidence_count = *iptr;
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_PRESCALER_CNT);
    readout->prescaler_count = *iptr;
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_TIME_STAMP_HIGH);
    i = *iptr;
    readout->time_stamp = ((unsigned long)i)<<32;
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_TIME_STAMP_LOW);
    i = *iptr;
    readout->time_stamp =  readout->time_stamp | (((unsigned long)i) & 0xFFFFFFFF);
    /* rates are not implimented
    iptr = (unsigned int *) (start + TRIGGER_LOGIC_COINCIDENCE_RATE);
    readout->coincidence_rate = *iptr;
    printf("coincidence_rate %d, \n",*iptr);
   iptr = (unsigned int *) (start + TRIGGER_LOGIC_PRESCALER_RATE);
   readout->prescaler_rate = *iptr;
    printf("prescaler_rate %d, \n",*iptr);
    */
    return readout;
}
/*
Readout_Data *Triger_Logic_tpc_Stream::pars_stream_ret(char *stream)
{
    char *start;
    int i;
    unsigned int *iptr;
    Readout_Data *readout;
    if((start=strstr(stream,"RS #"))==NULL)
        return NULL;
    readout = (Readout_Data*)malloc(sizeof(Readout_Data));
    iptr = (unsigned int *) start+4;
    readout->id = *iptr;
    for(i=0;i<4;i++)
            printf("%d %c %x, \t",i,*(start+i) ,*(start+i));

            printf("\n");
    for(i=4;i<TRIGGER_LOGIC_READBACK_FILE_SIZE;i=i+4)
    {
        iptr = (unsigned int *) (start +i);
        printf("%d %x, \t",i,*iptr);
        if(i%10==0)
            printf("\n");
    }
    for(i=0;i<10;i++)
    {
        iptr = (unsigned int *) (start +TRIGGER_COUNT_0+4*i);
        readout->trigger_counts[i] =  *iptr;
    }
    iptr = (unsigned int *) (start +TRIGGER_COUNT_0+4*i);
    readout->trigger_counts[i] =  *iptr;
    return readout;
}
*/
void Triger_Logic_tpc_Stream::dump_readout(Readout_Data *readout)
{
    int i;
    printf("id: %d\ntrigger c:\t",readout->id);
    for(i=0;i<10;i++)
    {
        printf("%d\t",readout->trigger_counts[i]);
    }
    printf("%c\n",readout->end_flag);
}
