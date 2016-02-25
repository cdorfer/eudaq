#include "trigger_controll.h"
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#define SERVER_PORT 80
//#define HOST "128.146.33.69"
using namespace std;
#include <libconfig.h++>
using namespace libconfig;

    trigger_controll::trigger_controll()
    {
        parser = new(http_responce_pars);
        //this->ip_adr = "192.168.0.10";
        this->ip_adr = "192.168.1.120";
    }
    void trigger_controll::set_scintillator_delay(int d)
    {
        this->scintillator_delay =d;
    }

    void trigger_controll::set_plane_1_delay(int d)
    {
        this->plane_1_delay=d;
    }
    void trigger_controll::set_plane_2_delay(int d)
    {
        this->plane_2_delay=d;
    }
    void trigger_controll::set_plane_3_delay(int d)
    {
        this->plane_3_delay=d;
    }
    void trigger_controll::set_plane_4_delay(int d)
    {
        this->plane_4_delay=d;
    }
    void trigger_controll::set_plane_5_delay(int d)
    {
        this->plane_5_delay=d;
    }
    void trigger_controll::set_plane_6_delay(int d)
    {
        this->plane_6_delay=d;
    }
    void trigger_controll::set_plane_7_delay(int d)
    {
        this->plane_7_delay=d;
    }
    void trigger_controll::set_plane_8_delay(int d)
    {
        this->plane_8_delay=d;
    }
    void trigger_controll::set_pad_delay(int d)
    {
        this->pad_delay=d;
    }

    int trigger_controll::get_scintillator_delay()
    {
        return this->scintillator_delay;
    }

    int trigger_controll::get_plane_1_delay()
    {
        return this->plane_1_delay;
    }
    int trigger_controll::get_plane_2_delay()
    {
        return this->plane_2_delay;
    }
    int trigger_controll::get_plane_3_delay()
    {
        return this->plane_3_delay;
    }
    int trigger_controll::get_plane_4_delay()
    {
        return this->plane_4_delay;
    }
    int trigger_controll::get_plane_5_delay()
    {
        return this->plane_5_delay;
    }
    int trigger_controll::get_plane_6_delay()
    {
        return this->plane_6_delay;
    }
    int trigger_controll::get_plane_7_delay()
    {
        return this->plane_7_delay;
    }
    int trigger_controll::get_plane_8_delay()
    {
        return this->plane_8_delay;
    }

    int trigger_controll::get_pad_delay()
    {
        return this->pad_delay;
    }

    int trigger_controll::set_delays()
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?a0=%d&a1=%d&a2=%d&a3=%d&a4=%d&a5=%d&a6=%d&a7=%d&a8=%d&a9=%d",scintillator_delay,
                plane_1_delay,plane_2_delay,plane_3_delay,plane_4_delay,
                plane_5_delay,plane_6_delay,plane_7_delay,plane_8_delay,pad_delay);
        return this->http_backend(cmd_str);
    }

    int trigger_controll::enable(bool state)
    {
        char cmd_str[128];
        if(state)
            sprintf(cmd_str,"/a?k=7");
        else
            sprintf(cmd_str,"/a?k=0");

        return this->http_backend(cmd_str);
    }
    int trigger_controll::get_coincidence_pulse_width()
    {
        return coincidence_pulse_width;
    }
    int trigger_controll::get_coincidence_edge_width()
    {
        return coincidence_edge_width;
    }
    int trigger_controll::set_coincidence_pulse_width(int width)
    {
        char cmd_str[128];
        coincidence_pulse_width = width;
        sprintf(cmd_str,"/a?r=%d",width);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_coincidence_edge_width(int width)
    {
        char cmd_str[128];
        coincidence_edge_width = width;
        sprintf(cmd_str,"/a?q=%d",width);
        return this->http_backend(cmd_str);
    }
    //send coincidence_pulse_width and send_coincidence_edge_width
    //send the stored value of coincidence_pulse_width and
    //coincidence_edge_width
    int trigger_controll::send_coincidence_pulse_width()
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?r=%d",coincidence_pulse_width);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::send_coincidence_edge_width()
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?q=%d",coincidence_edge_width);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_Pulser_freq(double freq)
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?o=%f",freq);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_Pulser_width(double freq,int width)
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?o=%f&p=%d",freq, width);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_handshake_delay(int delay)
    {
        char cmd_str[128];
        this->handshake_delay = delay;
        sprintf(cmd_str,"/a?m=%d",delay);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_handshake_mask(int mask)
    {
        char cmd_str[128];
        this->handshake_mask = mask;
        sprintf(cmd_str,"/a?l=%d",mask);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::get_handshake_delay()
    {
        return this->handshake_delay ;
    }
    int trigger_controll::get_handshake_mask()
    {
        return this->handshake_mask;
    }
    int trigger_controll::send_handshake_delay()
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?m=%d",this->handshake_delay );
        return this->http_backend(cmd_str);
    }
    int trigger_controll::send_handshake_mask()
    {
        char cmd_str[128];
        sprintf(cmd_str,"/a?l=%d",this->handshake_mask);
        return this->http_backend(cmd_str);
    }
    int trigger_controll::set_time()
    {
        char cmd_str[128];
        time_t t;
        unsigned int l,h;
        time(&t);
        t = t*1000;
        l = t & 0xFFFFFFFF;
        h = t >>32;
        sprintf(cmd_str,"/a?s=%u&t=%u",h,l);
        return this->http_backend(cmd_str);
    }
    /*************************************************************************
     * read_back
     * return a Readout_Data structure populated with the return data or NULL
     *************************************************************************/
    Readout_Data * trigger_controll::read_back()
    {
       Readout_Data * ret_data=NULL;
       if(http_backend((char*)"/a?R=1") !=0)
           return NULL;
       //May be beter to check if == in size
       if(parser->get_content_length() >= TRIGGER_LOGIC_READBACK_FILE_SIZE)
       {
           ret_data = (Readout_Data*) malloc(sizeof(Readout_Data));
           char *raw_ret = parser->get_content();
           if(ret_data == NULL)
               return NULL;
           memcpy(ret_data,raw_ret,sizeof(Readout_Data));
       }
       return ret_data;
    }

    int trigger_controll::http_backend(char * command)
    {
        int socket_desc;
        struct sockaddr_in server;
        char message[1024];
        char server_reply[2000];
        int recv_len=0;
        struct timeval tv;
        fd_set fdset;
        parser->clean_up();
            //Create socket
    //    puts("creating socket\n");
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1)
        {
            printf("Could not create socket");
            sprintf(error_str,"Error: Could not create socket");
            return 1;
        }
    //    puts("socket created\n");

        server.sin_addr.s_addr = inet_addr(this->ip_adr.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons( SERVER_PORT );

        // make non blocking so it does not lockup
        fcntl(socket_desc, F_SETFL, O_NONBLOCK);

        connect(socket_desc , (struct sockaddr *)&server , sizeof(server));
        //Connect to remote server
        /*if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
            puts("connect error");
            return 1;
        }
        */
        FD_ZERO(&fdset);
        FD_SET(socket_desc, &fdset);
        tv.tv_sec = 2;             /* 10 second timeout */
        tv.tv_usec = 0;
        if (select(socket_desc + 1, NULL, &fdset, NULL, &tv) == 1)
        {
          int so_error;
          socklen_t len = sizeof so_error;

          getsockopt(socket_desc, SOL_SOCKET, SO_ERROR, &so_error, &len);
           //see if the socket actualy connected
          if (so_error != 0) {
              printf("Error connecting\n");
              sprintf(error_str,"Error: Error connecting");
              close(socket_desc);
              return 1;
          }
        }
        //Send some data
        sprintf( message , "GET %s HTTP/1.0\r\n\r\n",command);
        if( send(socket_desc , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            sprintf(error_str,"Error: Send failed");
            close(socket_desc);
            return 1;
        }

        int flags =0;
        if (-1 == (flags = fcntl(socket_desc, F_GETFL, 0)))
            flags = 0;
        fcntl(socket_desc, F_SETFL, flags & (!O_NONBLOCK));

        //Receive a reply from the server
        if( (recv_len = recv(socket_desc, server_reply , 2000 , 0) )< 0)
        {
            puts("recv failed");
            sprintf(error_str,"Error: recv failed");
            close(socket_desc);
            return 1; //error
        }else{
            parser->pars_more(server_reply,recv_len);
            while(!parser->is_done()) //we have more data to get from the serevre
            {
                int toget = parser->get_how_much_more();
                if(toget <2000)
                    toget = 2000;
                if( (recv_len = recv(socket_desc, server_reply , toget , 0) )< 0)
                {
                    puts("recv failed");
                    sprintf(error_str,"Error: recv failed");
                    close(socket_desc);
                    return 1; //error
                }
                parser->pars_more(server_reply,recv_len);

            }
        }
        close(socket_desc);
        puts(parser->get_content());
        sprintf(error_str,"OK");

        return 0;
    }
    char * trigger_controll::get_error_str()
    {
        return this->error_str;
    }
    int trigger_controll::reset_counts()
    {
        char str[32];
        sprintf(str,"/a?c=0");
        return this->http_backend(str);
    }
    int trigger_controll::set_coincidence_enable(int en)
    {
        char str[32];
        sprintf(str,"/a?e=%d",en);
        return this->http_backend(str);
    }
    int trigger_controll::set_prescaler(int scaler)
    {
        char str[32];
        sprintf(str,"/a?f=%d",scaler);
        return this->http_backend(str);
    }
    /*
    int trigger_controll::set_mux(int mux_comand)
    {
        char str[32];
        sprintf(str,"/a?s=%d",mux_comand);
        return this->http_backend(str);
    }
    */
    int trigger_controll::set_prescaler_delay(int delay)
    {
        char str[32];
        sprintf(str,"/a?g=%d",delay);
        return this->http_backend(str);
    }
    int trigger_controll::set_pulser_delay(int delay)
    {
        char str[32];
        sprintf(str,"/a?j=%d",delay);
        return this->http_backend(str);
    }
    void trigger_controll::set_ip_adr(std::string ip_address)
    {
        this->ip_adr = ip_address;
    }

    std::string trigger_controll::get_ip_adr()
    {
        return this->ip_adr;
    }

    int trigger_controll::load_from_file(char *fname)
    {
        Config *cfg;
        cfg = new Config();
        // Read the file. If there is an error, market and contunue on.
        try
        {
          cfg->readFile(fname);
        }
        catch(const FileIOException &fioex)
        {
            printf("I/O error while reading config file.\n");
            return 1;
        }
        catch(const ParseException &pex)
        {
            std::string str;
            printf("Error parsing config file: " );//+ pex.getLine() + " - " + pex.getError();

    //      return;
        }
        Setting &root = cfg->getRoot();
        if( root.exists("ip_adr")) //make a default config file if settings donot exists
        {
            std::string str;
            root.lookupValue("ip_adr",str);
           ip_adr = str;
        }else{
            ip_adr = "192.168.0.10";
        }
        if( root.exists("delays")) //make a default config file if settings donot exists
        {
            int i;
            Setting &delays = root["delays"];

            if(delays.exists("scintillator"))
            {
                delays.lookupValue("scintillator",i);
                set_scintillator_delay(i);
            }
            if(delays.exists("plane1"))
            {
                delays.lookupValue("plane1",i);
                set_plane_1_delay(i);
            }
            if(delays.exists("plane2"))
            {
                delays.lookupValue("plane2",i);
                set_plane_2_delay(i);
            }
            if(delays.exists("plane3"))
            {
                delays.lookupValue("plane3",i);
                set_plane_3_delay(i);
            }
            if(delays.exists("plane4"))
            {
                delays.lookupValue("plane4",i);
                set_plane_4_delay(i);
            }
            if(delays.exists("plane5"))
            {
                delays.lookupValue("plane5",i);
                set_plane_5_delay(i);
            }
            if(delays.exists("plane6"))
            {
                delays.lookupValue("plane6",i);
                set_plane_6_delay(i);
            }
            if(delays.exists("plane7"))
            {
                delays.lookupValue("plane7",i);
                set_plane_7_delay(i);
            }
            if(delays.exists("plane8"))
            {
                delays.lookupValue("plane8",i);
                set_plane_8_delay(i);
            }
            if(delays.exists("pad"))
            {
                delays.lookupValue("pad",i);
                set_pad_delay(i);
            }
            if(set_delays())
            {
                printf("Error setting delays: %s\n",this->error_str);
                return 1;
            }
        }
        float f;
        int i;
        if(root.exists("pulser_width"))
        {
            root.lookupValue("pulser_width",i);
        }
        if( root.exists("pulser_freq"))
        {
            root.lookupValue("pulser_freq", f);
            if(set_Pulser_width(f,i)){ //error
                printf("error setting pulser dutty");
                return 1;
            }
        }
        if( root.exists("prescaler"))
        {
            root.lookupValue("prescaler", i);
            if(set_prescaler(i))
                return 1;
        }
        if( root.exists("prescaler_delay")){
            root.lookupValue("prescaler_delay", i);
            if(set_prescaler_delay(i))
                return 1;
        }
        /*
        if(! root.exists("readout_period")) //make a default config file if settings donot exists
        {
             root.add("readout_period",Setting::TypeInt) = 1000;
        }
        */
        if(! root.exists("use_planes")) //make a default config file if settings donot exists
        {
            bool use;
            int en=0;
            Setting &use_planes = root["use_planes"];
            if(use_planes.exists("scintillator")){
                use_planes.lookupValue("scintillator", use);
                en =(use);
            }
            if(use_planes.exists("plane1")){
                use_planes.lookupValue("plane1", en);
                use += en<<1;
            }
            if(use_planes.exists("plane2")){
                use_planes.lookupValue("plane2", en);
                use += en<<2;
            }
            if(use_planes.exists("plane3")){
               use_planes.lookupValue("plane3", en);
               use += en<<3;
            }
            if(use_planes.exists("plane4")){
                use_planes.lookupValue("plane4", en);
                use += en<<4;
            }
            if(use_planes.exists("plane5")){
                use_planes.lookupValue("plane5", en);
                use += en<<5;
            }
            if(use_planes.exists("plane6")){
                use_planes.lookupValue("plane6", en);
                use += en<<6;
            }
            if(use_planes.exists("plane7")){
                use_planes.lookupValue("plane7", en);
                use += en<<7;
            }
            if(use_planes.exists("plane8")){
                use_planes.lookupValue("plane8", en);
                use += en<<8;
            }
            if(use_planes.exists("pad")){
                use_planes.lookupValue("pad", en);
                use += en<<9;
            }

            if(set_coincidence_enable(en))
                return 1;
        }
        if( root.exists("coincidence_edge_width"))
        {
            root.lookupValue("coincidence_edge_width", i) ;
            if(set_coincidence_edge_width(i))
                return 1;
        }
        if(root.exists("coincidence_pulse_width"))
        {
            root.lookupValue("coincidence_pulse_width", i) ;
            if(set_coincidence_pulse_width(i))
                return 1;
        }
        if( root.exists("handshake_mask"))
        {
            root.lookupValue("handshake_mask", i) ;
            if(set_handshake_mask(i))
                return 1;
        }
        if(root.exists("handshake_delay"))
        {
            root.lookupValue("handshake_delay", i) ;
            if(set_handshake_delay(i))
                return 1;
        }
        this->set_time();//send the curent time stamp
        return 0;
    }
