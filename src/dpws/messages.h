typedef enum
{
	WssddAction_Hello,
	WssddAction_Bye,
	WssddAction_Probe,
	WssddAction_ProbeMatch,
	WsddAction_Resolve,
	WsddAction_ResolveMatch,
} E_WsddAction;
void discoverymsg_setAction(DiscoveryMsg *this, E_WsddAction action);
void discoverymsg_setAppSeq(DiscoveryMsg *this, DiscoveryAppSeq *appseq);
long discoverymsg_setUId(DiscoveryMsg *this);
long discoverymsg_getUId(DiscoveryMsg *this);
void discoverymsg_setRelatesToId(DiscoveryMsg *this, long msgid);
void discoverymsg_setTo(DiscoveryMsg *this,struct sockaddr_storage *address);
void discoverymsg_setReplyTo(DiscoveryMsg *this,struct sockaddr_storage *address);
void discoverymsg_setBody(DiscoveryMsg *this, DiscoveryBody *body);

typedef enum
{
	WsddEPType_Printer,
	WsddEPType_nas,
} E_WsddEPType;
void discoverybody_appendBody(DiscoveryBody *this, DiscoveryBody *bodyinbody);
void discoverybody_setEndPoint(DiscoveryBody *this, char uid[16]);
void discoverybody_appendType(DiscoveryBody *this, E_WsddEPType type);
void discoverybody_appendScopes(DiscoveryBody *this, char *scopes);
void discoverybody_setURL(DiscoveryBody *this, char *url);
int discoverybody_getVersion(DiscoveryBody *this);

void discoveryappseq_setId(DicoveryAppSEq *this, unsigned int id);
void discoveryappseq_setNb(DicoveryAppSEq *this, unsigned int nb);
void discoveryappseq_setUId(DicoveryAppSEq *this);
