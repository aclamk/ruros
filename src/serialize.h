/*
 * serialize.h
 *
 *  Created on: Apr 9, 2015
 *      Author: adam
 */

#ifndef _RUROS_SERIALIZE_H_
#define _RUROS_SERIALIZE_H_
#include <ruros.h>
#include <string>
#include <vector>
typedef std::vector<std::string> vector_of_string;

namespace ruros
{
   template<typename T> Result serialize(std::string& data,const T& x);
   template<typename T> Result deserialize(std::string& data,T& x);
   template<typename T> std::string tostring(const T& x);
}

#endif /* SERIALIZE_H_ */
