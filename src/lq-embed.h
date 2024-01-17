/******************************************************************************
 *  \file lq-embed.h
 *  \author Greg Terrell
 *  \license MIT License
 *
 *  Copyright (c) 2023 LooUQ Incorporated.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef __LQ_EMBED_H__
#define __LQ_EMBED_H__


#define lqLOGLEVEL_VRBS	    (5)
#define lqLOGLEVEL_DBG	    (4)
#define lqLOGLEVEL_INFO	    (3)
#define lqLOGLEVEL_WARN	    (2)
#define lqLOGLEVEL_ERROR 	(1)
#define lqLOGLEVEL_OFF	    (0)

#ifndef lqLOGBUFFER_SZ
     #define LOGBUFFER_SZ 256
#endif
#ifndef DBGBUFFER_SZ
     #define DBGBUFFER_SZ 180
#endif

#endif // __LQ_EMBED_H__

