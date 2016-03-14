#ifndef TRIGGER_CONTROLL_H
#define TRIGGER_CONTROLL_H
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include "http_responce_pars.h"
#include "triger_logic_tpc_stream.h"
#include <libconfig.h++>
/************************************************************************//**
 *  trigger_controll cllass 
 *  a class for contraling the trigger box. 
 *
 *  Inless sted otherwise all set comads sond the setting to the triger box.
 *  set_scintillator_delay, set_plane_*_delay and set_pad_delay do not you
 *  must call  set_delays(); to send all the input delays at once.
 *
 *  requires: libconfig
 *  @see Triger_Logic_tpc_Stream for read out. 
 **************************************************************************/
class trigger_controll
{
public:
    trigger_controll();
    /*********************************************************************//*!
     * sets the scintillator delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for the scintillator -  
     * @see set_delays()
     ************************************************************************/
    void set_scintillator_delay(int d);  //sets the delays in the class but does not
    /********************************************************************//*!
     * sets the plane 1 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 1 
     * @see set_delays()
     ************************************************************************/
    void set_plane_1_delay(int d);      //wrigtht them to the fpga.  must call
    /********************************************************************//*!
     * sets the plane 2 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 2 
     * @see set_delays()
     ************************************************************************/
    void set_plane_2_delay(int d);      //set_delays to load it to the fpga
    /*******************************************************************//*!
     * sets the plane 3 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 3 
     * @see set_delays()
     ************************************************************************/
    void set_plane_3_delay(int d);
    /*******************************************************************//*!
     * sets the plane 4 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 4 
     * @see set_delays()
     ************************************************************************/
    void set_plane_4_delay(int d);
    /*******************************************************************//*!
     * sets the plane 5 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 5 
     * @see set_delays()
     ************************************************************************/
    void set_plane_5_delay(int d);
    /*******************************************************************//*!
     * sets the plane 6 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 6 
     * @see set_delays()
     ************************************************************************/
    void set_plane_6_delay(int d);
    /*******************************************************************//*!
     * sets the plane 7 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 7 
     * @see set_delays()
     ************************************************************************/
    void set_plane_7_delay(int d);
    /*******************************************************************//*!
     * sets the plane 8 delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for  plane 8 
     * @see set_delays()
     ************************************************************************/
    void set_plane_8_delay(int d);
    /*******************************************************************//*!
     * sets the pad delay localy set_delays must be caled to send
     * all deays to the trigger box
     * @param d the input delay for pad 
     * @see set_delays()
     ************************************************************************/
    void set_pad_delay(int d);

    /*******************************************************************//*!
     * @return the scintillator delay stored in the class. set by  set_scintillator_delay()
     * @see set_scintillator_delay()
     ************************************************************************/
    int get_scintillator_delay();
    /*******************************************************************//*!
     * @return the plane 1 delay stored in the class. set by set_plane_1_delay()
     * @see set_plane_1_delay()
     ************************************************************************/
    int get_plane_1_delay();
    /*******************************************************************//*!
     * @return the plane 2 delay stored in the class. set by set_plane_2_delay()
     * @see set_plane_2_delay()
     ************************************************************************/
    int get_plane_2_delay();
    /*******************************************************************//*!
     * @return the plane 3 delay stored in the class. set by set_plane_3_delay()
     * @see set_plane_3_delay()
     ************************************************************************/
    int get_plane_3_delay();
    /*******************************************************************//*!
     * @return the plane 4 delay stored in the class. set by set_plane_4_delay()
     * @see set_plane_4_delay()
     ************************************************************************/
    int get_plane_4_delay();
    /*******************************************************************//*!
     * @return the plane 5 delay stored in the class. set by set_plane_5_delay()
     * @see set_plane_5_delay()
     ************************************************************************/
    int get_plane_5_delay();
    /*******************************************************************//*!
     * @return the plane 6 delay stored in the class. set by set_plane_6_delay()
     * @see set_plane_6_delay()
     ************************************************************************/
    int get_plane_6_delay();
    /*******************************************************************//*!
     * @return the plane 7 delay stored in the class. set by set_plane_7_delay()
     * @see set_plane_7_delay()
     ************************************************************************/
    int get_plane_7_delay();
    /*******************************************************************//*!
     * @return the plane 8 delay stored in the class. set by set_plane_8_delay()
     * @see set_plane_8_delay()
     ************************************************************************/
    int get_plane_8_delay();
    /*******************************************************************//*!
     * @return the pad delay stored in the class. set by set_pad_delay()
     * @see set_pad_delay()
     ************************************************************************/
    int get_pad_delay();

