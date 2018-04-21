/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2014 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *****************************************************************************/

#ifndef __TRACER_H
#define __TRACER_H

#include "config.h"

#include "OpdeSingleton.h"

#include <cstddef>
#include <vector>

namespace Ogre {
class Timer;
};

namespace Opde {

/** Performance tracer. Writes performance probes with function names. */
class Tracer : public Singleton<Tracer> {
public:
    /** Constructor */
    Tracer(Ogre::Timer *timer);

    /** Destructor. Does not deallocate the listeners, as this is not a wanted
     * behavior. */
    ~Tracer();

    /** Logs a start frame event to tracer */
    void traceStartFrame();

    /** logs a tracer record used for performance tracing */
    void trace(bool start, const char *func, const void *data);

    /** logs a custom event */
    void tracePoint(const char *text);

    // Singleton related stuff
    static Tracer &getSingleton(void);
    static Tracer *getSingletonPtr(void);

private:
    Ogre::Timer *mTimer;

    /** Frame number for perf tracer */
    size_t mTraceFrameNum;
    unsigned long mFrameStartTime;

    struct TraceRecord {
        unsigned long time;
        unsigned long spent;
        bool entry;
        bool function;
        const void *data = nullptr;
        const char *text;
    };

    typedef std::vector<TraceRecord> TraceLog;

    TraceLog mTraces;
};

/** RAII performance probe */
class PerfTracer {
public:
    PerfTracer(const char *text, const void *instance = nullptr)
        : text(text), instance(instance) {
        Tracer::getSingleton().trace(true, text, instance);
    }

    ~PerfTracer() { Tracer::getSingleton().trace(false, text, instance); }

private:
    const char *text;
    const void *instance;
};

// Use this to place performance probes into code
#ifdef FRAME_PROFILER
#define TRACE_FRAME_BEGIN ::Opde::Tracer::getSingleton().traceStartFrame();
#define TRACE_FUNCTION ::Opde::PerfTracer _perfTracerInstance(__PRETTY_FUNCTION__);
#define TRACE_METHOD ::Opde::PerfTracer _perfTracerInstance(__PRETTY_FUNCTION__, this);
#define TRACE_SCOPE(text) ::Opde::PerfTracer _perfTracerInstance##text(#text);
#define TRACE_POINT(text) ::Opde::Tracer::getSingleton().tracePoint(#text);
#define TRACE_SCOPE_OBJ(text, obj) ::Opde::PerfTracer _perfTracerInstance##text(#text, obj);
#else
#define TRACE_FRAME_BEGIN
#define TRACE_FUNCTION
#define TRACE_METHOD
#define TRACE_SCOPE(text)
#define TRACE_POINT(text)
#define TRACE_SCOPE_OBJ(text, obj)
#endif

} // namespace Opde

#endif
