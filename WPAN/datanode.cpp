#include "datanode.h"

DataNode::DataNode():DataNode{0,0,0}{}

DataNode::DataNode(uint8_t _address, uint32_t _group) : DataNode(_address,_group,0){}

DataNode::DataNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration)
: Node(_address,_group, _leaseDuration){
    protocols=NETWORK_PROTOCOL_ID | DATA_PROTOCOL_ID;
}

DataNode::DataNode(Node & base):DataNode(base.getAddr(),base.getGroup(),base.getLeaseDuration()){
    leaseStartTime=base.getLeaseStartTime();
}

DataNode::~DataNode(){
    clearDatagrams();
}

void DataNode::show(){
    cout<<"Address: "<<to_string(getAddr());
    cout<<" Group: "<<to_string(getGroup());
    cout<<" Type : Data node";
}

uint16_t DataNode::datagram_tx(uint8_t * data, uint16_t dataSize){
    const uint8_t headerSize=DATA_HEADER_SIZE+1;
    const uint8_t packetIndexOffset=1+sizeof(datagram_id_t);
    const uint8_t nPacketsOffset=packetIndexOffset+sizeof(packet_index_t);

    static datagram_id_t datagramID=1;
    if(dataSize==0 || dataSize>DATA_MAX_DATAGRAM_LENGTH) return 0;

    int remainingDataSize=dataSize;

#ifdef DEBUG_DATA
        cout<<"Data: Remaining data size/Max datagram length : "<<to_string(remainingDataSize)<<"/"<<to_string(DATA_MAX_DATAGRAM_LENGTH)<<endl;
#endif
    packet_index_t nPackets=ceil(remainingDataSize/DATA_MAX_PACKET_LENGTH);
    if(remainingDataSize%DATA_MAX_PACKET_LENGTH == 0){
        nPackets--;
    }
    packet_index_t packetIndex=0;
    uint8_t packetLength=0;

    uint8_t frame[ISM_MAX_DATA_SIZE]={0};
    uint8_t frameLength=0;
    uint16_t dataIndex=0;
    // Constant part of frame header
    frame[0]=DATA_PROTOCOL_ID;
    memcpy(&frame[1], &datagramID, sizeof(datagram_id_t));

#ifdef DEBUG_DATA
        cout<<"Data: Destination : "<<to_string(getAddr())<<endl;
#endif

    while(packetIndex<=nPackets && remainingDataSize>0){
#ifdef DEBUG_DATA
        cout<<"Data: Packet "<<to_string(packetIndex)<<"/"<<to_string(nPackets);
#endif
        // Build frame header
        memcpy(&frame[packetIndexOffset],&packetIndex,sizeof(packet_index_t));
        memcpy(&frame[nPacketsOffset],&nPackets,sizeof(packet_index_t));

        // Segment datagram in packets
        packetLength=remainingDataSize>=(DATA_MAX_PACKET_LENGTH)?
                        DATA_MAX_PACKET_LENGTH:remainingDataSize;
        memcpy(&frame[headerSize], &data[dataIndex], packetLength);

        remainingDataSize-=packetLength;
        dataIndex+=packetLength;
        frameLength=packetLength+headerSize;
#ifdef DEBUG_DATA
        cout<<", length =  "<<to_string(packetLength)<<", frame length =  "<<to_string(frameLength)<<", remaining data size = "<<to_string(remainingDataSize)<<endl;
#endif
        for(int i=0;i<3;i++){
            ism_tick();
            delayMs(20);
        }

        // Transmit frame

        while(!tx(frame,frameLength)){
#ifdef DEBUG_DATA
        cout<<"Data: Wait 100 ms for module to be available"<<endl;
#endif
            ism_tick();
            delayMs(20);
            ism_tick();
            delayMs(20);
            ism_tick();
            delayMs(20);
            ism_tick();
            delayMs(20);
            ism_tick();
            delayMs(20);
        }
        printDataFrame(frame,frameLength,true);
        ism_tick();
        packetIndex++;
    }
    datagramID++;

    // Return TX data size
    return dataIndex;
}

int DataNode::readyRxDatagrams(){
    return readyDatagrams;
}

