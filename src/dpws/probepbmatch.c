#include "messages.h"

DiscoveryMsg *msgseq_createProbe(MsgSeq *this, struct sockaddr_storage *dst, ClientService *client)
{
	DiscoveryMsg *msg = discoverymsg_new(WssddAction_ProbeMatches);
	if (msg)
	{
		msgseq_setUid(this, discoverymsg_getUId(msg));
		if (msgseq_isRelatesTo(this))
		{
			MsgSeq *previous = msgseq_getPrevious(this);
			discoverymsg_setRelatesToId(msg, msgseq_getUid(previous));
		}
		discoverymsg_setTo(msg, dst);
		if (msgseq_isSequence(this))
		{
			DiscoveryAppSeq *appseq = msgseq_getAppSeq(this, msg);
			discoverymsg_setAppSeq(msg, appseq);
		}
		DiscoveryBody *body = discoverybody_new(msg,NULL);
		EndPoint *ep = clientservice_getEPsearch(client);
		endpoint_fillBody(ep, body, msgseq_isLong(this));
		discoverymsg_setBody(msg, body);
	}
	return msg;
}

EndPoint *msgseq_parseProbe(MsgSeq *this, DiscoveryMsg *hello, TargetService *target)
{
	EndPoint *ep = NULL;
	DiscoveryBody *body = discoverymsg_getBody(hello);
	if (body)
	{
		ep = endpoint_new(E_WsddEPRefId_All);

		while ((type = discoverybody_types(body)) != -1)
		{
			enpoint_appendType(ep, type);
		}
		char *scop;
		while ((scop = discoverybody_scopes(body)) != NULL)
			endpoint_appendScopes(ep, scop);
		char *url;
		if ((url = discoverybody_urls(body)) != NULL)
			endpoint_setURL(ep, url);

		// look for at least one EndPoint contraints result and change the msgseq state
		EndPoint *ep2 = targetservice_searchEP(client, ep);
		if (ep2 != NULL)
		{
			msgseq_setChange(this, E_MsgSeqChange_EPFound);
		}
	}
	return ep;
}


DiscoveryMsg *msgseq_createProbeMatches(MsgSeq *this, struct sockaddr_storage *dst, TargetService *target, EndPoint *epcontraints)
{
	DiscoveryMsg *msg = discoverymsg_new(WssddAction_ProbeMatches);
	if (msg)
	{
		msgseq_setUid(this, discoverymsg_getUId(msg));
		if (msgseq_isRelatesTo(this))
		{
			MsgSeq *previous = msgseq_getPrevious(this);
			discoverymsg_setRelatesToId(msg, msgseq_getUid(previous));
		}
		discoverymsg_setTo(msg, dst);
		if (msgseq_isSequence(this))
		{
			DiscoveryAppSeq *appseq = msgseq_getAppSeq(this, msg);
			discoverymsg_setAppSeq(msg, appseq);
		}

		DiscoveryBody *body = discoverybody_new(msg,NULL);
		while ((ep = targetservice_searchEP(target, epcontraints) != NULL)
		{
			DiscoveryBody *bodyinbody = discoverybody_new(NULL,body);
			endpoint_fillBody(ep, bodyinbody, msgseq_isLong(this));
			discoverybody_appendBody(body, bodyinbody);
		}
		discoverymsg_setBody(msg, body);
	}
	return msg;
}

int msgseq_parseProbeMatches(MsgSeq *this, DiscoveryMsg *bye, ClientService *client)
{
	DiscoveryBody *body = discoverymsg_getBody(bye);
	if (body)
	{
		EndPoint *ep = endpoint_new(discoverybody_getEndPoint(body));
		if (clientservice_exists(client, ep))
		{
			if (clientservice_EPState(client, ep) == E_WsddEPState_Up)
			{
				clientservice_setEPState(client, ep, E_WsddEPState_Down);
			}
			else
			{
				clientservice_setEPState(client, ep, E_WsddEPState_Down);
				msgseq_setChange(this, E_MsgSeqChange_EPStateIllegale);
			}
		}
		else
		{
			while ((type = discoverybody_types(body)) != -1)
			{
				endpoint_appendType(body, type);
			}
			char *scop;
			while ((scop = discoverybody_scopes(body)) != NULL)
				endpoint_appendScopes(ep, scop);
			char *url;
			if ((url = discoverybody_urls(body)) != NULL)
				endpoint_setURL(ep, url);

			clientservice_createEP(client, ep);
			msgseq_setChange(this, E_MsgSeqChange_EPNew);
		}
		endpoint_destroy(ep);
	}
}