#pragma once
/**
 *  Classes.h
 *
 *  List of all declared classes
 *
 */

/**
 *  Set up namespace
 */
namespace AMQP {
    
/* MJ The classes below are exported */
#pragma GCC visibility push(default)


/**
 *  All classes defined by this library
 */
class Array;
class BasicDeliverFrame;
class BasicGetOKFrame;
class BasicHeaderFrame;
class BasicReturnFrame;
class BodyFrame;
class Channel;
class Connection;
class ConnectionHandler;
class ConnectionImpl;
class Exchange;
class Frame;
class Login;
class MessageImpl;
class Monitor;
class OutBuffer;
class ReceivedFrame;
class Table;

// MJ added because of missing vftable.
class ChannelImpl;
class Watchable;
class Field;

#pragma GCC visibility pop


/**
 *  End of namespace
 */
}

