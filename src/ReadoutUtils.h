#ifndef _READOUTUTILS_H
#define _READOUTUTILS_H

#include <string>

#include <Common/Configuration.h>

#include "RAWDataHeader.h"

namespace ReadoutUtils {

// function to convert a string to a 64-bit integer value
// allowing usual "base units" in suffix (k,M,G,T)
// input can be decimal (1.5M is valid, will give 1.5*1024*1024)
long long getNumberOfBytesFromString(const char * inputString);

// function to convert a value in bytes to a prefixed number 3+3 digits
// suffix is the "base unit" to add after calculated prefix, e.g. Byte-> kBytes
std::string NumberOfBytesToString(double value,const char*suffix);

}


// print RDH struct content to stdout
void dumpRDH(o2::Header::RAWDataHeader *rdh);


// end of _READOUTUTILS_H
#endif
