#include "Geminga.h"

namespace spica { namespace geminga {

    Geminga::Geminga(const std::vector<std::string> &args) :
        zeroconf(NULL)
    {
        // Do not process command line arguments as for now
    }

    Geminga::~Geminga()
    {
        if (this->zeroconf)
        {
            delete this->zeroconf;
        }
    }

    void Geminga::configure()
    {
        if (!this->configured)
        {
            this->zeroconf = new Zeroconf();
            this->configured = true;
        }
    }

    void Geminga::start()
    {
        if (!this->started)
        {
            this->zeroconf->start();
            this->started = true;
        }
    }

    void Geminga::stop()
    {
        if (this->started)
        {
            this->zeroconf->stop();
            this->started = false;
        }
    }

    void Geminga::wait()
    {
    }

    void Geminga::init_avahi()
    {
    }

} }