    /*******************************************************************//*!
     * sends the set dellays stored in the class to the triger box 
     * @return 0 on sucess 1 an error 
     * @see set_*_delay()
     * @see get_error_str()
     ************************************************************************/
    int set_delays();   //loads all delays to the fpga must set all indivdualy first
    /*******************************************************************//*!
     * coincidence pulse width is the nuber of 2.5 nS clk cyces to hold 
     * coincidence out high.  this is also used internaly fof setting 
     * the scaler output pulse width.
     * @return the coincidence pulse width stored in the class. set by \
     *         set_coincidence_pulse_width()
     * @see set_coincidence_pulse_width()
     ************************************************************************/
    int get_coincidence_pulse_width();
    /*******************************************************************//*!
     * coincidence edge width is how many clk cycles to hold a rising edge of 
     * the input signals for detecting a coincidence.  i.e. if coincidence 
     * edge width is set to 3 their can be a riseing edges on inputs within a 
     * 3 clk cycle window and a coincidence pulse will be generated.
     * @return the coincidence edge stored in the class. set by \
     *         set_coincidence_pulse_width()
     * @see set_coincidence_pulse_width()
     ************************************************************************/
    int get_coincidence_edge_width();
    /*******************************************************************//*!
     * send the stored coincidence edge width to the trigger box;
     * @return 0 on sucess 1 on error
     * @see set_coincidence_pulse_width()
     ************************************************************************/
    int send_coincidence_edge_width(); //resend the stored coincidence_edge_width
    /*******************************************************************//*!
     * send the stored coincidence edge width to the trigger box;
     * @return 0 on sucess 1 on error
     * @see set_coincidence_pulse_width()
     * @see get_coincidence_pulse_width()
     ************************************************************************/
    int send_coincidence_pulse_width(); //resend the stored coincidence_pulse_width


    /*******************************************************************//*!
     * send the command to clear all count registers.
     * this must be called on starup after enable has been set
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int clear_triggercounts();

    /*******************************************************************//*!
     * Set the delay for trigger output 1 and 2
     * trigger out 1 = CANDIG
     * trigger out 2 = DRS4_TRIG_IN_CH2
     * the delay is 12 bits. trig 1 is stored in 11 downto 0
     * trig 2 delay is stored in 23 downto 12
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int set_trigger_12_delay(int delay);
    /*******************************************************************//*!
     * Set the delay for trigger output 3 PSI46_ATB_DTB
     * the delay is 12 bits stored in 11 downto 0
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int set_trigger_3_delay(int delay);
    /*******************************************************************//*!
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int current_reset();
    int clear_coni_disable(int mask);
    /*******************************************************************//*!
     * DONOT USE 
     *
     * Use Triger_Logic_tpc_Stream class for readback. 
     ************************************************************************/
    Readout_Data* read_back(); //DO NOT USE use stream readout
    /*******************************************************************//*!
     * Get the error string form the last send command .
     * @return pointer to a string describing the last error
     ************************************************************************/
    char * get_error_str();
    /*******************************************************************//*!
     * Reset all counters on the triger box. This must be called during setup.
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int reset_counts();
    /*******************************************************************//*!
     * Sets the globle enable. This is the trigger enable and others.
     * @param true = enable false = disable
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int enable(bool state);
    /*******************************************************************//*!
     * Sets the coincidence pulse width in 2.5 ns divisons 
     * pulse_width is stored localy and sent to the trigger box
     * @param width - the nuber of 2.5 ns perids to hold coincidence out high
     * @return 0 on sucess 1 an error 
     * @see send_coincidence_pulse_width()
     ************************************************************************/
    int set_coincidence_pulse_width(int width);
    /*******************************************************************//*!
     * Sets the coincidence input edge width in 2.5 ns divisons 
     * edge width  is stored localy and sent to the trigger box
     * @param width - the nuber of 2.5 ns perids to hold trigger edges high
     *                going in to the  coincidence unit
     * @return 0 on sucess 1 an error 
     * @see send_coincidence_pulse_width()
     ************************************************************************/
    int set_coincidence_edge_width(int width);

