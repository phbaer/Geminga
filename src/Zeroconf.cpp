#include <iostream>

#include "Zeroconf.h"

namespace spica { namespace geminga {

    Zeroconf::Zeroconf() :
        error(0), started(false), configured(false), client(NULL),
        group(NULL), poll(NULL), name(NULL)
    {
        int error;
        
        /* Allocate main loop object */
        if (!(this->poll = avahi_threaded_poll_new()))
        {
            std::cerr << "Failed to create threaded poll object." << std::endl;
            shutdown(-1);
            return;
        }

        /* Allocate a new client */
        this->client = avahi_client_new(
                avahi_threaded_poll_get(this->poll), (AvahiClientFlags)0,
                &Zeroconf::clientCallback, this, &error);

        /* Check wether creating the client object succeeded */
        if (!this->client)
        {
            std::cerr << "Failed to create client: " <<
                avahi_strerror(error) << std::endl;
            shutdown(-1);
            return;
        }
    }

    Zeroconf::~Zeroconf()
    {
        shutdown();
    }

    void Zeroconf::start()
    {
        if (!this->started)
        {
            /* Run the main loop */
            avahi_threaded_poll_start(this->poll);
            this->started = true;
        }
    }

    void Zeroconf::stop()
    {
        if (this->started)
        {
            avahi_threaded_poll_stop(this->poll);
            this->started = false;
        }
    }

    int Zeroconf::failed()
    {
        return this->error;
    }

    void Zeroconf::shutdown(int error)
    {
        if (this->client)
        {
            avahi_client_free(this->client);
            this->client = NULL;
        }

        if (this->poll)
        {
            avahi_threaded_poll_free(this->poll);
            this->poll = NULL;
        }

        if (this->name)
        {
            avahi_free(this->name);
            this->name = NULL;
        }

        this->error = error;
    }

    void Zeroconf::updateName(AvahiClient *client, const char *hostname)
    {
        // Get local hostname if no name is provided
        if (!hostname)
        {
            hostname = avahi_client_get_host_name(client);
        }

        // Free local name
        if (this->name)
        {
            avahi_free(this->name);
        }

        // Update local name
        this->name = avahi_strdup_printf("Geminga@%s", hostname);
    }

    void Zeroconf::addService(const char *proto, int port)
    {
        int ret = 0;

        // Add the service
        if ((ret = avahi_entry_group_add_service(this->group,
                 AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0,
                 this->name, proto, NULL, NULL, port, NULL)) < 0)
        {
            if (ret == AVAHI_ERR_COLLISION)
            {
                std::cerr << "Service collition detected" << std::endl;
                shutdown(ret);
                return;
            }

            std::cerr << "Failed to add Geminga " << proto << " service: "
                << avahi_strerror(ret) << std::endl;
            shutdown(-1);
            return;
        }
    }


    void Zeroconf::createService(AvahiClient *client)
    {
        int ret = 0;

        if (!this->group)
        {
            if (!(this->group = avahi_entry_group_new(
                    client, entryGroupCallback, this)))
            {
                std::cerr << "avahi_entry_group_new() failed: " << 
                    avahi_strerror(avahi_client_errno(client)) << std::endl;
                return;
            }
        }

        // If the group is empty (either because it was just created, or
        // because it was reset previously, add our entries.
        if (avahi_entry_group_is_empty(this->group))
        {
            addService("_geminga_config._tcp", 60080);
            addService("_geminga_announce._udp", 60000);

            // Tell the server to register the service
            if ((ret = avahi_entry_group_commit(group)) < 0)
            {
                std::cerr << "Failed to commit entry group: " << avahi_strerror(ret) << std::endl;
                return;
            }
        }

        this->configured = true;
    }

    void Zeroconf::entryGroupCallback(AvahiEntryGroup *g,
                                   AvahiEntryGroupState state,
                                   AVAHI_GCC_UNUSED void *userdata)
    {
        Zeroconf *zeroconf = static_cast<Zeroconf *>(userdata);
        assert(zeroconf);

        assert(g == zeroconf->group || zeroconf->group == NULL);
    
        zeroconf->group = g;

        /* Called whenever the entry group state changes */
        switch (state)
        {
            case AVAHI_ENTRY_GROUP_ESTABLISHED:
                /* The entry group has been established successfully */
                std::cerr << "Service '" << zeroconf->name << "' successfully established." << std::endl;
                break;

            case AVAHI_ENTRY_GROUP_COLLISION:
                {
                    // A service name collision with a remote service
                    // happened. Let's pick a new name
                    zeroconf->updateName(avahi_entry_group_get_client(g),
                                         avahi_alternative_service_name(zeroconf->name));
        
                    std::cerr << "Service name collision, renaming service to '" << zeroconf->name << "'" << std::endl;
        
                    /* And recreate the services */
                    zeroconf->createService(avahi_entry_group_get_client(g));
                    break;
                }
    
            case AVAHI_ENTRY_GROUP_FAILURE:
                std::cerr << "Entry group failure: " << avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))) << std::endl;
    
                /* Some kind of failure happened while we were registering our services */
                avahi_threaded_poll_quit(zeroconf->poll);
                break;
    
            case AVAHI_ENTRY_GROUP_UNCOMMITED:
            case AVAHI_ENTRY_GROUP_REGISTERING:
                break;
        }
    }

    void Zeroconf::clientCallback(AvahiClient *c,
                                  AvahiClientState state,
                                  AVAHI_GCC_UNUSED void *userdata)
    {
        assert(c);

        Zeroconf *zeroconf = static_cast<Zeroconf *>(userdata);
        assert(zeroconf);

        zeroconf->updateName(c);


        /* Called whenever the client or server state changes */
        switch (state)
        {
            case AVAHI_CLIENT_S_RUNNING:
                /* The server has startup successfully and registered its host
                 * name on the network, so it's time to create our services */
                zeroconf->createService(c);
                break;
    
            case AVAHI_CLIENT_FAILURE:
                std::cerr << "Client failure: " << avahi_strerror(avahi_client_errno(c)) << std::endl;
                avahi_threaded_poll_quit(zeroconf->poll);
                break;
    
            case AVAHI_CLIENT_S_COLLISION:
                /* Let's drop our registered services. When the server is back
                 * in AVAHI_SERVER_RUNNING state we will register them
                 * again with the new host name. */
    
            case AVAHI_CLIENT_S_REGISTERING:
                /* The server records are now being established. This
                 * might be caused by a host name change. We need to wait
                 * for our own records to register until the host name is
                 * properly esatblished. */
                if (zeroconf->group)
                {
                    avahi_entry_group_reset(zeroconf->group);
                }
                break;
    
            case AVAHI_CLIENT_CONNECTING:
                break;
        }
    }

} }

