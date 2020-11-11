#ifndef _REQUESTTYPES_H_
#define _REQUESTTYPES_H_

enum RequestType
{
    Numeric_Request                             = 0x00,
    String_Request                              = 0x01,
    List_Request                                = 0x02,
    Generic_Request                             = 0x03,

    Force_Reconnect                             = 0xFE,
    Kill                                        = 0xFF

};


#endif