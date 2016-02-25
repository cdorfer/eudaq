#ifndef HTTP_RESPONCE_PARS_H
#define HTTP_RESPONCE_PARS_H

#define HTTP_RESPONCE_START 0
#define HTTP_RESPONCE_HAVE_HEADER 1
#define HTTP_RESPONCE_HAVE_CONTENT 2
#include <string.h>
#include <stdlib.h>

class http_responce_pars
{
public:
    http_responce_pars();
    int pars_more(char *str,int data_len);
    bool is_done();
    bool have_headers();
    long get_content_length();
    char *get_content_type();
    long get_how_much_more();
    char * get_content();
    int get_responce_code();
    void clean_up();
    ~http_responce_pars();
private:
    char *trim_past_semi(char *str);
    char *trim_past_first_white(char*str);

    int pars_header();
    int state;
    unsigned long content_length;
    char content_type[128];
    char tmp_buff[1024];
    int responce_code;
    char conection_state[32]; //curently unused this holds the conetcion sate
    char *content;
    char *tmp_content;
    int h_offset;
    unsigned long contet_offset;
};

#endif // HTTP_RESPONCE_PARS_H
