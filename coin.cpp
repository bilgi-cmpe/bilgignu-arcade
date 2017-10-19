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

// gpio pin numbers
#define INSERT 7
#define UP 28
#define DOWN 25
#define LEFT 27
#define RIGHT 24
#define START 23

// initialize curl constructor/destructor
static curlpp::Cleanup clean;

static int up = 0;
static int down = 0;
static int left = 0;
static int right = 0;
static int start = 0;
static int insert = 0;

// this is called when an interrupt happens (button press etc.)
static void left_()
{
    left++;
}
static void right_()
{
    right++;
}

static void up_()
{
    up++;
}

static void down_()
{
    down++;
}

static void start_()
{
    start++;
}
static void insert_()
{
    insert++;
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

    if (ioctl(fd, UI_SET_KEYBIT, KEY_UP) < 0)
        ioctl_problem_exit();

    if (ioctl(fd, UI_SET_KEYBIT, KEY_DOWN) < 0)
        ioctl_problem_exit();

    if (ioctl(fd, UI_SET_KEYBIT, KEY_LEFT) < 0)
        ioctl_problem_exit();

    if (ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT) < 0)
        ioctl_problem_exit();

    if (ioctl(fd, UI_SET_KEYBIT, KEY_A) < 0)
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

    // initialize wiringPi, abort if it can't be initialized
    if (wiringPiSetup() < 0)
        return 1;

    // initialize interrupt handler
    if (wiringPiISR(INSERT, INT_EDGE_FALLING, &insert_) < 0)
    {
        printf("interrupt setup fail\n");
    }

    if (wiringPiISR(UP, INT_EDGE_FALLING, &up_) < 0)
    {
        printf("interrupt setup fail\n");
    }
    if (wiringPiISR(DOWN, INT_EDGE_FALLING, &down_) < 0)
    {
        printf("interrupt setup fail\n");
    }
    if (wiringPiISR(LEFT, INT_EDGE_FALLING, &left_) < 0)
    {
        printf("interrupt setup fail\n");
    }
    if (wiringPiISR(RIGHT, INT_EDGE_FALLING, &right_) < 0)
    {
        printf("interrupt setup fail\n");
    }

    if (wiringPiISR(START, INT_EDGE_FALLING, &start_) < 0)
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
        // execute code on interrupt
        if (insert)
        {
            // do nothing until the INSERT voltage is back to normal (wait for button
            // to be released etc.
            while (digitalRead(INSERT) == 1)
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
            insert = 0;
            // destroy uinput device & close handle to uinput
        }
        else if (up)
        {
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_UP;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_UP;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            up = 0;
        }
        else if (down)
        {
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_DOWN;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_DOWN;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            down = 0;
        }
        else if (left)
        {
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_LEFT;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_LEFT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            left = 0;
        }
        else if (right)
        {
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_RIGHT;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_RIGHT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            right = 0;
        }
        else if (start)
        {
            while (digitalRead(START) == 1)
                ;
            // keypress events
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_A;
            ev.value = 1;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = KEY_A;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("write 2 error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            if (write(fd, &ev, sizeof(ev)) < 0)
            {
                printf("synchronize error");
                exit(EXIT_FAILURE);
            }
            usleep(10000);
            start = 0;
        }
    }
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    closed = 1;
}