    /*******************************************************************//*!
     * Sets the pulser frequancy in Hz 
     * @param freq - pulser frequancy in Hz
     * @return 0 on sucess 1 an error 
     * @see set_Pulser_width()
     ************************************************************************/
    int set_Pulser_freq(double freq);
    /*******************************************************************//*!
     * Sets the pulser frequancy and the pulse width
     * max imim midth is 20000! 
     * @param freq - pulser frequancy in Hz
     * @param width- the nuber of 2.5 ns clk cyles to hold pulse high max 20000
     * @return 0 on sucess 1 an error 
     * @see set_Pulser_freq()
     ************************************************************************/
    int set_Pulser_width(double freq,int width);
    /*******************************************************************//*!
     * Sets the mask of which inputs to use for determing a coincidence
     * mask bits are as follows:
     *     0 - sicinilator 
     *     1 - plane 1 
     *     2 - plane 2 
     *     3 - plane 3 
     *     4 - plane 4 
     *     5 - plane 5 
     *     6 - plane 6 
     *     7 - plane 7 
     *     8 - plane 8 
     *     9 - pad 
     * @param en - enable maks 
     * @return 0 on sucess 1 an error 
     ************************************************************************/
    int set_coincidence_enable(int en);
    /*******************************************************************//*!
     * Set the prescaler scaler 
     * @param scaler - nubre of coincidence required to generate a prescaler 
     *		       pullse. Range 1 cto 2^10-1
     * @return 0 on sucess 1 an error 
     ************************************************************************/
    int set_prescaler(int scaler);
    /*******************************************************************//*!
     * Un implimented do not use 
     *************************************************************************/
    int set_mux(int mux_comand); // not iwplimented do not use 
    /*******************************************************************//*!
     * @param delay - the prescaler delay  must be > 4
     ************************************************************************/
    int set_prescaler_delay(int delay);
    /*******************************************************************//*!
     * @param delay - the pulser delay must be > 4
     ************************************************************************/
    int set_pulser_delay(int delay);
     /*******************************************************************//*!
     * Store handshake in the class and send it the trigger box.
     * Handshake delay is vt1 of the handshake unit. this is the veto hold 
     * time beond the end of a busy pulse.  
     * the time of the deay is in units of 2.5 nS clock pulses
     * @return 0 on sucess 1 on error
     * @see get_handshake_delay()
     * @see send_handshake_delay()
     ************************************************************************/
    int set_handshake_delay(int delay);

    /*******************************************************************//*!
     * Send the two phase settings packed in one int
     * @param phases two ________ bit phase settings for the 40MHz clk gen packed in an int
     * @return 0 on sucess 1 on error
     ************************************************************************/
    int set_clk40_phases(int phases);
    /*******************************************************************//*!
     * @return the stored handshake delay 
     * @see set_handshake_delay(int mask)
     ************************************************************************/
    int get_handshake_delay();
    /*******************************************************************//*!
     * Send the stored handshake delay to the trigger box.
     * @return 0 on sucess 1 on error
     * @see set_handshake_delay(int delay)
     ************************************************************************/
    int send_handshake_delay();
    /*******************************************************************//*!
     * Set the handshake mask and send it to the trigger box.
     * @param mask - the lower 4 bit corispond to M1 - M4 of the handshake unit 
     * @return 0 on sucess 1 on error
     * @see send_handshake_mask()
     * @see get_handshake_mask()
     ************************************************************************/
   int set_handshake_mask(int mask);
    /*******************************************************************//*!
     * @return the handshake mask stored localy
     * @see send_handshake_mask()
     * @see get_handshake_mask()
     ************************************************************************/
    int get_handshake_mask();
    /*******************************************************************//*!
     * send the localy stored handshake mask
     * @return 0 on sucess 1 on error
     * @see send_handshake_mask()
     * @see get_handshake_mask()
     ************************************************************************/
    int send_handshake_mask();
    /*******************************************************************//*!
     * Sset the current time in the trigger box.  This sends the current 
     * unix  time stamp * 1000 to the triger box. 
     * Time on the trigger box is a mS counter stored as a 64 bit unsigned intiger 
     * @return 0 on sucess 1 on error
     * @see 
     ************************************************************************/
    int set_time();
    /**********************************************************************//*!
     * loads all saved setting to the trigger box form a settings file.
     * @param fname - pointer to the config file name to load to the fpga in the 
     *         trigger box
     * @return 0 on sucess
     *************************************************************************/
    int load_from_file(char *fname);
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

private:
    int http_backend(char * command);
    int scintillator_delay;
    int plane_1_delay;
    int plane_2_delay;
    int plane_3_delay;
    int plane_4_delay;
    int plane_5_delay;
    int plane_6_delay;
    int plane_7_delay;
    int plane_8_delay;
    int pad_delay;
    http_responce_pars * parser;
    char error_str[255];
    int coincidence_pulse_width;
    int coincidence_edge_width;
    int handshake_mask;
    int handshake_delay;
    std::string ip_adr;
};


#endif // TRIGGER_CONTROLL_H
