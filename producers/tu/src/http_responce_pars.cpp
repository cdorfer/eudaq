#include "http_responce_pars.h"

http_responce_pars::http_responce_pars()
{
    state = HTTP_RESPONCE_START;
    tmp_content = NULL;
    content = NULL;
    h_offset =0;
    content_length =0;
    contet_offset = 0;
    content_type[0]='\0';
    responce_code = 0;
}
int http_responce_pars::pars_more(char *str,int data_len)
{
    char *tmp_ptr;
    unsigned long content_len =0;
    switch(state)
    {
        case HTTP_RESPONCE_START:
            //two cases have header or not
            memcpy(tmp_buff+h_offset,str,data_len);
            h_offset += data_len;

            tmp_ptr=strstr(tmp_buff,"\r\n\r\n");
            if(tmp_ptr !=NULL)
            {
                *(tmp_ptr+2) = '\0'; // null terminate the second \r\n to make header parsing easer
                tmp_ptr = tmp_ptr + 4;
                pars_header();
                state = HTTP_RESPONCE_HAVE_HEADER;
                content_len = data_len- (tmp_ptr-tmp_buff); //data - header length
                this->content = (char*) malloc(this->content_length);
                if(content_len >= this->content_length)//sanity check so we dont over runn the content buffer
                { //done have everything
                    memcpy( this->content, tmp_ptr, this->content_length);
                    state = HTTP_RESPONCE_HAVE_CONTENT;
                }else {//content will come in more packets
                    memcpy( this->content, tmp_ptr, content_len);
                }
                this->contet_offset = content_len;
                //server_reply	"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-length: 2\r\nConnection: close\r\n\r\nOK\000\000\000@´ÿÿÿ\177\000\000\000\000\000\000\001\000\000\000\000\000\000\000\000\000\000\000\000ÿ\212\000\000\000\000\000@ÿ\212\000\000\000\000\000\001\000\000\000\000\000\000\000kþ?#=\000\000\000 xi\000\000\000\000\000\000\000\000\000\026\000\000\000\020¶ÿÿÿ\177\000\000¨ÝU#\001\000\000\000\004\000\000\000\000\000\000\000Ð´ÿÿÿ\177\000\000\000\000\000\000\000\000ð?\217µÿÿÿ\177\000\000\000\001\000\000\002\000\000\..."	char [2000]
            }else{
                memcpy(tmp_buff+h_offset,str,data_len);
                h_offset += data_len;
            }
            break;
        case HTTP_RESPONCE_HAVE_HEADER:
            if(this->content_length-this->contet_offset <= data_len)
            {
                memcpy( this->content+this->contet_offset , str, data_len);
                this->contet_offset += data_len;
                if(this->contet_offset == content_length) //we are done
                {
                    state = HTTP_RESPONCE_HAVE_CONTENT;
                }
            }else //have more data in the read buff than allicated coopy upto allocation
            {
                memcpy( this->content+this->contet_offset , str, this->content_length-this->contet_offset);
                state = HTTP_RESPONCE_HAVE_CONTENT;

            }
            break;
    }
    return state;
}

int http_responce_pars::pars_header()
{
    char  *tmp_ptr = tmp_buff;
    char  *tmp_ptr1 = tmp_buff;
    while((tmp_ptr1 = strstr(tmp_ptr,"\r\n"))!=NULL)
    {
           *tmp_ptr1='\0';
           tmp_ptr1+=2;
           if(strstr(tmp_ptr,"Content-Type")){
               tmp_ptr = trim_past_semi(tmp_ptr);
               strcpy(this->content_type,tmp_ptr);
           }else
           if(strstr(tmp_ptr,"Content-length")){
               tmp_ptr = trim_past_semi(tmp_ptr);
               content_length = atoi(tmp_ptr);
           }else
           if(strstr(tmp_ptr,"HTTP")){
               tmp_ptr = trim_past_first_white(tmp_ptr);
               if(tmp_ptr != NULL)
               {
                responce_code = atoi(tmp_ptr);
               }else
                   responce_code = -1;
           }else if(strstr(tmp_ptr,"Connection")){
               tmp_ptr = trim_past_semi(tmp_ptr);
               strcpy(this->conection_state,tmp_ptr);
           }
           tmp_ptr = tmp_ptr1;
    }
    return responce_code;
}
char * http_responce_pars::trim_past_semi(char *str)
{
    char *tmp_ptr;
    unsigned i;
    char have_semi = false;
    tmp_ptr = str;
    for(i=0;i<strlen(str);i++)
    {
        if(have_semi == true){ //skip only white spacess
            if( (*(tmp_ptr) != ' ') && (  *(tmp_ptr) != '\t') )
                return tmp_ptr;
        }else if( *tmp_ptr == ':')
        {
            have_semi = true;
        }
        tmp_ptr++;
    }
    return NULL;
}
char * http_responce_pars::trim_past_first_white(char *str)
{
    char *tmp_ptr;
    unsigned int i;
    char have_white = false;
    tmp_ptr = str;
    for(i=0;i<strlen(str);i++)
    {
        if(have_white == true){ //skip only white spacess
            if( (*(tmp_ptr) != ' ') && (  *(tmp_ptr) != '\t') )
                return tmp_ptr;
        }else if( *tmp_ptr == ' ' || *tmp_ptr == '\t' )
        {
            have_white = true;
        }
        tmp_ptr++;
    }
    return NULL;
}
bool http_responce_pars::is_done()
{
    if(state == HTTP_RESPONCE_HAVE_CONTENT)
        return true;
    return false;
}
bool http_responce_pars::have_headers()
{
    if(state >= HTTP_RESPONCE_HAVE_HEADER)
        return true;
    return false;
}


long http_responce_pars::get_content_length()
{
    return content_length;
}
int http_responce_pars::get_responce_code()
{
    return responce_code;
}

long http_responce_pars::get_how_much_more()
{
    return (this->content_length- this->contet_offset);
}

char* http_responce_pars::get_content()
{
    return this->content;
}
char* http_responce_pars::get_content_type()
{
    return this->content_type;
}

void http_responce_pars::clean_up()
{
    if(this->content != NULL)
        free(this->content);
    this->content = NULL;
    state = HTTP_RESPONCE_START;
    tmp_content = NULL;
    h_offset =0;
    content_length =0;
    contet_offset = 0;
    content_type[0]='\0';
    responce_code = 0;

}

http_responce_pars::~http_responce_pars()
{
    this->clean_up();
}
