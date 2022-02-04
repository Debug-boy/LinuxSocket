#include <malloc.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <thread>
#include <iostream>
#include <dirent.h>
#include <sys/file.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/uio.h>

namespace infinity::touch{

    struct TouchDev{
        int fingerNum = 9;
        int fd{};
    }touchDev;

    int getEventCount(){
        DIR *dir = opendir("/dev/input/");
        dirent *ptr;
        signed int count = 0;
        if(dir){
            while ((ptr = readdir(dir))){
                if (strstr(ptr->d_name, "event")){
                    ++count;
                }
            }
        }
        return count;
    }

    int getTouchEventNum(){
        int eventCount = getEventCount();
        int *fdArray = (int *)malloc(eventCount * 4 + 4);

        for (int i = 0; i < eventCount; i++){
            char temp[128];
            sprintf(temp, "/dev/input/event%d", i);
            fdArray[i] = open(temp, O_RDWR | O_NONBLOCK);
        }

        input_event ev{};
        while (true){
            for (int i = 0; i < eventCount; i++){
                memset(&ev, 0, sizeof(ev));
                read(fdArray[i], &ev, sizeof(ev));
                if (ev.type == EV_ABS){
                    free(fdArray);
                    return i;
                }
            }
            usleep(100);
        }
    }

    void sendEventData(void *v, unsigned int count){
        write(touchDev.fd, v, sizeof(struct input_event) * count);
    }

    void sendNullData(){
        static constexpr unsigned int number = 2;
        static struct input_event event[number];
        memset(event, 0, sizeof(event));

        event[0].type = EV_SYN;
        event[0].code = SYN_REPORT;
        event[0].value = 0;

        event[1].type = EV_ABS;
        event[1].code = ABS_MT_SLOT;
        event[1].value = touchDev.fingerNum;

        sendEventData(event, number);
    }

    bool touch_init(int *retX, int *retY){
        char tmp[256];
        sprintf(tmp, "/dev/input/event%d", getTouchEventNum());
        touchDev.fd = open(tmp, O_RDWR);
        if (touchDev.fd <= 0)
            return false;
        struct input_absinfo absX{}, absY{};
        ioctl(touchDev.fd, EVIOCGABS(ABS_MT_POSITION_X), &absX);
        ioctl(touchDev.fd, EVIOCGABS(ABS_MT_POSITION_Y), &absY);
        *retX = absX.maximum;
        *retY = absY.maximum;
        return true;
    }

    void touch_down(int id, int x, int y){
        static struct input_event event[9];
        memset(event, 0, sizeof(event));
        sendNullData();

        int tmpCnt = 0;
        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_SLOT;
        event[tmpCnt].value = touchDev.fingerNum;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TRACKING_ID;
        event[tmpCnt].value = 123;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TRACKING_ID;
        event[tmpCnt].value = 123;
        tmpCnt++;

        event[tmpCnt].type = EV_KEY;
        event[tmpCnt].code = BTN_TOUCH;
        event[tmpCnt].value = KEY_DOWN;
        tmpCnt++;

        event[tmpCnt].type = EV_KEY;
        event[tmpCnt].code = BTN_TOOL_FINGER;
        event[tmpCnt].value = KEY_DOWN;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_POSITION_X;
        event[tmpCnt].value = x;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_POSITION_Y;
        event[tmpCnt].value = y;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TOUCH_MAJOR;
        event[tmpCnt].value = 4;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        sendEventData(event, tmpCnt);
    }

    void touch_move(int id, int x, int y){
        static struct input_event event[8];
        memset(event, 0, sizeof(event));
        sendNullData();

        int tmpCnt = 0;
        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_SLOT;
        event[tmpCnt].value = touchDev.fingerNum;
        tmpCnt++;

        event[tmpCnt].type = EV_KEY;
        event[tmpCnt].code = BTN_TOUCH;
        event[tmpCnt].value = KEY_DOWN;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_POSITION_X;
        event[tmpCnt].value = x;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_POSITION_Y;
        event[tmpCnt].value = y;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TOUCH_MAJOR;
        event[tmpCnt].value = 4;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_WIDTH_MAJOR;
        event[tmpCnt].value = 10;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        sendEventData(event, tmpCnt);
    }

    void touch_alloc(){
        static constexpr unsigned int number = 5;
        static struct input_event event[number];
        memset(event, 0, sizeof(event));
        sendNullData();

        event[0].type = EV_SYN;
        event[0].code = SYN_REPORT;
        event[0].value = 0;

        event[1].type = EV_ABS;
        event[1].code = ABS_MT_SLOT;
        event[1].value = touchDev.fingerNum;

        event[2].type = EV_ABS;
        event[2].code = ABS_MT_TRACKING_ID;
        event[2].value = 123;

        event[3].type = EV_SYN;
        event[3].code = SYN_REPORT;
        event[3].value = 0;

        event[4].type = EV_SYN;
        event[4].code = SYN_REPORT;
        event[4].value = 0;

        sendEventData(event, number);
    }

    void touch_up(int id){
        touch_alloc();
        static struct input_event event[7];
        memset(event, 0, sizeof(event));
        sendNullData();

        int tmpCnt = 0;
        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_SLOT;
        event[tmpCnt].value = touchDev.fingerNum;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TRACKING_ID;
        event[tmpCnt].value = -1;
        tmpCnt++;

        event[tmpCnt].type = EV_ABS;
        event[tmpCnt].code = ABS_MT_TRACKING_ID;
        event[tmpCnt].value = -1;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        event[tmpCnt].type = EV_SYN;
        event[tmpCnt].code = SYN_REPORT;
        event[tmpCnt].value = 0;
        tmpCnt++;

        sendEventData(event, tmpCnt);
    }
}