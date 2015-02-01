// Module for RC servo interfacing by Milan Spacek

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_types.h"

#define DIRECT_MODE_OUTPUT(pin)	 platform_gpio_mode(pin,PLATFORM_GPIO_OUTPUT,PLATFORM_GPIO_PULLUP)
#define DIRECT_WRITE_LOW(pin)    (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0))
#define DIRECT_WRITE_HIGH(pin)   (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1))



static os_timer_t servo_timer;

static int servo_pins[4] = {-1,-1,-1,-1};
static int servo_pulse_lenght[4] = {1500,1500,1500,1500};

static unsigned current_servo = 0;
static int next_servo[4] = {1,2,3,0};
static int time_to_next_pulse[4] = {5,5,5,5};

static unsigned run = 0;


/*
char debug_buffer [128];
void debug(char *dataPtr)
{
    while ( *dataPtr )
    {
    	platform_uart_send( 0, *dataPtr++ );
    }
}
*/

int find_times_between_pulses(void)
{
	int tmp_time,find,i,ii,finded;

	finded = 0;
	for (i = 0 ; i < 4; i++)
	{
	  find = 0;
	  tmp_time = 5;
	  for (ii = 0 ; ii < 4; ii++)
	  {
			if (servo_pins[(ii+i+1)%4] == (-1) && find == 0)
			{
				tmp_time+=5;
			}
			else
			{
				find = 1;
				next_servo[i] = (ii+i+1)%4;
				time_to_next_pulse[i] = tmp_time;
				break;
			}
		}
		finded += find;
	}

/*
	for (i = 0 ; i < 4; i++)
	{
		os_sprintf(debug_buffer, "id %d next servo:%d,time%d\r\n",i,next_servo[i],time_to_next_pulse[i]);
		debug(debug_buffer);
	}
*/

return finded;
}




servo_timer_tick(void) // servo timer function
{
	if (run == 1 && time_to_next_pulse[current_servo] >= 0)
	{
		os_timer_disarm(&servo_timer); // dis_arm the timer
		os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
		os_timer_arm(&servo_timer, time_to_next_pulse[current_servo], 1); // arm the timer every 5ms and repeat

		if (servo_pins[current_servo] >= 0)
		{
			DIRECT_WRITE_HIGH(servo_pins[current_servo]);	// drive output high
			os_delay_us(servo_pulse_lenght[current_servo]);
			DIRECT_WRITE_LOW(servo_pins[current_servo]);	// drive output low
		}

		//os_sprintf(debug_buffer, "id=%d   time=%d   next=%d \r\n",current_servo,time_to_next_pulse[current_servo],next_servo[current_servo]);
		//debug(debug_buffer);

		current_servo = next_servo[current_servo];
	}
	else
	{
		os_timer_disarm(&servo_timer); // dis_arm the timer
	}
}



// Lua: setup( id, pin, default_pulse_lenght )
static int servo_setup( lua_State* L )
{
  unsigned id, pin, default_pulse;

  id = luaL_checkinteger( L, 1 );
  if (id > 3)
        return luaL_error( L, "wrong id, must be 0-3" );

  pin = luaL_checkinteger( L, 2 );
  MOD_CHECK_ID( gpio, pin );

  default_pulse = luaL_checkinteger( L, 3 );
  if (default_pulse < 500 || default_pulse > 2500 )
        return luaL_error( L, "wrong pulse length, must be 500-2500" );

  //os_sprintf(debug_buffer, "id=%d    pin=%d    time=%d",id,pin,default_pulse);
  //debug(debug_buffer);

  if (id >= 0 && id <= 3)
  {
	  servo_pins[id] = pin;
	  servo_pulse_lenght[id] = default_pulse;


	  if (find_times_between_pulses() > 0)
	  {
		  run = 1;

		  if (servo_pins[id] != 0)
		  {
			  DIRECT_WRITE_LOW(pin);
		  	  DIRECT_MODE_OUTPUT(pin);
		  }

		  os_timer_disarm(&servo_timer); // dis_arm the timer
		  os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
		  os_timer_arm(&servo_timer, time_to_next_pulse[current_servo], 1); // arm the timer every 5ms and repeat
	  }
	  else
	  {
		  run = 0;
		  os_timer_disarm(&servo_timer); // dis_arm the timer
	  }

  }

  return 1;
}




// Lua: position( id, pulse_lenght )
static int servo_position( lua_State* L )
{
  unsigned id, pulse_lenght;

  id = luaL_checkinteger( L, 1 );
  if (id > 3)
        return luaL_error( L, "wrong id, must be 0-3" );

  pulse_lenght = luaL_checkinteger( L, 2 );
  if (pulse_lenght < 500 || pulse_lenght > 2500 )
        return luaL_error( L, "wrong pulse length, must be 500-2500" );

  //os_sprintf(debug_buffer, "id=%d   time=%d",id,pulse_lenght);
  //debug(debug_buffer);

  if (id >= 0 && id <= 3)
  {
	  servo_pulse_lenght[id] = pulse_lenght;
  }

  return 1;
}




// Lua: stop()
static int servo_stop( lua_State* L )
{
	unsigned id;

	id = luaL_checkinteger( L, 1 );
	if (id > 3)
		return luaL_error( L, "wrong id, must be 0-3" );

	servo_pins[id] = -1;

	if (find_times_between_pulses() > 0)
	{
	  run = 1;

	  os_timer_disarm(&servo_timer); // dis_arm the timer
	  os_timer_setfn(&servo_timer, (os_timer_func_t *)servo_timer_tick, NULL); // set the timer function, dot get os_timer_func_t to force function convert
	  os_timer_arm(&servo_timer, time_to_next_pulse[current_servo], 1); // arm the timer every 5ms and repeat
	}
	else
	{
	  run = 0;
	  os_timer_disarm(&servo_timer); // dis_arm the timer
	}

  return 1;
}






// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE servo_map[] =
{
  { LSTRKEY( "setup" ),  LFUNCVAL( servo_setup ) },
  { LSTRKEY( "position" ), LFUNCVAL( servo_position ) },
  { LSTRKEY( "stop" ), LFUNCVAL( servo_stop ) },
#if LUA_OPTIMIZE_MEMORY > 0

#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_servo( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_SERVO, servo_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
