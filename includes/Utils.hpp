/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thjacque <thjacque@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/10 14:51:19 by thallard          #+#    #+#             */
/*   Updated: 2021/05/17 14:27:39 by thjacque         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <map>
#include <vector>
#include <pthread.h>
using namespace std;


#define ERROR 0
#define SUCCESS 1

#define STATUS_OK 200
#define STATUS_NO_CONTENT 204

#define STATUS_NOT_FOUND 404

#define DEFAULT_PATH "default/default.conf"

#define MAX_WORKERS 50 

#define DEFAULT_PORT 8080

int dispatcher_type_requests(map<string, string> request);
void *main_loop(void * arg);


#endif