/*
 * Main class for the Geminga Lightweight Service Discovery for Spica module.
 */

#ifndef Spica_Geminga_h
#define Spica_Geminga_h

#include <vector>
#include <string>

#include "Zeroconf.h"

namespace spica { namespace geminga {

    class Geminga
    {
        public:
            Geminga(const std::vector<std::string> &args);
            ~Geminga();

            void configure();
            void start();
            void stop();
            void wait();

        protected:
            Zeroconf *zeroconf;

            bool configured;
            bool started;

            void init_avahi();
    };

} }

#endif /* Spica_Geminga_h */

