#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <wiringPi.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <linux/input.h>
#include <linux/uinput.h>
#include <fstream>
#include <iostream>

/*
A simple program to update total amount of coins inserted (whenever a coin
insertion occurs) by sending a request to the API using curl library.

Credit: Bilgi Open Source Community - Deniz Gulnar
Charity Arcade Project
*/

// gpio pin number
#define PIN 7

// initialize curl constructor/destructor
static curlpp::Cleanup clean;

static int press = 0;

// this is called when an interrupt happens (button press etc.)
static void perform_()
{
    ;
    press++;
}

void ioctl_problem_exit()
{
    printf("ioctl error");
    exit(EXIT_FAILURE);
}

int main()
{

    int closed = 1;
    int total_coin = 0;
    std::string rd = "";
    std::ifstream r_file;
    std::ofstream w_file;

    int fd;
    struct uinput_user_dev uidev;
    struct input_event ev;

    // initialize wiringPi, abort if it can't be initialized
    if (wiringPiSetup() < 0)
        return 1;
    // initialize interrupt handler
    if (wiringPiISR(PIN, INT_EDGE_FALLING, &perform_) < 0)
    {
        printf("interrupt setup fail\n");
    }
    // if everything is working properly, create a curl request and set it up
    printf("Ready\n");
    std::list<std::string> header;
    header.push_back("Authorization: Token bae7787eb9292caee617a72956f6ce9c253df773");
    static curlpp::Easy request;
    static curlpp::Forms form;
    request.setOpt(curlpp::Options::HttpHeader(header));
    request.setOpt(curlpp::Options::CustomRequest("PATCH"));
    form.push_back(new curlpp::FormParts::Content("money", std::to_string(1)));
    request.setOpt(curlpp::Options::HttpPost(form));
    request.setOpt(curlpp::Options::Verbose(false));
    request.setOpt(curlpp::Options::Url("ozguryazilim.bilgi.edu.tr/api/arcade/2/"));
    // main loop
    while (true)
    {
        usleep(10000);
        // create uinput device
        if (closed)
        {
            closed = 0;
            fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
            if (fd < 0)
            {
                printf("uniput init error");
                exit(EXIT_FAILURE);
            }

            if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
                ioctl_problem_exit();

            if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0)
                ioctl_problem_exit();

            if (ioctl(fd, UI_SET_KEYBIT, KEY_D) < 0)
                ioctl_problem_exit();

            memset(&uidev, 0, sizeof(uidev));
            snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "coinmachine");
            uidev.id.bustype = BUS_USB;
            uidev.id.vendor = 0x1;
            uidev.id.product = 0x1;
            uidev.id.version = 4;

            if (write(fd, &uidev, sizeof(uidev)) < 0)
            {
                printf("write error");
                exit(EXIT_FAILURE);
            }
            if (ioctl(fd, UI_DEV_CREATE) < 0)
                ioctl_problem_exit();
        }
        // execute code on interrupt
        if (press)
        {
            // do nothing until the pin voltage is back to normal (wait for button
            // to be released etc.
            while (digitalRead(PIN) == 1)
                ;
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_D;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(50000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(50000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_D;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(50000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(50000);
            // perform curl request
            try
            {
                request.perform();
                std::cout << std::flush;
            }
            // or keep data in file if request failed
            catch (std::exception e)
            {
                r_file.open("cointotal.txt");
                r_file >> rd;
                r_file.close();
                total_coin = std::stoi(rd);
                total_coin += 1;
                w_file.open("cointotal.txt");
                w_file << total_coin;
                w_file.close();
            }
            // reset interrupt state back to 0
            press = 0;
            // destroy uinput device & close handle to uinput
            ioctl(fd, UI_DEV_DESTROY);
            close(fd);
            closed = 1;
        }
    }
}
