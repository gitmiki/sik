#ifndef CONFIG_HPP
#define CONFIG_HPP

static bool DEBUG = true;

static int UDP_PORT = 3382; //default 3382, change with -u
static int INTERFACE_PORT = 3637; //default 3637, change with -U
static int FIND_INTERVAL = 1; //default 1, change with -t
static int SERVICES_INTERVAL = 10; //default 10, change with -T
static float REFRESH_TIME = 1; //default 1, change with -v
static bool DNS_SD = false; //default faulse, change with -s

#endif /* CONFIG_HPP */
