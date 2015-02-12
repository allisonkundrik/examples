/* 
 * File:   main.cpp
 * Author: peter cf16 eu
 *
 * Created on May 18, 2014, 1:32 PM
 * 
 * Read the CPU temperature on AMD FX 4100 Quad Core
 * based on lm-sensors code
 * http://lm-sensors.org/browser/lm-sensors/trunk/lib/sysfs.c
 * and answer of Blue Moon
 * http://stackoverflow.com/a/23716850/1141471
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE_COUNT 20         /* number of measurements */

#define SENSORS_ERR_WILDCARDS	1 /* Wildcard found in chip name */
#define SENSORS_ERR_NO_ENTRY	2 /* No such subfeature known */
#define SENSORS_ERR_ACCESS_R	3 /* Can't read */
#define SENSORS_ERR_KERNEL	4 /* Kernel interface error */
#define SENSORS_ERR_DIV_ZERO	5 /* Divide by zero */
#define SENSORS_ERR_CHIP_NAME	6 /* Can't parse chip name */
#define SENSORS_ERR_BUS_NAME	7 /* Can't parse bus name */
#define SENSORS_ERR_PARSE	8 /* General parse error */
#define SENSORS_ERR_ACCESS_W	9 /* Can't write */
#define SENSORS_ERR_IO		10 /* I/O error */
#define SENSORS_ERR_RECURSION	11 /* Evaluation recurses too deep */

typedef enum sensors_feature_type {
	SENSORS_FEATURE_IN		= 0x00,
	SENSORS_FEATURE_FAN		= 0x01,
	SENSORS_FEATURE_TEMP		= 0x02,
	SENSORS_FEATURE_POWER		= 0x03,
	SENSORS_FEATURE_ENERGY		= 0x04,
	SENSORS_FEATURE_CURR		= 0x05,
	SENSORS_FEATURE_HUMIDITY	= 0x06,
	SENSORS_FEATURE_MAX_MAIN,
	SENSORS_FEATURE_VID		= 0x10,
	SENSORS_FEATURE_INTRUSION	= 0x11,
	SENSORS_FEATURE_MAX_OTHER,
	SENSORS_FEATURE_BEEP_ENABLE	= 0x18,
	SENSORS_FEATURE_MAX,
	SENSORS_FEATURE_UNKNOWN		= INT_MAX,
} sensors_feature_type;

/* All the sensor types (in, fan, temp, vid) are a multiple of 0x100 apart,
   and sensor subfeatures which have no compute mapping have bit 7 set. */
