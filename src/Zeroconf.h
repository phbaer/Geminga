/*
 * Zeroconf adapter class for the Geminga Lightweight Service Discovery for Spica
 * module.
 *
 * Based on the Avahi example program client-publish-service.c
 */

#ifndef Spica_Zeroconf_h
#define Spica_Zeroconf_h

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

namespace spica { namespace geminga {

    class Zeroconf
    {
        public:
            Zeroconf();
            ~Zeroconf();

            void start();
            void stop();

            int failed();

        private:
            int error;

            bool started;
            bool configured;

            AvahiClient *client;

            void shutdown(int error = 0);

            AvahiEntryGroup *group;
            AvahiThreadedPoll *poll;
            char *name;

            void updateName(AvahiClient *client, const char *hostname = NULL);
            void addService(const char *proto, int port);
            void createService(AvahiClient *client);

            static void entryGroupCallback(AvahiEntryGroup *g,
                                           AvahiEntryGroupState state,
                                           AVAHI_GCC_UNUSED void *userdata);

            static void clientCallback(AvahiClient *c,
                                       AvahiClientState state,
                                       AVAHI_GCC_UNUSED void * userdata);
    };

} }

#endif /* Spica_Zeroconf_h */

