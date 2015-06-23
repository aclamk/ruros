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
ruros::Result serialize_vector_of_string(std::string& data,const vector_of_string& x);
ruros::Result deserialize_vector_of_string(std::string& data,vector_of_string& x);
std::string tostring_vector_of_string(const vector_of_string& x);

ruros::Result serialize_string(std::string& data,const std::string& x);
ruros::Result deserialize_string(std::string& data,std::string& x);
std::string tostring_string(const std::string& x);

ruros::Result serialize_uint16_t(std::string& data,const uint16_t x);
ruros::Result deserialize_uint16_t(std::string& data,uint16_t& x);
std::string tostring_uint16_t(const uint16_t x);

ruros::Result serialize_bool(std::string& data,const bool x);
ruros::Result deserialize_bool(std::string& data,bool& x);
std::string tostring_bool(const bool x);

ruros::Result serialize_int(std::string& data,const int x);
ruros::Result deserialize_int(std::string& data,int& x);
std::string tostring_int(const int x);


#endif /* SERIALIZE_H_ */
