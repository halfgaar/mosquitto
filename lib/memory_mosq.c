/*
Copyright (c) 2009-2019 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "memory_mosq.h"

#include <pthread.h>

#ifdef REAL_WITH_MEMORY_TRACKING
#  if defined(__APPLE__)
#    include <malloc/malloc.h>
#    define malloc_usable_size malloc_size
#  elif defined(__FreeBSD__)
#    include <malloc_np.h>
#  else
#    include <malloc.h>
#  endif
#endif

#ifdef REAL_WITH_MEMORY_TRACKING
static unsigned long memcount = 0;
static unsigned long max_memcount = 0;
#endif

#ifdef WITH_BROKER
static size_t mem_limit = 0;
void memory__set_limit(size_t lim)
{
	pthread_mutex_lock(&mem_mutex);
	mem_limit = lim;
	pthread_mutex_unlock(&mem_mutex);
}
#endif

void *mosquitto__calloc(size_t nmemb, size_t size)
{
	pthread_mutex_lock(&mem_mutex);
#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem_limit && memcount + size > mem_limit){
		pthread_mutex_unlock(&mem_mutex);
		return NULL;
	}
#endif
	void *mem = calloc(nmemb, size);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem){
		memcount += malloc_usable_size(mem);
		if(memcount > max_memcount){
			max_memcount = memcount;
		}
	}
#endif

	pthread_mutex_unlock(&mem_mutex);
	return mem;
}

void mosquitto__free(void *mem)
{
	pthread_mutex_lock(&mem_mutex);
#ifdef REAL_WITH_MEMORY_TRACKING
	if(!mem){
		pthread_mutex_unlock(&mem_mutex);
		return;
	}
	memcount -= malloc_usable_size(mem);
#endif
	free(mem);
	pthread_mutex_unlock(&mem_mutex);
}

void *mosquitto__malloc(size_t size)
{
	pthread_mutex_lock(&mem_mutex);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem_limit && memcount + size > mem_limit){
		pthread_mutex_unlock(&mem_mutex);
		return NULL;
	}
#endif
	void *mem = malloc(size);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem){
		memcount += malloc_usable_size(mem);
		if(memcount > max_memcount){
			max_memcount = memcount;
		}
	}
#endif

	pthread_mutex_unlock(&mem_mutex);
	return mem;
}

#ifdef REAL_WITH_MEMORY_TRACKING
unsigned long mosquitto__memory_used(void)
{
	return memcount;
}

unsigned long mosquitto__max_memory_used(void)
{
	return max_memcount;
}
#endif

void *mosquitto__realloc(void *ptr, size_t size)
{
	pthread_mutex_lock(&mem_mutex);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem_limit && memcount + size > mem_limit){
		pthread_mutex_unlock(&mem_mutex);
		return NULL;
	}
#endif
	void *mem;
#ifdef REAL_WITH_MEMORY_TRACKING
	if(ptr){
		memcount -= malloc_usable_size(ptr);
	}
#endif
	mem = realloc(ptr, size);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem){
		memcount += malloc_usable_size(mem);
		if(memcount > max_memcount){
			max_memcount = memcount;
		}
	}
#endif

	pthread_mutex_unlock(&mem_mutex);
	return mem;
}

char *mosquitto__strdup(const char *s)
{
	pthread_mutex_lock(&mem_mutex);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(mem_limit && memcount + strlen(s) > mem_limit){
		pthread_mutex_unlock(&mem_mutex);
		return NULL;
	}
#endif
	char *str = strdup(s);

#ifdef REAL_WITH_MEMORY_TRACKING
	if(str){
		memcount += malloc_usable_size(str);
		if(memcount > max_memcount){
			max_memcount = memcount;
		}
	}
#endif

	pthread_mutex_unlock(&mem_mutex);
	return str;
}

