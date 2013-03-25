/*
  MQTT-S command-line publishing client
  Copyright (C) 2013 Nicholas Humfrey

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (http://www.gnu.org/copyleft/gpl.html)
  for more details.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "mqtts.h"

char client_id[21] = "";
const char *topic_name = NULL;
const char *message_data = NULL;
const char *mqtts_host = "127.0.0.1";
const char *mqtts_port = "1883";
uint16_t keep_alive = 0;
uint16_t topic_id = 0;
uint8_t qos = 0;
uint8_t retain = FALSE;
uint8_t debug = FALSE;


static void usage()
{
    fprintf(stderr, "Usage: mqtts-pub [opts] -t <topic> -m <message>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -d             Enable debug messages.\n");
    fprintf(stderr, "  -h <host>      MQTT-S host to connect to. Defaults to '%s'.\n", mqtts_host);
    fprintf(stderr, "  -i <clientid>  ID to use for this client. Defaults to 'mqtts-client-$$'.\n");
    fprintf(stderr, "  -m <message>   Message payload to send.\n");
    fprintf(stderr, "  -n             Send a null (zero length) message.\n");
    fprintf(stderr, "  -p <port>      Network port to connect to. Defaults to %s.\n", mqtts_port);
    fprintf(stderr, "  -r             Message should be retained.\n");
    fprintf(stderr, "  -t <topic>     MQTT topic name to publish to.\n");
    exit(-1);
}

static void parse_opts(int argc, char** argv)
{
    int ch;

    // Parse the options/switches
    while ((ch = getopt(argc, argv, "dh:i:m:nprt:?")) != -1)
        switch (ch) {
        case 'd':
            debug = TRUE;
        break;

        case 'h':
            mqtts_host = optarg;
        break;

        case 'i':
            strcpy(client_id, optarg);
        break;

        case 'm':
            message_data = optarg;
        break;

        case 'n':
            message_data = "";
        break;

        case 'p':
            mqtts_port = optarg;
        break;

        case 'r':
            retain = TRUE;
        break;

        case 't':
            topic_name = optarg;
        break;

        case '?':
        default:
            usage();
        break;
    }

    // Generate a Client ID if none given
    if (client_id[0] == '\0') {
        snprintf(client_id, sizeof(client_id)-1, "mqtts-client-%d", getpid());
    }

    // Missing Parameter?
    if (!topic_name || !message_data) {
        usage();
    }
}

int main(int argc, char* argv[])
{
    int sock;

    // Parse the command-line options
    parse_opts(argc, argv);
    
    // Enable debugging?
    mqtts_set_debug(debug);

    // Create a UDP socket
    sock = create_socket(mqtts_host, mqtts_port);
    if (sock) {
        // Connect to gateway
        send_connect(sock, client_id, keep_alive);
        recieve_connack(sock);

        // Register the topic
        send_register(sock, topic_name);
        topic_id = recieve_regack(sock);

        // Publish to the topic
        send_publish(sock, topic_id, message_data, qos, retain);

        // Finally, disconnect
        send_disconnect(sock);

        close(sock);
    }

    return 0;
}
