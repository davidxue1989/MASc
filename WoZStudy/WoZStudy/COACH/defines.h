#pragma once

#define NAO_PROMPT

#define USER_NAME "Ann"

#define TIME_WAIT_PROMPT    1
#define TIME_WAIT_TASK_FIN	30


//#define  robotIP "127.0.0.1"
//#define  robotIP "192.168.1.3"
#define  robotIP "192.168.1.121"
//#define  robotIP "192.168.0.1"
#define robotPort 9559

#define	FIRSTMONITORWIDTH	3200
#define	FIRSTMONITORHEIGHT	1800
#define	SECONDMONITORWIDTH	1680
#define	SECONDMONITORHEIGHT	1050
//#define	SECONDMONITORWIDTH	1920
//#define	SECONDMONITORHEIGHT	1080

//#define ADULT
#ifdef ADULT
	#define	DefaultHeadAngle1	0.322098047f
	#define	DefaultHeadAngle2	-0.503193974f
#else
	#define	DefaultHeadAngle1	0.0f
	#define	DefaultHeadAngle2	0.0f
#endif
