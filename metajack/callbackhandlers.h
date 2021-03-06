#ifndef CALLBACKHANDLERS_H
#define CALLBACKHANDLERS_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "metajackclient.h"
#include <map>
#include <utility>

template<class T> class CallbackHandler : public std::map<MetaJackClient*, typename std::pair<T, void*> > {
public:
    void setCallback(MetaJackClient *client, T callback, void *callbackArgument) {
        if (callback) {
            (*this)[client] = std::make_pair(callback, callbackArgument);
        } else {
            this->erase(client);
        }
    }
    void invokeCallbacks() {
        for (typename std::map<MetaJackClient*, typename std::pair<T, void*> >::iterator i = this->begin(); i != this->end(); i++) {
            if (i->first->isActive()) {
                invokeCallback(i->second.first, i->second.second);
            }
        }
    }
protected:
    virtual void invokeCallback(T callback, void *callbackArgument) = 0;
};

class JackThreadInitCallbackHandler : public CallbackHandler<JackThreadInitCallback> {
public:
    static void invokeCallbacksWithoutArgs(void *arg) {
        JackThreadInitCallbackHandler *callbackHandler = (JackThreadInitCallbackHandler*)arg;
        callbackHandler->invokeCallbacks();
    }
protected:
    void invokeCallback(JackThreadInitCallback callback, void *arg) {
        callback(arg);
    }
};

class JackShutdownCallbackHandler : public CallbackHandler<JackShutdownCallback> {
protected:
    virtual void invokeCallback(JackShutdownCallback callback, void *arg) {
        callback(arg);
    }
};

class JackInfoShutdownCallbackHandler : public CallbackHandler<JackInfoShutdownCallback> {
public:
    void invokeCallbacksWithArgs(jack_status_t statusCode, const char *reason) {
        this->statusCode = statusCode;
        this->reason = reason;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackInfoShutdownCallback callback, void *arg) {
        callback(statusCode, reason, arg);
    }
private:
    jack_status_t statusCode;
    const char* reason;
};

class JackFreewheelCallbackHandler : public CallbackHandler<JackFreewheelCallback> {
public:
    void invokeCallbacksWithArgs(int starting) {
        this->starting = starting;
        invokeCallbacks();
    }
    static void invokeCallbacksWithArgs(int starting, void *arg) {
        JackFreewheelCallbackHandler *callbackHandler = (JackFreewheelCallbackHandler*)arg;
        callbackHandler->invokeCallbacksWithArgs(starting);
    }
protected:
    virtual void invokeCallback(JackFreewheelCallback callback, void *arg) {
        callback(starting, arg);
    }
private:
    int starting;
};

class JackBufferSizeCallbackHandler : public CallbackHandler<JackBufferSizeCallback> {
public:
    void invokeCallbacksWithArgs(jack_nframes_t bufferSize) {
        this->bufferSize = bufferSize;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackBufferSizeCallback callback, void *arg) {
        callback(bufferSize, arg);
    }
private:
    jack_nframes_t bufferSize;
};

class JackSampleRateCallbackHandler : public CallbackHandler<JackSampleRateCallback> {
public:
    void invokeCallbacksWithArgs(jack_nframes_t sampleRate) {
        this->sampleRate = sampleRate;
        invokeCallbacks();
    }
    static int invokeCallbacksWithArgs(jack_nframes_t sampleRate, void *arg) {
        JackSampleRateCallbackHandler *callbackHandler = (JackSampleRateCallbackHandler*)arg;
        callbackHandler->invokeCallbacksWithArgs(sampleRate);
        return 0;
    }
protected:
    virtual void invokeCallback(JackSampleRateCallback callback, void *arg) {
        callback(sampleRate, arg);
    }
private:
    jack_nframes_t sampleRate;
};

class JackClientRegistrationCallbackHandler : public CallbackHandler<JackClientRegistrationCallback> {
public:
    void invokeCallbacksWithArgs(const char* name, int registered) {
        this->name = name;
        this->registered = registered;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackClientRegistrationCallback callback, void *arg) {
        callback(name, registered, arg);
    }
private:
    const char* name;
    int registered;
};

class JackPortRegistrationCallbackHandler : public CallbackHandler<JackPortRegistrationCallback> {
public:
    void invokeCallbacksWithArgs(jack_port_id_t portId, int registered) {
        this->portId = portId;
        this->registered = registered;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackPortRegistrationCallback callback, void *arg) {
        callback(portId, registered, arg);
    }
private:
    jack_port_id_t portId;
    int registered;
};

class JackPortConnectCallbackHandler : public CallbackHandler<JackPortConnectCallback> {
public:
    void invokeCallbacksWithArgs(jack_port_id_t a, jack_port_id_t b, int connect) {
        this->a = a;
        this->b = b;
        this->connect = connect;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackPortConnectCallback callback, void *arg) {
        callback(a, b, connect, arg);
    }
private:
    jack_port_id_t a, b;
    int connect;
};

class JackPortRenameCallbackHandler : public CallbackHandler<JackPortRenameCallback> {
public:
    void invokeCallbacksWithArgs(jack_port_id_t portId, const char *oldName, const char *newName) {
        this->portId = portId;
        this->oldName = oldName;
        this->newName = newName;
        invokeCallbacks();
    }
protected:
    virtual void invokeCallback(JackPortRenameCallback callback, void *arg) {
        callback(portId, oldName, newName, arg);
    }
private:
    jack_port_id_t portId;
    const char *oldName, *newName;
};

class JackGraphOrderCallbackHandler : public CallbackHandler<JackGraphOrderCallback> {
protected:
    virtual void invokeCallback(JackGraphOrderCallback callback, void *arg) {
        callback(arg);
    }
};

class JackXRunCallbackHandler : public CallbackHandler<JackXRunCallback> {
public:
    static int invokeCallbacksWithoutArgs(void *arg) {
        JackXRunCallbackHandler *callbackHandler = (JackXRunCallbackHandler*)arg;
        callbackHandler->invokeCallbacks();
        return 0;
    }
protected:
    virtual void invokeCallback(JackXRunCallback callback, void *arg) {
        callback(arg);
    }
};

class JackSyncCallbackHandler : public CallbackHandler<JackSyncCallback> {
public:
    int invokeCallbacksWithArgs(jack_transport_state_t state, jack_position_t *pos) {
        this->ready = true;
        this->state = state;
        this->pos = pos;
        invokeCallbacks();
        return ready ? 1 : 0;
    }
    static int invokeCallbacksWithArgs(jack_transport_state_t state, jack_position_t *pos, void *arg) {
        JackSyncCallbackHandler *callbackHandler = (JackSyncCallbackHandler*)arg;
        return callbackHandler->invokeCallbacksWithArgs(state, pos);
    }
protected:
    virtual void invokeCallback(JackSyncCallback callback, void *arg) {
        ready = ready && callback(state, pos, arg);
    }
private:
    jack_transport_state_t state;
    jack_position_t *pos;
    bool ready;
};

#endif//CALLBACKHANDLERS_H
