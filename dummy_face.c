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
#include "dummy_face.h"
#include "watch.h"
#include "watch_private_display.h"
int lowTideMinutes = 722; //Changed to a global variable
int lowTideHourGap = 0;
int resetTide = 0;

void dummy_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(dummy_state_t));
        memset(*context_ptr, 0, sizeof(dummy_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void dummy_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    dummy_state_t *state = (dummy_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
}

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

bool dummy_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    dummy_state_t *state = (dummy_state_t *)context;
    watch_date_time date_time;
    int nowSeconds;
    int modSeconds;
    //int lowTideMinutes;
    //if(lowTideMinutes < 0 || lowTideMinutes > 722){
    //    lowTideMinutes = 722;
    //}
    //int lowTideMinutes = 722;
    //int lowTideHourGap = 0;
    int NumTidesRemainder;
//    char buf[7];
    char tideString[7];
    char modHour[3];
    char eightString[9];
    //uint32_t realLowTideTime;
    //uint32_t currentUNIXTime;
    //uint32_t differenceUNIXTime;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // Show your initial UI here.
             //lowTideMinutes = 722;
             //sprintf(eightString, "%02d%s", 0, "llllll");
             //watch_display_string(eightString, 2);
            break;
        case EVENT_TICK:
            // If needed, update your display here.
            date_time = watch_rtc_get_date_time();
            nowSeconds = date_time.unit.second;
            modSeconds = nowSeconds % 12;   //Actual mod (%) will be based on 12 hours and 2 minutes = 722 minutes
                                            // Maybe base on elapsed UNIX time
            // UNIX Time Test
            // uint32_t watch_utility_convert_to_unix_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute,
            // uint8_t second, uint32_t utc_offset)
            //
            //realLowTideTime = __watch_utility_convert_to_unix_time(2024, 8, 3, 27, 0, 0, -4); // Find the UNIX time of an actual previous low tide
                                                                                            // UNIX = 1722540420
            //currentUNIXTime = __watch_utility_convert_to_unix_time(date_time.unit.year, date_time.unit.month, date_time.unit.day, date_time.unit.hour, date_time.unit.minute, 0, -4);
                                                                                            // UNIX = 1722822780 8/4/2024 9:53 PM
            //differenceUNIXTime = currentUNIXTime - realLowTideTime; // UNIX Time is number of seconds since 1/1/1970, so this is seconds elapsed
                                                                    // 1722822780 - 1722540420 = 282360
            // Low tide every 722 minutes = 43320 seconds. Can we MOD 43320?
            
            if(resetTide == 1){
                lowTideMinutes = 722;
                resetTide = 0;
            }

            if(nowSeconds==0){ 				// Just for testing. Change tide once a minute.
                lowTideMinutes = lowTideMinutes - 60;
                lowTideHourGap = (lowTideMinutes / 60);
            }

            //NumTidesRemainder = (differenceUNIXTime % 43320)/60/60; // Find the remainder after dividing by tide cycle seconds
                                                            // 43320 is seconds so / 60 /60 to get hours
                                                            // 282360 % 43320 is  quotient:6 remainder:22440
                                                            // 22440/60/60 = 6.23. It is actually 8 hours and 40 minutes to the next low tide
                                                            // There actually HAVE been 6 low tides since then. The remainder is what we want
                                                            // 0.23 * 43320 = 9963.6 seconds 9963.6/60/60 = 2.76. That is not 8 hours.
                                                            // 1.00-0.23=0.77. 0.77*43320 = 33356.4 seconds. 33356.4/60/60=9.26. Also not 8 hourds

            switch (lowTideHourGap) {
                case 0:
                    strcpy(tideString, "llllll");
                    break;
                case 1:
                    strcpy(tideString, "-----)");
                    break;
                case 2:
                    strcpy(tideString, "----))");
                    break;
                case 3:
                    strcpy(tideString, "---)))");
                    break;
                case 4:
                    strcpy(tideString, "--))))");
                    break;
                case 5:
                    strcpy(tideString, "-)))))");
                    break;
                case 6:
                    strcpy(tideString, "hhhhhh");
                    break;
                case 7:
                    strcpy(tideString, "-(((((");
                    break;
                case 8:
                    strcpy(tideString, "--((((");
                    break;
                case 9:
                    strcpy(tideString, "---(((");
                    break;
                case 10:
                    strcpy(tideString, "----((");
                    break;
                case 11:
                    strcpy(tideString, "-----(");
                    break;
                case 12:
                    strcpy(tideString, "llllll");
                    break;
                default:
                    strcpy(tideString, "??????");
            }
            sprintf(eightString, "%02d%s", lowTideHourGap, tideString);
            // 4th position is the first position of the large characters
            watch_display_string(eightString, 2);
            break;
         case EVENT_ALARM_LONG_PRESS: // Maybe use a long press to reset to 722 minutes
             lowTideMinutes = 722;
             lowTideHourGap = (lowTideMinutes / 60);
             sprintf(eightString, "%02d%s", lowTideHourGap, "llllll");
             watch_display_string(eightString, 2);
             resetTide = 1;
             break;
        case EVENT_LIGHT_BUTTON_UP:
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            break;
        case EVENT_ALARM_BUTTON_UP:
            // Just in case you have need for another button.
            // Maybe use this button to adjust the current number of hours from low tide
            // lowTideMinutes = lowTideMinutes + 60;
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

void dummy_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}
