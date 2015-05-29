// Module for software serial
// gbox3d custom 2015.5.29 

//#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"

#include "c_types.h"
#include "c_string.h"

#define DIRECT_MODE_OUTPUT(pin)   platform_gpio_mode(pin,PLATFORM_GPIO_OUTPUT,PLATFORM_GPIO_PULLUP)
#define DIRECT_WRITE_LOW(pin)    (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0))
#define DIRECT_WRITE_HIGH(pin)   (GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1))


static inline u8 chbit(u8 data, u8 bit)
{
    if ((data & bit) != 0)
    {
    	return 1;
    }
    else
    {
    	return 0;
    }
}

// Function for printing individual characters
static int soft_uart_putchar_c(u8 pin, unsigned _bit_time, char data)
{
	unsigned i;
	unsigned start_time = 0x7FFFFFFF & system_get_time();
	unsigned wait_time = start_time + (_bit_time*(i+1)/6));
	//Start Bit
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0);
	for(i = 0; i <= 8; i ++ )
	{
		//gbox3d custom
		while ((0x7FFFFFFF & system_get_time()) < wait_time)
		{
			//If system timer overflow, escape from while loop
			if ((0x7FFFFFFF & system_get_time()) < start_time){break;}
		}
		GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), chbit(data,1<<i));
	}

	// Stop bit
	//gbox3d custom
	wait_time = start_time + (_bit_time*9/6);
	while ((0x7FFFFFFF & system_get_time()) < wait_time)
	{
		//If system timer overflow, escape from while loop
		if ((0x7FFFFFFF & system_get_time()) < start_time){break;}
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1);

	// Delay after byte, for new sync
	//gbox3d *6 time = (b*4)+(b*2)=(b<<2)+(b<<1)
	os_delay_us((_bit_time<<2)+(_bit_time<<1));

	return 1;
}



// Lua: write( pin, baudrate, string1, [string2], ..., [stringn] )
static int softuart_write( lua_State* L )
{
  unsigned pin, baudrate, bit_time;

  const char* buf;
  size_t len, i;
  int total = lua_gettop( L ), s;
  
  pin = luaL_checkinteger( L, 1 );
  MOD_CHECK_ID( gpio, pin );

  baudrate = luaL_checkinteger( L, 2 );
  //if( baudrate > 57600 )
  //        return luaL_error( L, "max baudrate is 57600" );

  //bit_time = (1000000 / baudrate);
  bit_time = (6000000 / baudrate);

  if (pin >= 0)
  {
	  DIRECT_WRITE_HIGH(pin);
	  DIRECT_MODE_OUTPUT(pin);
	  os_delay_us((bit_time<<3)/6);
  }

  for( s = 3; s <= total; s ++ )
  {
    if( lua_type( L, s ) == LUA_TNUMBER )
    {
      len = lua_tointeger( L, s );
      if( len > 255 )
        return luaL_error( L, "invalid number" );
      soft_uart_putchar_c(pin,bit_time,( u8 )len);
    }
    else
    {
      luaL_checktype( L, s, LUA_TSTRING );
      buf = lua_tolstring( L, s, &len );
      for( i = 0; i < len; i ++ )
      	soft_uart_putchar_c(pin, bit_time, buf[ i ]);
    }
  }
  return 0;
}

// Module function map
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE softuart_map[] =
{
  { LSTRKEY( "write" ), LFUNCVAL( softuart_write ) },
#if LUA_OPTIMIZE_MEMORY > 0

#endif
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_softuart( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_SOFTUART, softuart_map );
  // Add constants

  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0  
}
