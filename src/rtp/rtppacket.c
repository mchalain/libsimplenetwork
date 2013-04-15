
Service *servicertp_new(int serviceid)
{
	Service *this = service_new(UDP);
	return this;
}
/*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           timestamp                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           synchronization source (SSRC) identifier            |
   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   |            contributing source (CSRC) identifiers             |
   |                             ....                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

typedef struct
{
	unsigned long m_version:2;
	unsigned long m_padding:1;
	unsigned long m_extension:1;
	unsigned long m_csrc_count:4;
	unsigned long m_marker:1;
	unsigned long m_payload_type: 7;
	unsigned long m_seqnumber: 16;
	unsigned long m_timestamp;
	unsigned long m_ssrc;
	unsigned long m_csrc[15];

} RTPHeader;

typedef struct _RTPPacket RTPPacket;
RTPPacket *rtppacket_new()
{
	RTPPacket *this = malloc(sizeof(RTPPacket));
	memset(this, 0, sizeof(RTPPacket));

	return this;
}

static void rtppacket_init(RTPPacket *this, RTPProfile *profile)
{
	this->m_header = this->m_buffer;

}

int rtppacket_parse(RTPPacket *this, const char *buffer)
{
	int ret = 0;
	this->m_buffer = buffer;
	RTPProfile *profile = rtpprofile_new(((RTPHeader *)(this->m_buffer)->payloadtype);
	rtppacket_init(this, profile);

	return ret;
}

void rtppacket_build(RTPPacket *this, RTPProfile *profile)
{
	this->m_buffer = malloc(rtpprofile_psize(profile) + sizeof(RTPHeader));
	rtppacket_init(this, profile);
}
/*
   name of                              sampling              default
   encoding  sample/frame  bits/sample      rate  ms/frame  ms/packet
   __________________________________________________________________
   DVI4      sample        4                var.                   20
   G722      sample        8              16,000                   20
   G723      frame         N/A             8,000        30         30
   G726-40   sample        5               8,000                   20
   G726-32   sample        4               8,000                   20
   G726-24   sample        3               8,000                   20
   G726-16   sample        2               8,000                   20
   G728      frame         N/A             8,000       2.5         20
   G729      frame         N/A             8,000        10         20
   G729D     frame         N/A             8,000        10         20
   G729E     frame         N/A             8,000        10         20
   GSM       frame         N/A             8,000        20         20
   GSM-EFR   frame         N/A             8,000        20         20
   L8        sample        8                var.                   20
   L16       sample        16               var.                   20
   LPC       frame         N/A             8,000        20         20
   MPA       frame         N/A              var.      var.
   PCMA      sample        8                var.                   20
   PCMU      sample        8                var.                   20
   QCELP     frame         N/A             8,000        20         20
   VDVI      sample        var.             var.                   20

		PT   encoding    media type  clock rate   channels
                    name                    (Hz)
               ___________________________________________________
		0    PCMU        A            8,000       1
               1    reserved    A
               2    reserved    A
               3    GSM         A            8,000       1
               4    G723        A            8,000       1
               5    DVI4        A            8,000       1
               6    DVI4        A           16,000       1
               7    LPC         A            8,000       1
               8    PCMA        A            8,000       1        (G711)
               9    G722        A            8,000       1
               10   L16         A           44,100       2
               11   L16         A           44,100       1
               12   QCELP       A            8,000       1
               13   CN          A            8,000       1
               14   MPA         A           90,000       (see text)  MP3
               15   G728        A            8,000       1
               16   DVI4        A           11,025       1
               17   DVI4        A           22,050       1
               18   G729        A            8,000       1
*/
typedef enum
{
	RTPProfile_MPA = 14;
	RTPProfile_G711 = 8;
	RTPProfile_G728 = 15;
	RTPProfile_L16 = 11;
} RTPProfile;

void rtppacket_setprofile(RTPPacket *this, RTPProfile profile)
{
}

void rtppacket_appendcsrc(RTPPacket *this, unsigned long csrcid)
{
	this->m_header->m_csrc[this->m_header->m_csrc_count] = csrcid;
	this->m_header->m_csrc_count = (this->m_header->m_csrc_count + 1) % 0x0F;
}

void rtppacket_addpading(RTPPacket *this, int paddingcounter)
{

	return NULL;
}

typedef struct
{
	struct
	{
		unsigned long m_version:2;
		unsigned long m_padding:1;
		unsigned long m_report:5;
		unsigned long m_payload_type:8;
		unsigned long m_length:16;
		unsigned long m_ssrc;
	} *m_header;
} RTCPPacket;

typedef enum
{
	RTCPType_SenderReport = 0x200;
	RTCPType_ReceiverReport = 0x201;
	RTCPType_SourceDescription = 0x202;
	RTCPType_Goodbye = 0x203;
	RTCPType_ApplicationDefined = 0x204;
} RTCPType;

RTCPPacket *rtcppacket_new(RTCPType type)
{
	RTCPPacket *this = malloc(sizeof(RTCPPacket));
	memset(this, 0, sizeof(RTCPPacket));

	this->m_header = this->m_buffer;
	this->m_payload_type = type & 0xFF;

	return this;
}