typedef enum sensors_subfeature_type {
	SENSORS_SUBFEATURE_IN_INPUT = SENSORS_FEATURE_IN << 8,
	SENSORS_SUBFEATURE_IN_MIN,
	SENSORS_SUBFEATURE_IN_MAX,
	SENSORS_SUBFEATURE_IN_LCRIT,
	SENSORS_SUBFEATURE_IN_CRIT,
	SENSORS_SUBFEATURE_IN_AVERAGE,
	SENSORS_SUBFEATURE_IN_LOWEST,
	SENSORS_SUBFEATURE_IN_HIGHEST,
	SENSORS_SUBFEATURE_IN_ALARM = (SENSORS_FEATURE_IN << 8) | 0x80,
	SENSORS_SUBFEATURE_IN_MIN_ALARM,
	SENSORS_SUBFEATURE_IN_MAX_ALARM,
	SENSORS_SUBFEATURE_IN_BEEP,
	SENSORS_SUBFEATURE_IN_LCRIT_ALARM,
	SENSORS_SUBFEATURE_IN_CRIT_ALARM,

	SENSORS_SUBFEATURE_FAN_INPUT = SENSORS_FEATURE_FAN << 8,
	SENSORS_SUBFEATURE_FAN_MIN,
	SENSORS_SUBFEATURE_FAN_MAX,
	SENSORS_SUBFEATURE_FAN_ALARM = (SENSORS_FEATURE_FAN << 8) | 0x80,
	SENSORS_SUBFEATURE_FAN_FAULT,
	SENSORS_SUBFEATURE_FAN_DIV,
	SENSORS_SUBFEATURE_FAN_BEEP,
	SENSORS_SUBFEATURE_FAN_PULSES,
	SENSORS_SUBFEATURE_FAN_MIN_ALARM,
	SENSORS_SUBFEATURE_FAN_MAX_ALARM,

	SENSORS_SUBFEATURE_TEMP_INPUT = SENSORS_FEATURE_TEMP << 8,
	SENSORS_SUBFEATURE_TEMP_MAX,
	SENSORS_SUBFEATURE_TEMP_MAX_HYST,
	SENSORS_SUBFEATURE_TEMP_MIN,
	SENSORS_SUBFEATURE_TEMP_CRIT,
	SENSORS_SUBFEATURE_TEMP_CRIT_HYST,
	SENSORS_SUBFEATURE_TEMP_LCRIT,
	SENSORS_SUBFEATURE_TEMP_EMERGENCY,
	SENSORS_SUBFEATURE_TEMP_EMERGENCY_HYST,
	SENSORS_SUBFEATURE_TEMP_LOWEST,
	SENSORS_SUBFEATURE_TEMP_HIGHEST,
	SENSORS_SUBFEATURE_TEMP_MIN_HYST,
	SENSORS_SUBFEATURE_TEMP_LCRIT_HYST,
	SENSORS_SUBFEATURE_TEMP_ALARM = (SENSORS_FEATURE_TEMP << 8) | 0x80,
	SENSORS_SUBFEATURE_TEMP_MAX_ALARM,
	SENSORS_SUBFEATURE_TEMP_MIN_ALARM,
	SENSORS_SUBFEATURE_TEMP_CRIT_ALARM,
	SENSORS_SUBFEATURE_TEMP_FAULT,
	SENSORS_SUBFEATURE_TEMP_TYPE,
	SENSORS_SUBFEATURE_TEMP_OFFSET,
	SENSORS_SUBFEATURE_TEMP_BEEP,
	SENSORS_SUBFEATURE_TEMP_EMERGENCY_ALARM,
	SENSORS_SUBFEATURE_TEMP_LCRIT_ALARM,

	SENSORS_SUBFEATURE_POWER_AVERAGE = SENSORS_FEATURE_POWER << 8,
	SENSORS_SUBFEATURE_POWER_AVERAGE_HIGHEST,
	SENSORS_SUBFEATURE_POWER_AVERAGE_LOWEST,
	SENSORS_SUBFEATURE_POWER_INPUT,
	SENSORS_SUBFEATURE_POWER_INPUT_HIGHEST,
	SENSORS_SUBFEATURE_POWER_INPUT_LOWEST,
	SENSORS_SUBFEATURE_POWER_CAP,
	SENSORS_SUBFEATURE_POWER_CAP_HYST,
	SENSORS_SUBFEATURE_POWER_MAX,
	SENSORS_SUBFEATURE_POWER_CRIT,
	SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL = (SENSORS_FEATURE_POWER << 8) | 0x80,
	SENSORS_SUBFEATURE_POWER_ALARM,
	SENSORS_SUBFEATURE_POWER_CAP_ALARM,
	SENSORS_SUBFEATURE_POWER_MAX_ALARM,
	SENSORS_SUBFEATURE_POWER_CRIT_ALARM,

	SENSORS_SUBFEATURE_ENERGY_INPUT = SENSORS_FEATURE_ENERGY << 8,

	SENSORS_SUBFEATURE_CURR_INPUT = SENSORS_FEATURE_CURR << 8,
	SENSORS_SUBFEATURE_CURR_MIN,
	SENSORS_SUBFEATURE_CURR_MAX,
	SENSORS_SUBFEATURE_CURR_LCRIT,
	SENSORS_SUBFEATURE_CURR_CRIT,
	SENSORS_SUBFEATURE_CURR_AVERAGE,
	SENSORS_SUBFEATURE_CURR_LOWEST,
	SENSORS_SUBFEATURE_CURR_HIGHEST,
	SENSORS_SUBFEATURE_CURR_ALARM = (SENSORS_FEATURE_CURR << 8) | 0x80,
	SENSORS_SUBFEATURE_CURR_MIN_ALARM,
	SENSORS_SUBFEATURE_CURR_MAX_ALARM,
	SENSORS_SUBFEATURE_CURR_BEEP,
	SENSORS_SUBFEATURE_CURR_LCRIT_ALARM,
	SENSORS_SUBFEATURE_CURR_CRIT_ALARM,

	SENSORS_SUBFEATURE_HUMIDITY_INPUT = SENSORS_FEATURE_HUMIDITY << 8,

	SENSORS_SUBFEATURE_VID = SENSORS_FEATURE_VID << 8,

	SENSORS_SUBFEATURE_INTRUSION_ALARM = SENSORS_FEATURE_INTRUSION << 8,
	SENSORS_SUBFEATURE_INTRUSION_BEEP,

	SENSORS_SUBFEATURE_BEEP_ENABLE = SENSORS_FEATURE_BEEP_ENABLE << 8,

	SENSORS_SUBFEATURE_UNKNOWN = INT_MAX,
} sensors_subfeature_type;