uint16_t DataNode::readDatagram(uint8_t * buffer, uint16_t maxSize){
    int datagramIndex=0;
    uint16_t dataSize=0;
    // Find first ready datagram
    while(datagramIndex<datagrams.size()){
        if(datagrams.at(datagramIndex).ready){
            break;
        }
        datagramIndex++;
    }
#ifdef DEBUG_DATA
    cout << "Data: Read datagram "<<to_string(datagramIndex)<<endl;
#endif
    if(datagramIndex<datagrams.size()){
        // If found, copy data in buffer if buffer is big enough
        if(maxSize>=datagrams.at(datagramIndex).size){
            dataSize=datagrams.at(datagramIndex).size;
        }else{
            dataSize=maxSize;
        }
        memcpy(buffer,
                datagrams.at(datagramIndex).data,
                dataSize);

        // Free datagram
        free(datagrams.at(datagramIndex).data);
        datagrams.erase(datagrams.begin()+datagramIndex);
        readyDatagrams--;

        return dataSize;

    }
    return 0; // could not find datagram, no data copied
}

void DataNode::clearDatagrams(){
    for(int i=0;i<datagrams.size();i++){
        free(datagrams.at(i).data);
    }
    datagrams.clear();
}

void DataNode::dataCallback(const uint8_t* data, uint8_t size){
    packet_index_t packetIndex=0;
    packet_index_t packetNumber=0;
    datagram_id_t datagramId=0;
    sDatagram datagram;
    int datagramIndex=0;
    uint8_t * pDatagramData=NULL;
    const uint8_t headerSize=DATA_HEADER_SIZE;

    datagram.ready=false;
#ifdef DEBUG_DATA
    cout << "Data: RX callback"<<endl;
#endif
    printDataFrame(data,size,false);
    // Copy frame indices header
    if(size>=headerSize){
        // Copy packet ID
        memcpy(&datagramId, data, sizeof(datagram_id_t));
        // Copy sub-packet index
        memcpy(&packetIndex, data+sizeof(datagram_id_t),
               sizeof(packet_index_t));
        // Copy total sub-packet number
        memcpy(&packetNumber,
               data+sizeof(datagram_id_t)+sizeof(packet_index_t),
               sizeof(packet_index_t));

        // Init packet or find existing datagram from ID
        // If first packet, init datagram
        if(packetIndex==0){
#ifdef DEBUG_DATA
            cout << "Data: RX Packet 0"<<endl;
#endif
            datagram.id=datagramId;
            datagram.size=DATA_MAX_PACKET_LENGTH*(packetNumber+1);
            // Allocate and init new memory zone for new datagram
            pDatagramData=(uint8_t *) malloc(datagram.size);
            pDatagramData=(uint8_t *) memset(pDatagramData, 0, datagram.size);
            if(pDatagramData!=NULL){
                datagram.data=pDatagramData;
                datagrams.push_back(datagram);
            }
        }else if(packetIndex<=packetNumber){
            // Find datagram in vector from datagram ID
            while(datagramIndex<datagrams.size()){
                if(datagrams.at(datagramIndex).id == datagramId){
                    break;
                }
                datagramIndex++;
            }
            if(datagramIndex<datagrams.size()){
                datagram=datagrams.at(datagramIndex);
            }else return; // could not find datagram
        }else{
            // ERROR
            return;
        }

        // Copy data
        // If new datagram
        if(packetIndex==0){
            // copy first packet
            memcpy(datagram.data, data+headerSize, size-headerSize);
        }else if(packetIndex<=packetNumber){
            memcpy(datagram.data+DATA_MAX_PACKET_LENGTH*packetIndex,
                   data+headerSize,size-headerSize);

        }else{ // packet index overflow
            // Remove datagram from vector
            free(datagrams.at(datagramIndex).data);
            datagrams.erase(datagrams.begin()+datagramIndex);
        }
        if(packetIndex==packetNumber){ // If last packet
            // Update datagram size
            datagrams.at(datagramIndex).size-=(DATA_MAX_PACKET_LENGTH-(size-headerSize));
#ifdef DEBUG_DATA
            cout << "Data: RX Datagram size = "<<to_string(datagrams.at(datagramIndex).size)<<endl;
#endif
            datagrams.at(datagramIndex).data= (uint8_t *) realloc(datagram.data, datagrams.at(datagramIndex).size);
            datagrams.at(datagramIndex).ready=true;

            // Update number of ready datagrams
            readyDatagrams++;
        }
    }
}

// Redefinition
void DataNode::printDataFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_DATA
    if(dir)
        cout<<"DataNode DATA TX: ";
    else
        cout<<"DataNode DATA RX: ";

    printBufferHex(buffer, size);
#endif
}
