
#ifndef CONNECTION_H
#define CONNECTION_H


/**
  * class Connection
  * 
  */

class Connection
{
public:
  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  Connection();

  /**
   * Empty Destructor
   */
  virtual ~Connection();

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  



  /**
   * @brief Constructor
   * @param _pNode pointer to target dataNode
   * @param  _pNode
   */
   Connection(DataNode* _pNode)
  {
  }


  /**
   * @brief check Connection IO
   * 
   * Check for new messages from socket and DataNode.
   * Call communication handlers if new data is available for transfer.
   */
  void tick()
  {
  }


  /**
   * @brief Getter function for node address
   * @return node address
   * @return int
   */
  int getNodeAddr()
  {
  }


  /**
   * @brief Getter function for Connection status
   * @return eConnectionState status enum value
   * @return eConnectionState
   */
  eConnectionState getStatus()
  {
  }

protected:
  // Static Protected attributes
  //  

  // Protected attributes
  //  


  // Protected attribute accessor methods
  //  


  // Protected attribute accessor methods
  //

private:
  // Static Private attributes
  //  

  // Private attributes
  //  

  // < UDP socket file descriptor
  int sock_ext;
  // < Linked DataNode
  DataNode* pNode;
  // < Node address
  uint8_t nodeAddr;
  // < UDP RX source address
  struct sockaddr srcAddr;
  // < IP source address length
  socklen_t srcAddrLen;
  // < Status of Connection
  eConnectionState status;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of sock_ext
   * < UDP socket file descriptor
   * @param value the new value of sock_ext
   */
  void setSock_ext(int value)
  {
    sock_ext = value;
  }

  /**
   * Get the value of sock_ext
   * < UDP socket file descriptor
   * @return the value of sock_ext
   */
  int getSock_ext()
  {
    return sock_ext;
  }

  /**
   * Set the value of pNode
   * < Linked DataNode
   * @param value the new value of pNode
   */
  void setPNode(DataNode* value)
  {
    pNode = value;
  }

  /**
   * Get the value of pNode
   * < Linked DataNode
   * @return the value of pNode
   */
  DataNode* getPNode()
  {
    return pNode;
  }

  /**
   * Set the value of nodeAddr
   * < Node address
   * @param value the new value of nodeAddr
   */
  void setNodeAddr(uint8_t value)
  {
    nodeAddr = value;
  }

  /**
   * Get the value of nodeAddr
   * < Node address
   * @return the value of nodeAddr
   */
  uint8_t getNodeAddr()
  {
    return nodeAddr;
  }

  /**
   * Set the value of srcAddr
   * < UDP RX source address
   * @param value the new value of srcAddr
   */
  void setSrcAddr(struct sockaddr value)
  {
    srcAddr = value;
  }

  /**
   * Get the value of srcAddr
   * < UDP RX source address
   * @return the value of srcAddr
   */
  struct sockaddr getSrcAddr()
  {
    return srcAddr;
  }

  /**
   * Set the value of srcAddrLen
   * < IP source address length
   * @param value the new value of srcAddrLen
   */
  void setSrcAddrLen(socklen_t value)
  {
    srcAddrLen = value;
  }

  /**
   * Get the value of srcAddrLen
   * < IP source address length
   * @return the value of srcAddrLen
   */
  socklen_t getSrcAddrLen()
  {
    return srcAddrLen;
  }

  /**
   * Set the value of status
   * < Status of Connection
   * @param value the new value of status
   */
  void setStatus(eConnectionState value)
  {
    status = value;
  }

  /**
   * Get the value of status
   * < Status of Connection
   * @return the value of status
   */
  eConnectionState getStatus()
  {
    return status;
  }


  /**
   * @brief deInit Connection
   */
  void deInit()
  {
  }


  /**
   * @brief initialize UDP socket depending on node address
   */
  void initSocket()
  {
  }


  /**
   * @brief deinit UDP socket
   */
  void deInitSocket()
  {
  }


  /**
   * @brief RX data from UDP socket and TX to WPAN Node
   * @return int
   */
  int extToLocalHandler()
  {
  }


  /**
   * @brief RX data from WPAN Node and TX to UDP socket
   * TX to last RX address
   * @return int
   */
  int localToExtHandler()
  {
  }

  void initAttributes();

};

#endif // CONNECTION_H
