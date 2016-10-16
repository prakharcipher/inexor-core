#pragma once

/// network packet buffer
struct packetbuf : ucharbuf
{
    ENetPacket *packet;
    int growth;

	/// call constructor from inherited class in this constructor
    packetbuf(ENetPacket *packet) : ucharbuf(packet->data, packet->dataLength), packet(packet), growth(0)
	{
	}

	/// reserve memory in this constructor
    packetbuf(int growth, int pflags = 0) : growth(growth)
    {
        packet = enet_packet_create(NULL, growth, pflags);
        buf = (uchar *)packet->data;
        maxlen = packet->dataLength;
    }

    /// call cleanup in destructor!
	~packetbuf() 
	{
		cleanup();
	}

	/// set ENET_PACKET_FLAG_RELIABLE - flag
    void reliable() 
	{
		packet->flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	/// resize ENET packet, copy data and buffer length
    void resize(int n)
    {
        enet_packet_resize(packet, n);
        buf = (uchar *)packet->data;
        maxlen = packet->dataLength;
    }

	/// check if [n] bytes memory are available in this buffer
	/// if not reserve memory!
    void checkspace(int n)
    {
        if(len + n > maxlen && packet && growth > 0) resize(max(len + n, maxlen + growth));
    }

	/// create a sub buffer of [sz] bytes size
    ucharbuf subbuf(int sz)
    {
        checkspace(sz);
        return ucharbuf::subbuf(sz);
    }

	/// write to buffer functions
    void put(const uchar &val)
    {
        checkspace(1);
        ucharbuf::put(val);
    }
    void put(const uchar *vals, int numvals)
    {
        checkspace(numvals);
        ucharbuf::put(vals, numvals);
    }

	/// ?
    ENetPacket *finalize()
    {
        resize(len);
        return packet;
    }

	/// destroy ENET packet and reset memory
    void cleanup()
    {
        if(growth > 0 && packet && !packet->referenceCount) 
		{
			enet_packet_destroy(packet); 
			packet = NULL; 
			buf = NULL; 
			len = maxlen = 0;
		}
    }
};