static
int get_type_scaling(sensors_subfeature_type type)
{
	/* Multipliers for subfeatures */
	switch (type & 0xFF80) {
	case SENSORS_SUBFEATURE_IN_INPUT:
	case SENSORS_SUBFEATURE_TEMP_INPUT:
	case SENSORS_SUBFEATURE_CURR_INPUT:
	case SENSORS_SUBFEATURE_HUMIDITY_INPUT:
		return 1000;
	case SENSORS_SUBFEATURE_FAN_INPUT:
		return 1;
	case SENSORS_SUBFEATURE_POWER_AVERAGE:
	case SENSORS_SUBFEATURE_ENERGY_INPUT:
		return 1000000;
	}

	/* Multipliers for second class subfeatures
	   that need their own multiplier */
	switch (type) {
	case SENSORS_SUBFEATURE_POWER_AVERAGE_INTERVAL:
	case SENSORS_SUBFEATURE_VID:
	case SENSORS_SUBFEATURE_TEMP_OFFSET:
		return 1000;
	default:
		return 1;
	}
}

const int TEMP_IDX_MAX = 3;
double AMD_FX_4100_tempInfo[ TEMP_IDX_MAX];

int get_cpu_temperature( double* t)
{
    FILE *f;
    const char* n[] = {"/sys/class/hwmon/hwmon0/device/temp1_input",
                       "/sys/class/hwmon/hwmon0/device/temp2_input",
                       "/sys/class/hwmon/hwmon0/device/temp3_input"};

    for ( int i = 0; i < TEMP_IDX_MAX; ++i) {
        if ( ( f = fopen( n[i], "r"))) {
            int res, err = 0;

            errno = 0;
            res = fscanf( f, "%lf", &t[i]);
            if ( res == EOF && errno == EIO)
                err = -SENSORS_ERR_IO;
            else if ( res != 1)
                err = -SENSORS_ERR_ACCESS_R;
            res = fclose( f);
            if ( err)
                return err;

            if ( res == EOF) {
                if ( errno == EIO)
                    return -SENSORS_ERR_IO;
                else
                    return -SENSORS_ERR_ACCESS_R;
            }
            t[i] /= get_type_scaling( SENSORS_SUBFEATURE_TEMP_INPUT);
        } else
            return -SENSORS_ERR_KERNEL;
    }
}

struct Line {
    char line[ 100];
    struct Line* next;
    struct Line* pred;
};

int main(void) {
    
    int lines = 0;
    struct Line* head = 0;
    struct Line* tail = 0;
    
    while(1)
    {
        usleep(4000000) ;
        FILE* log;
        if( ( log = fopen( "/home/peter/log/log_cpu_temperature.txt", "w")) == 0) {
            exit(-1);
        }
        int err = 0;
        if( ( err = get_cpu_temperature( AMD_FX_4100_tempInfo)) < 0) {
            fprintf( log, "error get_cpu_temperature:%d\n", err);
            exit( -1);
        }
        if( lines == MAX_LINE_COUNT)
        {
            struct Line* new_head = head->pred;
            free( head);
            head = new_head;
            head->next = 0;
            --lines;
        }
        
        struct Line* new_line = (Line*)malloc( sizeof(Line));
        struct tm* gmt;
        time_t now = time(0);
        gmt = gmtime( &now);
        char buff[50];
        strftime ( new_line->line, sizeof( new_line->line), "%Y-%m-%d %H:%M:%S, ", gmt);
        sprintf( buff, "temp1: %.4lf, temp2: %.4lf, temp3: %.4lf",
                AMD_FX_4100_tempInfo[0], AMD_FX_4100_tempInfo[1], AMD_FX_4100_tempInfo[2]);
        strcat( new_line->line, buff);
        new_line->next = tail;
        if( tail) tail->pred = new_line;
        new_line->pred = 0;
        tail = new_line;
        ++lines;
        if( lines == 1) head = new_line;
        
        for( struct Line* pl = head; ; pl = pl->pred) {
            if( fprintf( log, "%s\n", pl->line) < 0)
                exit(-1);
            if( pl->pred == 0) break;
        }
        fclose( log);
    }

    return 0;
}



