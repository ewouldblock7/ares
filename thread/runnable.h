/*
 * runnable.h
 *
 *  Created on: 2013-8-4
 *      Author: seven
 */

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

namespace ares {

class Runnable {
public:
	Runnable(){};
	virtual ~Runnable(){};

	virtual void run() = 0;

private:
	Runnable(const Runnable &);
	Runnable & operator=(const Runnable &);
};

} /* namespace ares */
#endif /* RUNNABLE_H_ */
