/*
||
|| @file ATWEBSERVER.h
|| @version 0.5
|| @author Gerald Lechner
|| @contact lechge@gmail.com
||
|| @description
|| |This library defines functions to show lists and forms on a TFT display
|| |It also interacts with Touch-Screen events
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/
#include <Arduino.h>
#include <AT_Database.h>
#include "WebServer.h"


#ifndef AT_Webserver_h
#define AT_Webserver_h

class AT_Webserver {
  public:
    AT_Webserver(WebServer * srv, AT_Database *database);
    void begin();
    void handleClient();
    void handleRoot(uint8_t refresh = 10);
    void handleNotFound();
  private:
    uint32_t convertColor(uint16_t color565);
    void sendResults();
    void sendSimpleWidget(ATDISPLAYWIDGET wdg);
    AT_Database * _database;
    WebServer * _server;
};

#endif
