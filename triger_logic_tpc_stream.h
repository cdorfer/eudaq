#ifndef TRIGGER_LOGIC_TPC_STREAM_H
#define TRIGGER_LOGIC_TPC_STREAM_H 1
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
//#define HOST_NAME "128.146.33.69"
#define HOST_PORT 80
typedef void (*Triger_CallbackType)(int);

struct RET_DATA{
    //! id of the readout incomented by 1 each readout.
    unsigned int id;
    //! 28 bit count of trigger input pulses
    unsigned int trigger_counts[10];
    //! 28 bit count of coincidences without the sinillator
    unsigned int coincidence_count_no_sin;
    //! 28 bit count of coincidences
    unsigned int coincidence_count;
    //! 28 bit count of beam curent pulses
    unsigned int beam_curent;
    //! 28 bit count of prescaler output pulses
    unsigned int prescaler_count; 
    //! 28 bit count of prescaler xor pulser
    unsigned int prescaler_count_xor_pulser_count;
   
    unsigned int pulser_delay_and_xor_pulser_count;
    //! raw 28 bit counter output form the handshake unit
    unsigned int handshake_count;
     //! Unimplimented
    unsigned int coincidence_rate;
    //! Unimplimented
    unsigned int prescaler_rate;
   //! 64bit mS time stamp if time is set = Epoch *1000+ mS
    unsigned long time_stamp; 
    //! simple check sum of all retured bytes 
    unsigned int check_sum;
    //! should be 'E" = 69 desimal
    unsigned int end_flag;
}typedef Readout_Data;

class Triger_Logic_tpc_Stream
{
   private:
   int socket_desc;
   int error;
   bool is_socket_open;
   Readout_Data *pars_stream_ret(char *stream);
   std::string ip_adr;
   public:
   /*******************************************************************//*!
    * A constructor 
    *************************************************************************/

    Triger_Logic_tpc_Stream();
    /*******************************************************************//*!
     * Dumps redout data to the screen
     * @param data read out from the trigger box
     *************************************************************************/
    void dump_readout(Readout_Data *readout);
   /*******************************************************************//*!
    * Returs the stataus of the socket conection to the trigger box.  
    *
    * It dese not test the connection to see if it is god or not just if 
    * the socket is open or not.
    * @return true on connected fase on disconnected 
    * @see set_coincidence_pulse_width()
    *************************************************************************/
    bool is_open();
   /*******************************************************************//*!
    * Open the socket connecion to the trigger box for readback.
    *************************************************************************/
   int open();
   /*******************************************************************//*!
    * Closes the socket connecion to the trigger box for readback.
    * @return 0 on sucess true on fail
    ************************************************************************/
   int close();
   /**********************************************************************//*!
    * set the ip adress of the trigger controll box
    * @param ip_address - the ip address of the trigger controll box
    *************************************************************************/
   void set_ip_adr(std::string);
   /**********************************************************************//*!
    * gets the ip adress of the trigger controll box
    * @return std::string contiaining the ip address.
    *************************************************************************/
   std::string get_ip_adr();
   /*******************************************************************//*!
    * Des the actual readout form the trigger box.
    * @return pointer to a Readout_Data strucuer on sucess NULL on fail
    ************************************************************************/
   Readout_Data *timer_handler();
};

#define TRIGGER_LOGIC_HEADER	0
#define TRIGGER_LOGIC_READBACK_ID	4
#define TRIGGER_COUNT_0     8
#define TRIGGER_COUNT_1     12
#define TRIGGER_COUNT_2     16
#define TRIGGER_COUNT_3     20
#define TRIGGER_COUNT_4     24
#define TRIGGER_COUNT_5     28
#define TRIGGER_COUNT_6     32
#define TRIGGER_COUNT_7     36
#define TRIGGER_COUNT_8     40
#define TRIGGER_COUNT_9     44
#define TRIGGER_LOGIC_COINCIDENCE_CNT       48
#define TRIGGER_LOGIC_BEAM_CURRENT          52
#define TRIGGER_LOGIC_PRESCALER_CNT         56
#define TRIGGER_LOGIC_PRESCALER_XOR_PULSER_CNT      60
#define TRIGGER_LOGIC_PRESCALER_XOR_PULSER_AND_PRESCALER_DELAYED_CNT    64
#define TRIGGER_LOGIC_PULSER_DELAY_AND_XOR_PULSER_CNT       68
#define TRIGGER_LOGIC_HANDSHAKE_CNT     72
#define TRIGGER_LOGIC_COINCIDENCE_CNT_NO_SIN       76
#define TRIGGER_LOGIC__S       80

#define TRIGGER_LOGIC_TIME_STAMP_HIGH 84
#define TRIGGER_LOGIC_TIME_STAMP_LOW 88
#define TRIGGER_LOGIC_CHECK_SUM     96
#define SLAVE_REG_24    92
#define TRIGGER_LOGIC_END_FAG     92
#define TRIGGER_LOGIC_READBACK_FILE_SIZE    104


#endif
