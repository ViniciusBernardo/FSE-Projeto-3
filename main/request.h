#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "cJSON.h"

void get_weather_data();
void http_request(char * url);
char * get_ipstack_url(char * ip_address);
char * get_openweathermap_url(cJSON * json);

#endif