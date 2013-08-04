/*
 * log.h

 *
 *  Created on: 2013-7-27
 *      Author: jx.peng
 */

#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>

#define ARES_DEBUG(fmt, arg...) do{ \
	fprintf(stderr, "[DEBUG][%s:%d]" fmt"\n", __FILE__, __LINE__, ##arg);\
}while(0);

#define ARES_NOTICE(fmt, arg...) do{ \
	fprintf(stderr, "[NOTICE][%s:%d]" fmt"\n", __FILE__, __LINE__, ##arg);\
}while(0);

#define ARES_WARNING(fmt, arg...) do{ \
	fprintf(stderr, "[WARNING][%s:%d]" fmt"\n", __FILE__, __LINE__, ##arg);\
}while(0);

#define ARES_FATAL(fmt, arg...) do{ \
	fprintf(stderr, "[FATAL][%s:%d]" fmt"\n", __FILE__, __LINE__, ##arg);\
}while(0);


#endif /* LOG_H_ */
