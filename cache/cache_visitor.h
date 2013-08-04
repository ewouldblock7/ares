/*
 * cache_visitor.h

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#ifndef CACHE_VISITOR_H_
#define CACHE_VISITOR_H_
#include <boost/shared_ptr.hpp>

namespace ares {
class Slice;

class CacheVisitor {
public:
	CacheVisitor(){};
	virtual ~CacheVisitor(){};
	virtual void visit(const Slice & key, const boost::shared_ptr<void> & value) = 0;

};

} /* namespace ares */
#endif /* CACHE_VISITOR_H_ */
