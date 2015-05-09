/*
 * Copyright (c) 2012, 2013, Joel Bodenmann aka Tectu <joel@unormal.org>
 * Copyright (c) 2012, 2013, Andrew Hannam aka inmarket
 *
 * All rights reserved.
 *
 * uRedistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * uRedistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * uRedistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the <organization> nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gfx.h"

gfxThreadHandle		thd1;
gfxThreadHandle		thd2;

/*
 * Function 1: Prints a message
 */
threadreturn_t Thread1_function(void* param)
{
	// We don't use the parameter
	(void)param;

	// We never return
	while (1) {
		printf("Message from Thread 1\r\n");
		gfxSleepMilliseconds(500);
	}

	return (threadreturn_t)0;
}

/*
 * Function 2: Prints a message and appends the parameter
 */
threadreturn_t Thread2_function(void* param)
{
	int i;

	// we never return
	while (1) {
		printf("Message from Thread 2: %s\r\n", (char*)param);
		gfxSleepMilliseconds(500);
	}

	return (threadreturn_t)0;
}

int main(void)
{
	char* msg = "uGFX";

	gfxInit();

	/* Create a static thread from the default heap with normal priority. No parameter passed */
	thd1 = gfxThreadCreate(NULL, 128, NORMAL_PRIORITY, Thread1_function, 0);

	/* Create a static thread from the default heap with normal priority. String parameter passed */
	thd2 = gfxThreadCreate(NULL, 128, NORMAL_PRIORITY, Thread2_function, (void*)msg);

    while(TRUE) {
    	gfxSleepMilliseconds(500);
    }   
}
