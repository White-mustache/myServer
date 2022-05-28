
#ifndef _MQTT_ERROR_H_
#define _MQTT_ERROR_H_


typedef enum mypro_error {
    M_SSL_CERT_ERROR                                     = -0x001C,      /* cetr parse failed */
    M_SOCKET_FAILED_ERROR                                = -0x001B,      /* socket fd failed */
    M_SOCKET_UNKNOWN_HOST_ERROR                          = -0x001A,      /* socket unknown host ip or domain */ 
    M_SET_PUBLISH_DUP_FAILED_ERROR                       = -0x0019,      /* mqtt publish packet set udp bit failed */
    M_CLEAN_SESSION_ERROR                                = -0x0018,      /* mqtt clean session error */
    M_ACK_NODE_IS_EXIST_ERROR                            = -0x0017,      /* mqtt ack list is exist ack node */
    M_ACK_HANDLER_NUM_TOO_MUCH_ERROR                     = -0x0016,      /* mqtt ack handler number is too much */
    M_RESUBSCRIBE_ERROR                                  = -0x0015,      /* mqtt resubscribe error */
    M_SUBSCRIBE_ERROR                                    = -0x0014,      /* mqtt subscribe error */
    M_SEND_PACKET_ERROR                                  = -0x0013,      /* mqtt send a packet */
    M_SERIALIZE_PUBLISH_ACK_PACKET_ERROR                 = -0x0012,      /* mqtt serialize publish ack packet error */
    M_PUBLISH_PACKET_ERROR                               = -0x0011,      /* mqtt publish packet error */
    M_RECONNECT_TIMEOUT_ERROR                            = -0x0010,      /* mqtt try reconnect, but timeout */
    M_SUBSCRIBE_NOT_ACK_ERROR                            = -0x000F,      /* mqtt subscribe, but not ack */
    M_NOT_CONNECT_ERROR                                  = -0x000E,      /* mqtt not connect */
    M_SUBSCRIBE_ACK_PACKET_ERROR                         = -0x000D,      /* mqtt subscribe, but ack packet error */
    M_UNSUBSCRIBE_ACK_PACKET_ERROR                       = -0x000C,      /* mqtt unsubscribe, but ack packet error */
    M_PUBLISH_ACK_PACKET_ERROR                           = -0x000B,      /* mqtt pubilsh ack packet error */
    M_PUBLISH_ACK_TYPE_ERROR                             = -0x000A,      /* mqtt pubilsh ack type error */
    M_PUBREC_PACKET_ERROR                                = -0x0009,      /* mqtt pubrec packet error */
    M_BUFFER_TOO_SHORT_ERROR                             = -0x0008,      /* mqtt buffer too short */
    M_NOTHING_TO_READ_ERROR                              = -0x0007,      /* mqtt nothing to read */
    M_SUBSCRIBE_QOS_ERROR                                = -0x0006,      /* mqtt subsrcibe qos error */
    M_BUFFER_OVERFLOW_ERROR                              = -0x0005,      /* mqtt buffer overflow */
    M_CONNECT_FAILED_ERROR                               = -0x0004,      /* mqtt connect failed */
    M_MEM_NOT_ENOUGH_ERROR                               = -0x0003,      /* mqtt memory not enough */
    M_NULL_VALUE_ERROR                                   = -0x0002,      /* mqtt value is null */
    M_FAILED_ERROR                                       = -0x0001,      /* failed */
    M_SUCCESS_ERROR                                      = 0x0000        /* success */
} mypro_error_t;

#define RETURN_ERROR(x) { return x; }


#endif

