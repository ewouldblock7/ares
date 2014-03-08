/*
 * Slice.h

 *
 *  Created on: 2013-4-27
 *      Author: seven
 */

#ifndef SLICE_H_
#define SLICE_H_
#include <stdint.h>
#include <string.h>
#include <string>

namespace ares {

class Slice {
public:
    Slice(const std::string & str) :
            data_(str.c_str()), size_(str.size()) {
    }
    Slice(const char * data, uint32_t size) :
            data_(data), size_(size) {
    }
    ~Slice() {
    }

    const char * data() const {
        return data_;
    }
    uint32_t size() const {
        return size_;
    }

private:
    const char * data_;
    uint32_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size())
            && (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
    return !(x == y);
}

} /* namespace ares */
#endif /* SLICE_H_ */
