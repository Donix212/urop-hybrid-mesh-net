 #ifndef ICMP_SOCKET_H
 #define ICMP_SOCKET_H
 
 #include "ns3/callback.h"
 #include "ns3/object.h"
 #include "ns3/ptr.h"
 #include "ns3/socket.h"
 #include "ns3/traced-callback.h"
 
 namespace ns3
 {
 
 class Node;
 class Packet;
 
 /**
  * @ingroup socket
  * @ingroup icmp
  *
  * @brief (abstract) base class for all IcmpSockets for IPv4 unicast operations.
  *
  * This class exists solely for hosting IcmpSocket attributes that can
  * be reused across different implementations.
  */
 class IcmpSocket : public Socket
 {
 public:
   /**
    * Get the type ID.
    * @return the object TypeId
    */
   static TypeId GetTypeId();
 
   IcmpSocket ();
   ~IcmpSocket () override;
 
 private:
   /**
    * @brief Set the receiving buffer size.
    * @param size the buffer size
    */
   virtual void SetRcvBufSize (uint32_t size) = 0;
   /**
    * @brief Get the receiving buffer size.
    * @returns the buffer size
    */
   virtual uint32_t GetRcvBufSize () const = 0;
 
 
   /**
    * @brief Set the MTU discover capability.
    * @param discover the MTU discover capability
    */
   virtual void SetMtuDiscover (bool discover) = 0;
   /**
    * @brief Get the MTU discover capability.
    * @returns the MTU discover capability
    */
   virtual bool GetMtuDiscover () const = 0;
 };
 
 } // namespace ns3
 
 #endif /* ICMP_SOCKET_H */
 