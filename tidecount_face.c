/*
 * MIT License
 *
 * Copyright (c) 2024 <#author_name#>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "tidecount_face.h"
#include "watch.h"
#include "watch_utility.h"
#include "watch_private_display.h"
//char sixBuf[6 + 1];
//char eightBuf[8 + 1];
char tenBuf[10 + 1];
int deltaMinutes = 722;
int currentUNIXTime;
int lastUNIXTime;
int hoursTil;
int minutesTil;
char tideDirection[2 + 1];


// Function taken from `src/time/__year_to_secs.c` of musl libc
// https://musl.libc.org
static uint32_t __year_to_secs(uint32_t year, int *is_leap)
{
	if (year-2ULL <= 136) {
		int y = year;
		int leaps = (y-68)>>2;
		if (!((y-68)&3)) {
			leaps--;
			if (is_leap) *is_leap = 1;
		} else if (is_leap) *is_leap = 0;
		return 31536000*(y-70) + 86400*leaps;
	}

	int cycles, centuries, leaps, rem;

	if (!is_leap) is_leap = &(int){0};
	cycles = (year-100) / 400;
	rem = (year-100) % 400;
	if (rem < 0) {
		cycles--;
		rem += 400;
	}
	if (!rem) {
		*is_leap = 1;
		centuries = 0;
		leaps = 0;
	} else {
		if (rem >= 200) {
			if (rem >= 300) centuries = 3, rem -= 300;
			else centuries = 2, rem -= 200;
		} else {
			if (rem >= 100) centuries = 1, rem -= 100;
			else centuries = 0;
		}
		if (!rem) {
			*is_leap = 0;
			leaps = 0;
		} else {
			leaps = rem / 4U;
			rem %= 4U;
			*is_leap = !rem;
		}
	}

	leaps += 97*cycles + 24*centuries - *is_leap;

	return (year-100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}

// Function taken from `src/time/__month_to_secs.c` of musl libc
// https://musl.libc.org
static int __month_to_secs(int month, int is_leap)
{
	static const int secs_through_month[] = {
		0, 31*86400, 59*86400, 90*86400,
		120*86400, 151*86400, 181*86400, 212*86400,
		243*86400, 273*86400, 304*86400, 334*86400 };
	int t = secs_through_month[month];
	if (is_leap && month >= 2) t+=86400;
	return t;
}

// Function adapted from `src/time/__tm_to_secs.c` of musl libc
// https://musl.libc.org
uint32_t __watch_utility_convert_to_unix_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t utc_offset) {
    int is_leap;

    // POSIX tm struct starts year at 1900 and month at 0
    // https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/time.h.html
    uint32_t timestamp = __year_to_secs(year - 1900, &is_leap);
    timestamp += __month_to_secs(month - 1, is_leap);

    // Regular conversion from musl libc
    timestamp += (day - 1) * 86400;
    timestamp += hour * 3600;
    timestamp += minute * 60;
    timestamp += second;
    timestamp -= utc_offset;

    return timestamp;
}


void tidecount_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(tidecount_state_t));
        memset(*context_ptr, 0, sizeof(tidecount_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void tidecount_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    tidecount_state_t *state = (tidecount_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
}

bool tidecount_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    tidecount_state_t *state = (tidecount_state_t *)context;
    watch_date_time date_time;
//    int currentUNIXTime;
//    int lastUNIXTime;
//      UNIX Time is number of seconds since 1970
//
    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // Show your initial UI here.
            //char buf[6 + 1];
            watch_set_colon();
            date_time = watch_rtc_get_date_time();
            currentUNIXTime = __watch_utility_convert_to_unix_time(date_time.unit.year, date_time.unit.month, date_time.unit.day, date_time.unit.hour, date_time.unit.minute, 0, -4);
            lastUNIXTime = currentUNIXTime;
            //snprintf(
            //    buf,
            //    sizeof(buf),
            //    "2d%02d%02d",
            //    date_time.unit.hour,
            //    date_time.unit.minute,
            //    date_time.unit.second
            //);
            //sprintf(buf, "))))))");
//            sprintf(tenBuf, "))  %02d%02d%02d", date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
//            sprintf(tenBuf, "))    %04d",deltaMinutes);
            hoursTil = deltaMinutes / 60;
            minutesTil = deltaMinutes % 60;
            if(hoursTil >= 6){
                sprintf(tideDirection,"hi");
            }
            else{
                sprintf(tideDirection,"lo");
            }
            sprintf(
                tenBuf,
                "%s  %2d%02d  ",
                tideDirection,
                hoursTil,
                minutesTil
             );

//            sprintf(tenBuf, "%s  %02d%02d",tideDirection,hoursTil,minutesTil);

            watch_display_string(tenBuf, 0);


            break;
        case EVENT_TICK: // this updates all the time
            // If needed, update your display here.
            //char buf[6 + 1];

            date_time = watch_rtc_get_date_time();
            currentUNIXTime = __watch_utility_convert_to_unix_time(date_time.unit.year, date_time.unit.month, date_time.unit.day, date_time.unit.hour, date_time.unit.minute, 0, -4);
            deltaMinutes = deltaMinutes - ((currentUNIXTime - lastUNIXTime)/60);
            if (deltaMinutes <= 0){
                deltaMinutes = 722;
            }
            lastUNIXTime = currentUNIXTime;

            //snprintf(
            //    buf,
            //    sizeof(buf),
            //    "2d%02d%02d",
            //    date_time.unit.hour,
            //    date_time.unit.minute,
            //    date_time.unit.second
            //);
            //sprintf(buf, "))))))");
//            sprintf(tenBuf, "))  %02d%02d%02d", date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
//            sprintf(tenBuf, "))    %04d",deltaMinutes);

            hoursTil = deltaMinutes / 60;
            minutesTil = deltaMinutes % 60;
            if(hoursTil >= 6){
                sprintf(tideDirection,"hi");
            }
            else{
                sprintf(tideDirection,"lo");
            }
            sprintf(
                tenBuf,
                "%s  %2d%02d  ",
                tideDirection,
                hoursTil,
                minutesTil
             );

//            sprintf(tenBuf, "%s  %02d%02d",tideDirection,hoursTil,minutesTil);
            watch_display_string(tenBuf, 0);

            break;
        case EVENT_LIGHT_BUTTON_UP:
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Just in case you have need for another button.
            deltaMinutes = deltaMinutes - 15;
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            // movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void tidecount_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}
