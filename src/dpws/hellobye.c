#include "messages.h"

DiscoveryMsg *msgseq_createHello(MsgSeq *this, struct sockaddr_storage *dst, TargetService *target)
{
	DiscoveryMsg *msg = discoverymsg_new(WssddAction_Hello);
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
		EndPoint *ep = targetservice_getEP(target);
		endpoint_fillBody(ep, body, msgseq_isLong(this));
		discoverymsg_setBody(msg, body);
	}
	return msg;
}

int msgseq_parseHello(MsgSeq *this, DiscoveryMsg *hello, ClientService *client)
{
	DiscoveryBody *body = discoverymsg_getBody(hello);
	if (body)
	{
		EndPoint *ep = endpoint_new(discoverybody_getEndPoint(body));
		if (clientservice_exists(client, ep))
		{
			if (clientservice_EPState(client, ep) == E_WsddEPState_Down)
			{
				clientservice_setEPState(client, ep, E_WsddEPState_Up);
			}
			else
			{
				clientservice_setEPState(client, ep, E_WsddEPState_Restart);
				msgseq_setChange(this, E_MsgSeqChange_EPStateIllegale);
			}
		}
		else
		{
			while ((type = discoverybody_types(body)) != -1)
			{
				discoverybody_appendType(body, type);
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


DiscoveryMsg *msgseq_createBye(MsgSeq *this, struct sockaddr_storage *dst, TargetService *target)
{
	DiscoveryMsg *msg = discoverymsg_new(WssddAction_Bye);
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
		EndPoint *ep = targetservice_getEndPoint(target);
		endpoint_fillBody(ep, body, msgseq_isLong(this));
		discoverymsg_setBody(msg, body);
	}
	return msg;
}

int msgseq_parseBye(MsgSeq *this, DiscoveryMsg *bye, ClientService *client)
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
		endpoint_destroy(ep);
	}
}