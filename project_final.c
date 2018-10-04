/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Testing the broadcast layer in Rime
 * \author
 *         Ashraf Shaikh Mohammed <ashraf@iastate.edu>
 */

#include <stdio.h>
#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "cc2420.h"
#include "dev/leds.h"
#include "dev/light-sensor.h"
#include "dev/button-sensor.h"
#include "dev/sht11/sht11-sensor.h"

uint16_t temp_values[4] = {0,0,0,0} ; // An array to store temperature values
uint16_t temp_temp, avg_temp, n, sum; // Variables for working with the temperature variable 
int motes_addr_0[3] = {0,0,0}; // Arrays to store the motes addresses
int motes_addr_1[3] = {0,0,0};
int addr_0, addr_1, j, c, k, l, m, s, t, f, v, lock; // some variables to help with our algorithm
int i = 0;
int check = 0; // to count the number of times we send
int times_recvd[3] = {0,0,0}; // array for keeping track of how many times we receive from corresponding motes

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/

struct test_packet { 
  uint16_t temp;
}; // create a structure to send packet with temperature values

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{

  temp_temp = ((struct test_packet *)packetbuf_dataptr())->temp; // store the obtained temperature value in a variable
  addr_0 = from->u8[0]; // store the address of mote from which data is received
  addr_1 = from->u8[1];

  printf("broadcast message received from %d.%d: '%u'\n",
         addr_0, addr_1, temp_temp);

	 if(i>0)     // to keep track of addresses of motes. i tells us how many motes we receiving from.
	 {
		
	  j=0;c=0;

	  while(j < i)  // we check if the received address is already in the array by looping through the array
	  {	

		
	   if((addr_0 != motes_addr_0[j]) && (addr_1 != motes_addr_1[j])) // to see in how many locations of the mote address array the value wasn't found.
		{				
		 c++;	
		}
	   j++;
		
	  }

	  if(c == i*12)  /* if the received address is not found in any location in motes address array, then store it in the new location. (c value was found to increase by multiple of 12) */
	  {
	  	 motes_addr_0[i] = addr_0;
		 motes_addr_1[i] = addr_1;
		 i++;
	  }

	 }
	 else  /* The received address of the first mote is stored in the first position in mote address array */
	 {
	  	 motes_addr_0[i] = addr_0;
		 motes_addr_1[i] = addr_1;
		 i++;
	 }

	printf(" i in recv is: %d \n", i); // for testing and debugging purposes

	for(m = 0; m < 3; m++) /* displaying the addresses of motes stored in array and also for testing and debugging purposes */
	 printf(" the %d addr is %d.%d \n", m+1, motes_addr_0[m], motes_addr_1[m] );

  
	for(k=0; k<i; k++)  // filling up the temperature values in corresponding locations of motes address
	{
	 if((addr_0 == motes_addr_0[k]) && (addr_1 == motes_addr_1[k]))
		{
		  temp_values[k+1] = temp_temp;
		  times_recvd[k]++;  // to keep track of how many times you receive from that mote
		}
	 printf("temp vals new:- k(%d): %u, k+1(%d): %u \n",k,temp_values[k],k+1,temp_values[k+1]); /* Displaying the temperature values in the temp values array and also for testing and debugging purposes */
	 printf("received %d times from address: %d.%d \n", times_recvd[k], motes_addr_0[k], motes_addr_1[k]);
	}
   
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  struct test_packet pkt; // create packet variable to send temp values.

  SENSORS_ACTIVATE(sht11_sensor); // activate temp sensor
  SENSORS_ACTIVATE(button_sensor); // activate button sensor

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  leds_off(LEDS_RED);  // Ensuring that all leds are off to begin with
  leds_off(LEDS_GREEN);
  leds_off(LEDS_BLUE);

  broadcast_open(&broadcast, 129, &broadcast_call);

  cc2420_set_txpower(31); // changed Tx pwr level to max

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 2); 

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et) || data == &button_sensor);

    pkt.temp = sht11_sensor.value(SHT11_SENSOR_TEMP); // storing the temp value as received from the sensor in our packet

    temp_values[0] = pkt.temp; /* store its own temperature value as the first element of temperature array */
	 
    sum = 0;     

    printf(" i in main is: %d \n", i); // for testing and debugging purposes

    for(v = 0; v<i; v++)  // looping through the times recvd array
    {
	if(check > (times_recvd[v]+40))  /* testing for mote failure i.e sending far more than receiving implies you're not receiving from that mote. 40 is a really large value. we can change it to a convenient value like 10, 15, 20 or whatever you wish  */
	{   
	    f = v;
	    while(f < i-1)   // shifting the next elements in array to previous position.
		{
		motes_addr_0[f] = motes_addr_0[f+1];
		motes_addr_1[f]	= motes_addr_1[f+1];
		temp_values[f+1] = temp_values[f+2];
		times_recvd[f] = times_recvd[f+1];
		f++;
		}
	    motes_addr_0[f] = 0;
	    motes_addr_1[f] = 0;
	    temp_values[f+1] = 0;
	    times_recvd[f] = 0;
	    v--;
	    i--;
	}
    }

    for(l=0; l<i+1; l++)  // calculating avg temperature values from temp values in temp array
	sum += temp_values[l];
	
    avg_temp = sum/(i+1);

    avg_temp = (0.01*avg_temp - 39.6)*1.8 + 32;  /* change the avg temp value to fahrenheit */

    if(data == &button_sensor) //Incorporate the LED glowing functionality
	lock=1;

    if(lock == 1) //Incorporate the LED glowing functionality
    {
      printf(" led mode on");  // for testing and debugging purposes
      n = avg_temp;
      t = 0;  // t is for keeping track of which digit place we currently in; 0 means 1's place digit, 1 means 10's place digit and so on.
      while(n > 0) // for looping through each digit in the number
      {
	    for(s = 1; s <= 2*(n%10); s++) /* Getting the different leds to glow as many times as the corresponding digits in the temp value in Fahrenheit */
	    {
		if((s%2)==1)  // for blinking effect, to toggle between on and off
		{ 
		  if(t==0)
		    leds_on(LEDS_RED); // red glows corresponding to ones place digit in the temp value
		  else if(t==1)
		    leds_on(LEDS_GREEN); // green glows corresponding to tens place digit in the temp value
		  else
		    leds_on(LEDS_BLUE); // blue glows corresponding to hundreds place digit in the temp value
		}
		else 
		{   
		  if(t==0)
		    leds_off(LEDS_RED);
		  else if(t==1)
		    leds_off(LEDS_GREEN);
		  else
		    leds_off(LEDS_BLUE);
		}

		    etimer_set(&et, CLOCK_SECOND * 1); 
		    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	    }
	    n=n/10; // decreasing the number by taking out the last digit from right
	    t++;
      }
      lock = 0;  // to ensure leds functionality can be used next time only when button is pressed
    }

    packetbuf_copyfrom(&pkt, sizeof(struct test_packet)); /* Copy the data from temp variable to send in buffer. */

    broadcast_send(&broadcast); 

    check++; // counts the number of times you send the data

    printf("times sent is: %d \n", check);

    printf("broadcast message sent: %u \n", pkt.temp);

    printf("avg temp value is: %u \n", avg_temp);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
