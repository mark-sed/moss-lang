/**
 * @file memory.hpp
 * @author Marek Sedlacek
 * @copyright Copyright 2024 Marek Sedlacek. All rights reserved.
 *            See accompanied LICENSE file.
 * 
 * @brief Memory pools for the vm
 */

#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

namespace moss {

class MemoryPool {
private:
    //
public:
    MemoryPool(){}
    virtual ~MemoryPool() {}
};

}

#endif//_MEMORY_HPP_