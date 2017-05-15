#include <stdio.h>
#include <wiringPi.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>

/*
A simple program to update total amount of coins inserted (whenever a coin
insertion occurs) by sending a request to the API using curl library.

Credit: Bilgi Open Source Community - Deniz G�lnar
Charity Arcade Project
*/

// gpio pin number
#define PIN 7

// initialize curl constructor/destructor
static curlpp::Cleanup clean;

static int press = 0;

// this is called when an interrupt happens (button press etc.)
static void perform_() {;
	press++;
}


int main() {
// initialize wiringPi, abort if it can't be initialized
    if(wiringPiSetup() < 0)return 1;
// initialize interrupt handler
    if(wiringPiISR(PIN, INT_EDGE_FALLING, &perform_) < 0)
    {
        printf("interrupt setup fail\n");
    }
// if everything is working properly, create a curl request and set it up
	printf("Ready\n");
 		static curlpp::Easy request;
                request.setOpt(curlpp::Options::Verbose(true));
                request.setOpt(curlpp::Options::Url("TBD"));

// main loop
    while(1)
    {
// execute code on interrupt
        if(press)
        {
// do nothing until the pin voltage is back to normal (wait for button
// to be released etc.)
            while(digitalRead(PIN)==0);
// perform curl request
		request.perform();
// flush output buffer
		printf("\n");
// reset interrupt state back to 0
            press = 0;
        }
    }
}
